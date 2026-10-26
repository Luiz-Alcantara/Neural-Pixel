#include <gtk/gtk.h>
#include <pango/pango.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <string.h>
#include "str_utils.h"
#include "structs.h"

static gboolean is_lora_key(gunichar key, gpointer data)
{
	return key == '<' || key == '>';
}

static void check_word(GtkTextBuffer *tb, const GtkTextIter *si, const GtkTextIter *ei, SpellCheckData *data)
{
	char *word = gtk_text_buffer_get_text(tb, si, ei, FALSE);
	gboolean misspelled = FALSE;

	if (word[0] != '\0' && !check_word_contains_num(word)) {
		char *key_to_check = normalize_word(word);
		misspelled = !g_hash_table_contains(data->dict, key_to_check);
		g_free(key_to_check);
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
	GtkTextIter insert_pos;
	gtk_text_buffer_get_iter_at_mark(tb, &insert_pos, gtk_text_buffer_get_insert(tb));

	GtkTextIter scan_pos = *range_s, gap_start = *range_s, lora_key_pos = *range_s;
	
	gboolean inside_angle = FALSE;
	if (!gtk_text_iter_backward_find_char(&lora_key_pos, is_lora_key, NULL, NULL)) {
		inside_angle = FALSE;
	} else {
		inside_angle = gtk_text_iter_get_char(&lora_key_pos) == '<';
	}

	while (gtk_text_iter_compare(&scan_pos, range_e) < 0) {
		gunichar current_char = gtk_text_iter_get_char(&scan_pos);

		if (inside_angle) {
			if (current_char == '>') { inside_angle = FALSE; }
			gtk_text_iter_forward_char(&scan_pos);
			continue;
		}
		if (current_char == '<') {
			inside_angle = TRUE;
			gtk_text_iter_forward_char(&scan_pos);
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
			if (current_char == '\'' || current_char == 0x2019 || current_char == '-') {
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
		if (gtk_text_iter_compare(&gap_start, &word_start) < 0) gtk_text_buffer_remove_tag(tb, data->error_tag, &gap_start, &word_start);
		gap_start = word_end;

		if (skip_word &&
		    gtk_text_iter_compare(&insert_pos, &word_start) > 0 &&
		    gtk_text_iter_compare(&insert_pos, &word_end) <= 0) {
			continue;
		}
		check_word(tb, &word_start, &word_end, data);
	}

	if (gtk_text_iter_compare(&gap_start, range_e) < 0) gtk_text_buffer_remove_tag(tb, data->error_tag, &gap_start, range_e);
}

static void check_span(GtkTextBuffer *tb, const GtkTextIter *range_s, const GtkTextIter *range_e, SpellCheckData *data, gboolean skip_word)
{
	GtkTextIter word_start = *range_s, word_end = *range_e;
	GtkTextIter temp_iter = word_start;
	
	while (gtk_text_iter_backward_char(&temp_iter)) {
		gunichar key = gtk_text_iter_get_char(&temp_iter);
		if (!g_unichar_isalnum(key) && key != '\'' && key != 0x2019 && key != '-') break;
		word_start = temp_iter;
	}

	while (!gtk_text_iter_is_end(&word_end)) {
		gunichar key = gtk_text_iter_get_char(&word_end);
		if (!g_unichar_isalnum(key) && key != '\'' && key != 0x2019 && key != '-') break;
		gtk_text_iter_forward_char(&word_end);
	}
	
	check_range(tb, &word_start, &word_end, data, skip_word);
}

static void on_text_inserted(GtkTextBuffer *tb, GtkTextIter *pos, const gchar *text, gint len, gpointer user_data)
{
	SpellCheckData *data = user_data;
	size_t n_bytes = len < 0 ? strlen(text) : (size_t)len;
	glong n_chars = g_utf8_strlen(text, n_bytes);
	if (n_chars == 0) return;

	GtkTextIter si = *pos;
	gtk_text_iter_backward_chars(&si, n_chars);

	if (memchr(text, '<', n_bytes) || memchr(text, '>', n_bytes)) {
		GtkTextIter bsi, bei;
		gtk_text_buffer_get_bounds(tb, &bsi, &bei);
		check_range(tb, &bsi, &bei, data, n_chars == 1);
		return;
	}

	check_span(tb, &si, pos, data, n_chars == 1);
}

static void on_delete_range(GtkTextBuffer *tb, GtkTextIter *range_s, GtkTextIter *range_e, gpointer user_data)
{
	SpellCheckData *data = user_data;
	char *text = gtk_text_buffer_get_text(tb, range_s, range_e, TRUE);
	if (strpbrk(text, "<>")) data->needs_full_check = TRUE;
	g_free(text);
}

static void on_delete_range_after(GtkTextBuffer *tb, GtkTextIter *range_s, GtkTextIter *range_e, gpointer user_data)
{
	SpellCheckData *data = user_data;

	if (data->needs_full_check) {
		data->needs_full_check = FALSE;
		GtkTextIter buf_s, buf_e;
		gtk_text_buffer_get_bounds(tb, &buf_s, &buf_e);
		check_range(tb, &buf_s, &buf_e, data, TRUE);
		return;
	}
	check_span(tb, range_s, range_s, data, TRUE);
}

static void on_cursor_moved(GtkTextBuffer *tb, GParamSpec *pspec, gpointer user_data)
{
	SpellCheckData *data = user_data;
	GtkTextIter old_pos, cur_pos;
	gtk_text_buffer_get_iter_at_mark(tb, &old_pos, data->cursor_mark);
	gtk_text_buffer_get_iter_at_mark(tb, &cur_pos, gtk_text_buffer_get_insert(tb));
	if (gtk_text_iter_equal(&old_pos, &cur_pos)) return;

	gtk_text_buffer_move_mark(tb, data->cursor_mark, &cur_pos);
	check_span(tb, &old_pos, &old_pos, data, TRUE);
}

void spell_check_buffer_full(GtkTextBuffer *tb, SpellCheckData *data)
{
	GtkTextIter buf_s, buf_e;
	gtk_text_buffer_get_bounds(tb, &buf_s, &buf_e);
	check_range(tb, &buf_s, &buf_e, data, FALSE);
}

void tv_enable_spell_check(GtkTextView *tv, SpellCheckData *data)
{
	GtkTextBuffer *tb = gtk_text_view_get_buffer(tv);

	data->error_tag = gtk_text_buffer_create_tag(tb, "spell-error", "underline", PANGO_UNDERLINE_ERROR, NULL);

	GtkTextIter cursor;
	gtk_text_buffer_get_iter_at_mark(tb, &cursor, gtk_text_buffer_get_insert(tb));
	data->cursor_mark = gtk_text_buffer_create_mark(tb, NULL, &cursor, TRUE);

	g_signal_connect_after(tb, "insert-text", G_CALLBACK(on_text_inserted), data);
	g_signal_connect(tb, "delete-range", G_CALLBACK(on_delete_range), data);
	g_signal_connect_after(tb, "delete-range", G_CALLBACK(on_delete_range_after), data);
	g_signal_connect(tb, "notify::cursor-position", G_CALLBACK(on_cursor_moved), data);

	spell_check_buffer_full(tb, data);
}

SpellCheckData *spell_checker_new(const char *dict_path)
{
	if (g_file_test(dict_path, G_FILE_TEST_EXISTS) == FALSE) return NULL;

	SpellCheckData *data = g_new0(SpellCheckData, 1);
	data->dict = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	FILE *dict_file = fopen(dict_path, "r");
	if (dict_file) {
		char line[128];
		while (fgets(line, sizeof(line), dict_file)) {
			line[strcspn(line, "\r\n")] = 0;
			if (line[0] != '\0' && g_utf8_validate(line, -1, NULL)) g_hash_table_add(data->dict, normalize_word(line));
		}
		fclose(dict_file);
	} else {
		g_warning("Failed to open the dictionary file: '%s'.", dict_path);
		return NULL;
	}

	return data;
}
