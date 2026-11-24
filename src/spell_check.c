#include <gtk/gtk.h>
#include <pango/pango.h>
#include <string.h>
#include "spell_check.h"
#include "str_utils.h"

#define LORA_OPEN "<lora:"
#define LORA_OPEN_LEN (sizeof(LORA_OPEN) - 1)
#define SPELL_CHECK_DATA_KEY "spell-check-data"
#define SPELL_ERROR_TAG_NAME "spell-error"

struct SpellDict {
	GHashTable *words;
	gatomicrefcount ref_count;
};

typedef struct {
	SpellDict *dict;
	GtkTextTag *error_tag;
	GtkTextMark *cursor_mark;
	gulong insert_handler_id;
	gulong delete_handler_id;
	gulong delete_after_handler_id;
	gulong cursor_handler_id;
	gboolean needs_full_check;
} SpellCheckData;

static gboolean is_ascii_lowercase(const char *s)
{
	for (const unsigned char *c = (const unsigned char *)s; *c != '\0'; c++) {
		if (*c >= 0x80 || (*c >= 'A' && *c <= 'Z')) return FALSE;
	}
	return TRUE;
}

static gboolean is_word_joiner(gunichar ch)
{
	return ch == '\'' || ch == 0x2019 || ch == '-';
}

static void expand_to_lines(const GtkTextIter *a, const GtkTextIter *b, GtkTextIter *line_s, GtkTextIter *line_e)
{
	*line_s = *a;
	gtk_text_iter_set_line_offset(line_s, 0);
	*line_e = *b;
	if (!gtk_text_iter_ends_line(line_e)) gtk_text_iter_forward_to_line_end(line_e);
}

static gboolean has_open_angle_nearby(const GtkTextIter *pos)
{
	GtkTextIter it = *pos;
	for (size_t i = 0; i < LORA_OPEN_LEN - 1; i++) {
		if (gtk_text_iter_starts_line(&it)) return FALSE;
		if (!gtk_text_iter_backward_char(&it)) return FALSE;
		if (gtk_text_iter_get_char(&it) == '<') return TRUE;
	}
	return FALSE;
}

static gboolean inside_lora_at(const GtkTextIter *pos)
{
	GtkTextIter ls = *pos;
	gtk_text_iter_set_line_offset(&ls, 0);

	GtkTextIter end_iter = *pos;
	for (size_t i = 0; i < LORA_OPEN_LEN && !gtk_text_iter_ends_line(&end_iter); i++) {
		gtk_text_iter_forward_char(&end_iter);
	}

	char *line = gtk_text_iter_get_slice(&ls, &end_iter);
	size_t line_len = strlen(line);
	int byte_idx = gtk_text_iter_get_line_index(pos);
	size_t clamped_idx = byte_idx < 0 ? 0 : ((size_t)byte_idx < line_len ? (size_t)byte_idx : line_len);
	const char *end = line + clamped_idx;
	gboolean inside = FALSE;

	for (const char *p = line; p < end; p++) {
		if (inside) {
			if (*p == '>') inside = FALSE;
		} else if (g_str_has_prefix(p, LORA_OPEN)) {
			inside = TRUE;
			p += LORA_OPEN_LEN - 1;
		}
	}

	g_free(line);
	return inside;
}

static gboolean at_lora_open(const GtkTextIter *pos)
{
	GtkTextIter it = *pos;
	for (size_t i = 0; i < LORA_OPEN_LEN; i++) {
		if (gtk_text_iter_get_char(&it) != (gunichar)(unsigned char)LORA_OPEN[i]) return FALSE;
		gtk_text_iter_forward_char(&it);
	}
	return TRUE;
}

static void check_word(GtkTextBuffer *tb, const GtkTextIter *si, const GtkTextIter *ei, SpellCheckData *data)
{
	if (data->dict == NULL || data->dict->words == NULL || data->error_tag == NULL) return;

	char *word = gtk_text_buffer_get_text(tb, si, ei, FALSE);
	if (word == NULL) return;

	gboolean misspelled = FALSE;

	if (word[0] != '\0' && !check_word_contains_num(word)) {
		if (is_ascii_lowercase(word)) {
			misspelled = !g_hash_table_contains(data->dict->words, word);
		} else {
			char *key_to_check = normalize_word(word);
			if (key_to_check != NULL) {
				misspelled = !g_hash_table_contains(data->dict->words, key_to_check);
				g_free(key_to_check);
			}
		}
	}

	if (misspelled) {
		gtk_text_buffer_apply_tag(tb, data->error_tag, si, ei);
	} else {
		gtk_text_buffer_remove_tag(tb, data->error_tag, si, ei);
	}
	g_free(word);
}

static void check_range(GtkTextBuffer *tb, const GtkTextIter *range_s, const GtkTextIter *range_e, SpellCheckData *data, gboolean skip_word)
{
	if (data == NULL || data->error_tag == NULL) return;
	if (gtk_text_iter_compare(range_s, range_e) >= 0) return;

	GtkTextIter insert_pos;
	gtk_text_buffer_get_iter_at_mark(tb, &insert_pos, gtk_text_buffer_get_insert(tb));

	GtkTextIter scan_pos = *range_s, gap_start = *range_s;
	gboolean inside_lora = inside_lora_at(range_s);

	while (gtk_text_iter_compare(&scan_pos, range_e) < 0) {
		gunichar current_char = gtk_text_iter_get_char(&scan_pos);

		if (inside_lora) {
			if (current_char == '>' || gtk_text_iter_ends_line(&scan_pos)) inside_lora = FALSE;
			gtk_text_iter_forward_char(&scan_pos);
			continue;
		}
		if (current_char == '<' && at_lora_open(&scan_pos)) {
			inside_lora = TRUE;
			gtk_text_iter_forward_chars(&scan_pos, (gint)LORA_OPEN_LEN);
			continue;
		}
		if (!g_unichar_isalnum(current_char)) {
			gtk_text_iter_forward_char(&scan_pos);
			continue;
		}

		GtkTextIter word_start = scan_pos;
		while (gtk_text_iter_compare(&scan_pos, range_e) < 0) {
			current_char = gtk_text_iter_get_char(&scan_pos);
			if (g_unichar_isalnum(current_char)) {
				gtk_text_iter_forward_char(&scan_pos);
				continue;
			}
			if (is_word_joiner(current_char)) {
				GtkTextIter next = scan_pos;
				gtk_text_iter_forward_char(&next);
				if (gtk_text_iter_compare(&next, range_e) < 0 && g_unichar_isalnum(gtk_text_iter_get_char(&next))) {
					scan_pos = next;
					continue;
				}
			}
			break;
		}

		GtkTextIter word_end = scan_pos;
		if (gtk_text_iter_compare(&gap_start, &word_start) < 0) {
			gtk_text_buffer_remove_tag(tb, data->error_tag, &gap_start, &word_start);
		}
		gap_start = word_end;

		if (skip_word && gtk_text_iter_compare(&insert_pos, &word_start) > 0 && gtk_text_iter_compare(&insert_pos, &word_end) <= 0) {
			gtk_text_buffer_remove_tag(tb, data->error_tag, &word_start, &word_end);
			continue;
		}
		check_word(tb, &word_start, &word_end, data);
	}

	if (gtk_text_iter_compare(&gap_start, range_e) < 0) {
		gtk_text_buffer_remove_tag(tb, data->error_tag, &gap_start, range_e);
	}
}

static void check_span(GtkTextBuffer *tb, const GtkTextIter *range_s, const GtkTextIter *range_e, SpellCheckData *data, gboolean skip_word)
{
	GtkTextIter word_start = *range_s, word_end = *range_e;
	GtkTextIter temp_iter = word_start;

	while (gtk_text_iter_backward_char(&temp_iter)) {
		gunichar key = gtk_text_iter_get_char(&temp_iter);
		if (!g_unichar_isalnum(key) && !is_word_joiner(key)) break;
		word_start = temp_iter;
	}

	while (!gtk_text_iter_is_end(&word_end)) {
		gunichar key = gtk_text_iter_get_char(&word_end);
		if (!g_unichar_isalnum(key) && !is_word_joiner(key)) break;
		gtk_text_iter_forward_char(&word_end);
	}

	check_range(tb, &word_start, &word_end, data, skip_word);
}

static glong count_iter_steps(const gchar *text, gssize n_bytes)
{
	glong steps = g_utf8_strlen(text, n_bytes);
	for (gssize i = 0; i + 1 < n_bytes; i++) {
		if (text[i] == '\r' && text[i + 1] == '\n') {
			steps--;
		}
	}
	return steps;
}

static void on_text_inserted(GtkTextBuffer *tb, GtkTextIter *pos, const gchar *text, gint len, gpointer user_data)
{
	SpellCheckData *data = user_data;
	if (data == NULL || text == NULL || pos == NULL) return;

	gssize n_bytes = len < 0 ? (gssize)strlen(text) : (gssize)len;
	if (n_bytes <= 0) return;

	glong n_steps = count_iter_steps(text, n_bytes);
	if (n_steps <= 0) return;

	GtkTextIter si = *pos;
	if (n_steps > G_MAXINT) {
		gtk_text_buffer_get_start_iter(tb, &si);
	} else {
		gtk_text_iter_backward_chars(&si, (gint)n_steps);
	}

	gboolean is_single_char = (n_steps == 1);

	if (gtk_text_iter_get_line(&si) != gtk_text_iter_get_line(pos) ||
		memchr(text, '<', (size_t)n_bytes) != NULL ||
		memchr(text, '>', (size_t)n_bytes) != NULL ||
		has_open_angle_nearby(&si)) {
		GtkTextIter ls, le;
		expand_to_lines(&si, pos, &ls, &le);
		check_range(tb, &ls, &le, data, is_single_char);
		return;
	}

	check_span(tb, &si, pos, data, is_single_char);
}

static void on_delete_range(GtkTextBuffer *tb, GtkTextIter *range_s, GtkTextIter *range_e, gpointer user_data)
{
	SpellCheckData *data = user_data;
	if (data == NULL) return;

	gboolean multiline = gtk_text_iter_get_line(range_s) != gtk_text_iter_get_line(range_e);
	char *text = gtk_text_buffer_get_text(tb, range_s, range_e, TRUE);
	data->needs_full_check = multiline || (text != NULL && strpbrk(text, "<>") != NULL);
	g_free(text);
}

static void on_delete_range_after(GtkTextBuffer *tb, GtkTextIter *range_s, GtkTextIter *range_e, gpointer user_data)
{
	(void)range_e;
	SpellCheckData *data = user_data;
	if (data == NULL) return;

	if (data->needs_full_check || has_open_angle_nearby(range_s)) {
		data->needs_full_check = FALSE;
		GtkTextIter ls, le;
		expand_to_lines(range_s, range_s, &ls, &le);
		check_range(tb, &ls, &le, data, TRUE);
		return;
	}
	check_span(tb, range_s, range_s, data, TRUE);
}

static void on_cursor_moved(GtkTextBuffer *tb, GParamSpec *pspec, gpointer user_data)
{
	(void)pspec;
	SpellCheckData *data = user_data;
	if (data == NULL || data->cursor_mark == NULL || gtk_text_mark_get_deleted(data->cursor_mark)) return;

	GtkTextIter old_pos, cur_pos;
	gtk_text_buffer_get_iter_at_mark(tb, &old_pos, data->cursor_mark);
	gtk_text_buffer_get_iter_at_mark(tb, &cur_pos, gtk_text_buffer_get_insert(tb));
	if (gtk_text_iter_equal(&old_pos, &cur_pos)) return;

	gtk_text_buffer_move_mark(tb, data->cursor_mark, &cur_pos);
	check_span(tb, &old_pos, &old_pos, data, TRUE);
}

static void spell_check_data_free(gpointer user_data)
{
	SpellCheckData *data = user_data;
	if (data == NULL) return;
	g_clear_pointer(&data->dict, spell_dict_unref);
	g_free(data);
}

static void spell_check_buffer_full(GtkTextBuffer *tb, SpellCheckData *data)
{
	GtkTextIter buf_s, buf_e;
	gtk_text_buffer_get_bounds(tb, &buf_s, &buf_e);
	check_range(tb, &buf_s, &buf_e, data, FALSE);
}

static void detach_spell_check(GtkTextBuffer *tb)
{
	SpellCheckData *data = g_object_get_data(G_OBJECT(tb), SPELL_CHECK_DATA_KEY);
	if (data == NULL) return;

	if (data->insert_handler_id > 0) g_signal_handler_disconnect(tb, data->insert_handler_id);
	if (data->delete_handler_id > 0) g_signal_handler_disconnect(tb, data->delete_handler_id);
	if (data->delete_after_handler_id > 0) g_signal_handler_disconnect(tb, data->delete_after_handler_id);
	if (data->cursor_handler_id > 0) g_signal_handler_disconnect(tb, data->cursor_handler_id);

	if (data->error_tag != NULL) {
		GtkTextIter buf_s, buf_e;
		gtk_text_buffer_get_bounds(tb, &buf_s, &buf_e);
		gtk_text_buffer_remove_tag(tb, data->error_tag, &buf_s, &buf_e);
	}

	if (data->cursor_mark != NULL && !gtk_text_mark_get_deleted(data->cursor_mark)) {
		gtk_text_buffer_delete_mark(tb, data->cursor_mark);
	}

	g_object_set_data(G_OBJECT(tb), SPELL_CHECK_DATA_KEY, NULL);
}

void tv_disable_spell_check(GtkTextView *tv)
{
	if (!GTK_IS_TEXT_VIEW(tv)) return;
	GtkTextBuffer *tb = gtk_text_view_get_buffer(tv);
	if (tb == NULL) return;
	detach_spell_check(tb);
}

void tv_enable_spell_check(GtkTextView *tv, SpellDict *dict)
{
	if (!GTK_IS_TEXT_VIEW(tv) || dict == NULL) {
		g_printerr("[E] Failed to enable spell checker, invalid function argument(s).\n");
		return;
	}

	GtkTextBuffer *tb = gtk_text_view_get_buffer(tv);
	if (tb == NULL) return;

	detach_spell_check(tb);

	SpellCheckData *data = g_new0(SpellCheckData, 1);
	data->dict = spell_dict_ref(dict);

	GtkTextTag *error_tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(tb), SPELL_ERROR_TAG_NAME);
	if (error_tag == NULL) {
		data->error_tag = gtk_text_buffer_create_tag(tb, SPELL_ERROR_TAG_NAME, "underline", PANGO_UNDERLINE_ERROR, NULL);
	} else {
		data->error_tag = error_tag;
	}

	GtkTextIter cursor;
	gtk_text_buffer_get_iter_at_mark(tb, &cursor, gtk_text_buffer_get_insert(tb));
	data->cursor_mark = gtk_text_buffer_create_mark(tb, NULL, &cursor, TRUE);

	data->insert_handler_id = g_signal_connect_after(tb, "insert-text", G_CALLBACK(on_text_inserted), data);
	data->delete_handler_id = g_signal_connect(tb, "delete-range", G_CALLBACK(on_delete_range), data);
	data->delete_after_handler_id = g_signal_connect_after(tb, "delete-range", G_CALLBACK(on_delete_range_after), data);
	data->cursor_handler_id = g_signal_connect(tb, "notify::cursor-position", G_CALLBACK(on_cursor_moved), data);
	g_object_set_data_full(G_OBJECT(tb), SPELL_CHECK_DATA_KEY, data, spell_check_data_free);

	spell_check_buffer_full(tb, data);
}

SpellDict *spell_dict_ref(SpellDict *dict)
{
	if (dict == NULL) return NULL;
	g_atomic_ref_count_inc(&dict->ref_count);
	return dict;
}

void spell_dict_unref(SpellDict *dict)
{
	if (dict == NULL) return;
	if (g_atomic_ref_count_dec(&dict->ref_count)) {
		g_clear_pointer(&dict->words, g_hash_table_unref);
		g_free(dict);
	}
}

SpellDict *spell_dict_new(const char *dict_path)
{
	if (dict_path == NULL || dict_path[0] == '\0') return NULL;

	gchar *contents = NULL;
	gsize length = 0;
	GError *error = NULL;

	if (!g_file_get_contents(dict_path, &contents, &length, &error)) {
		if (!g_error_matches(error, G_FILE_ERROR, G_FILE_ERROR_NOENT)) {
			g_warning("Failed to read the dictionary file '%s': %s", dict_path, error->message);
		}
		g_error_free(error);
		return NULL;
	}

	SpellDict *dict = g_new0(SpellDict, 1);
	g_atomic_ref_count_init(&dict->ref_count);
	dict->words = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	gchar *p = contents;
	gchar *end = contents + length;

	if (length >= 3 && (guchar)p[0] == 0xEF && (guchar)p[1] == 0xBB && (guchar)p[2] == 0xBF) {
		p += 3;
	}

	while (p < end) {
		gchar *nl = memchr(p, '\n', (size_t)(end - p));
		gchar *next = nl != NULL ? nl + 1 : end;
		size_t line_len = (size_t)((nl != NULL ? nl : end) - p);
		if (nl != NULL) *nl = '\0';

		if (memchr(p, '\0', line_len) == NULL) {
			g_strstrip(p);
			if (p[0] != '\0') {
				if (is_ascii_lowercase(p)) {
					g_hash_table_add(dict->words, g_strdup(p));
				} else if (g_utf8_validate(p, -1, NULL)) {
					char *normalized = normalize_word(p);
					if (normalized != NULL) {
						g_hash_table_add(dict->words, normalized);
					}
				}
			}
		}

		p = next;
	}

	g_free(contents);
	return dict;
}
