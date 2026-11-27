#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#ifdef _WIN32
	#include <windows.h>
	#include <process.h>
#else
	#include <unistd.h>
#endif
#include "constants.h"
#include "custom_widgets.h"

gchar *ascii_format_double(const char *format, double value)
{
	gchar result[G_ASCII_DTOSTR_BUF_SIZE];
	g_ascii_formatd(result, sizeof(result), format, value);
	return g_strdup(result);
}

int check_file_has_line(const char *file_path, const char *line_to_search)
{
	FILE *fp = g_fopen(file_path, "r");
	if (!fp) return 0;

	size_t cap = strlen(line_to_search) + 3;
	char *buf = malloc(cap);
	if (!buf) { fclose(fp); return 0; }

	int found = 0;

	while (fgets(buf, (int)cap, fp)) {
		size_t len = strlen(buf);

		if (len > 0 && buf[len - 1] == '\n') {
			buf[--len] = '\0';
			if (len > 0 && buf[len - 1] == '\r') buf[--len] = '\0';
		} else if (!feof(fp)) {
			int c;
			while ((c = getc(fp)) != EOF && c != '\n')
				;
			continue;
		}

		if (strcmp(buf, line_to_search) == 0) {
			found = 1;
			break;
		}
	}

	free(buf);
	fclose(fp);
	return found;
}

int check_list_contains_item(const char* const* list, const char* item)
{
	int i = 0;
	while (list[i] != NULL) {
		if (strcmp(list[i], item) == 0) {
			return i;
		}
		i++;
	}
	return 0;
}

int check_gtk_list_contains_item(GtkStringList *list, const char *item)
{
	if (!G_IS_LIST_MODEL(list) || item == NULL) { return 0; }

	gboolean has_ext = (strchr(item, '.') != NULL);
	size_t item_len = strlen(item);
	guint n = g_list_model_get_n_items(G_LIST_MODEL(list));

	for (guint i = 0; i < n; i++) {
		const char *s = gtk_string_list_get_string(list, i);
		if (has_ext) {
			if (strcmp(s, item) == 0) { return i; }
		} else {
			const char *dot = strrchr(s, '.');
			size_t base_len = dot ? (size_t)(dot - s) : strlen(s);
			if (base_len == item_len && strncmp(s, item, base_len) == 0) { return i; }
		}
	}
	return 0;
}

gboolean check_word_contains_num(const char *word)
{
	for (int i = 0; word[i] != '\0'; i++) {
		if (g_ascii_isdigit(word[i])) return TRUE;
	}
	return FALSE;
}

int compare_strings(const void *a, const void *b)
{
	return strcasecmp(*(const char **)a, *(const char **)b);
}

int count_digits(double n)
{
	long long int i = (int)n;
	if (i < 0) {
		i = -i;
	}
	int l = 0;
	if (i == 0) {
		l = 1;
	} else {
		while (i > 0) {
			i /= 10;
			l++;
		}
	}
	return l;
}

char *convert_int_to_string(int x)
{
	char* str = malloc(12);

	if (str == NULL) {
	return NULL;
	}

	snprintf(str, 12, "%d", x);
	return str;
}

char *convert_double_to_string(double n, char *str_format)
{
	int str_size = 0;

	if (strcmp(str_format, "%.2f") == 0) {
		str_size = count_digits(n) + 4;
	}
	if (strcmp(str_format, "%.1f") == 0) {
		str_size = count_digits(n) + 3;
	}
	if (strcmp(str_format, "%.0f") == 0) {
		if (n < 0) {
			str_size = count_digits(n) + 2;
		} else {
			str_size = count_digits(n) + 1;
		}
	}

	char *str = malloc(str_size * sizeof(char));
	snprintf(str, str_size, str_format, n);
	return str;
}

char *convert_long_long_int_to_string(long long int x)
{
	char *str = malloc(LONGLONG_STR_SIZE);
	if (!str) {
		return NULL;
	}

	int success = snprintf(str, LONGLONG_STR_SIZE, "%lld", x);
	if (success < 0 || success >= LONGLONG_STR_SIZE) {
		free(str);
		return NULL;
	}
	return str;
}

#include <math.h>

char *extract_adapters_from_prompt(GtkBox *box_model_adapters_cards, int adapter_type, int negative_embedding, const char *prompt)
{
	if (prompt == NULL) return NULL;
	if (box_model_adapters_cards == NULL) return g_strdup(prompt);

	const char *open_tag = adapter_type ? "<lora:" : "(embedding:";
	const char close_chr = adapter_type ? '>' : ')';
	const size_t open_len = strlen(open_tag);

	GString *out = g_string_new(NULL);
	const char *p = prompt;
	const char *tag;

	while ((tag = strstr(p, open_tag)) != NULL) {
		const char *start = tag + open_len;
		const char *end = strchr(start, close_chr);
		if (end == NULL) break;

		g_string_append_len(out, p, tag - p);

		while (out->len > 0 && g_ascii_isspace(out->str[out->len - 1])) {
			g_string_truncate(out, out->len - 1);
		}
		gboolean had_before = out->len > 0;
		gboolean comma_before = had_before && out->str[out->len - 1] == ',';

		const char *colon = memchr(start, ':', end - start);
		const char *name_end = colon ? colon : end;

		g_autofree char *name = g_strndup(start, name_end - start);
		g_strstrip(name);

		double value = 1.0;
		if (colon != NULL) {
			char *num_end;
			double v = g_ascii_strtod(colon + 1, &num_end);
			if (num_end != colon + 1 && isfinite(v)) value = v;
		}

		if (*name != '\0') {
			if (!adapter_exists(GTK_WIDGET(box_model_adapters_cards), name, adapter_type)) {
				add_adapter_card(box_model_adapters_cards, name, value, adapter_type, negative_embedding);
			}
		}

		p = end + 1;

		const char *next = p;
		while (*next != '\0' && g_ascii_isspace(*next)) next++;
		gboolean at_end = (*next == '\0');
		gboolean comma_after = (*next == ',');

		if (comma_before && comma_after) p = next + 1;

		if (had_before && !at_end && !(comma_before) && *next != ',') {
			g_string_append_c(out, ' ');
		} else if (comma_before && !at_end && !comma_after) {
			g_string_append_c(out, ' ');
		}

		while (*p != '\0' && g_ascii_isspace(*p)) p++;
	}

	g_string_append(out, p);

	while (out->len > 0 && (g_ascii_isspace(out->str[out->len - 1]) || out->str[out->len - 1] == ',')) {
		g_string_truncate(out, out->len - 1);
	}

	gsize lead = 0;
	while (lead < out->len && (g_ascii_isspace(out->str[lead]) || out->str[lead] == ',')) lead++;
	if (lead > 0) g_string_erase(out, 0, lead);

	return g_string_free(out, FALSE);
}

char* generate_sd_seed()
{
	long long int seed;

	#ifdef _WIN32
		LARGE_INTEGER counter;
		if (QueryPerformanceCounter(&counter)) {
			seed = (long long int)counter.QuadPart;
		} else {
			seed = (long long int)time(NULL);
			seed ^= (long long int)GetCurrentThreadId() << 16;
		}
		seed ^= (long long int)GetCurrentProcessId() << 32;
	#else
		struct timespec ts;
		if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
			seed = (long long int)ts.tv_sec * 1000000000LL;
			seed += (long long int)ts.tv_nsec;
		} else {
			seed = (long long int)time(NULL);
		}
		seed ^= (long long int)getpid() << 32;
	#endif

	seed ^= seed >> 33;
	seed *= 0xFF51AFD7ED558CCD;
	seed ^= seed >> 33;
	seed *= 0xC4CEB9FE1A85EC53;
	seed ^= seed >> 33;

	seed &= 0x7FFFFFFFFFFFFFFF;

	char *seed_str = malloc(LONGLONG_STR_SIZE);
	if (!seed_str) {
		return NULL;
	}

	int chars_written = snprintf(seed_str, LONGLONG_STR_SIZE, "%lld", seed);

	if (chars_written < 0 || chars_written >= LONGLONG_STR_SIZE) {
		free(seed_str);
		return NULL;
	}

	return seed_str;
}

char *get_time_str()
{
	time_t rt;
	struct tm timeinfo;

	time(&rt);

	#ifdef _WIN32
		if (localtime_s(&timeinfo, &rt) != 0) {
			return NULL;
		}
	#else
		if (localtime_r(&rt, &timeinfo) == NULL) {
			return NULL;
		}
	#endif

	char *t = malloc(16);
	if (t == NULL) {
	return NULL;
	}

	if (strftime(t, 16, "%Y%m%d_%H%M%S", &timeinfo) == 0) {
	free(t);
	return NULL;
	}

	return t;
}

gboolean is_string_blank(const gchar *str)
{
	if (str == NULL) return TRUE;

	for (const gchar *p = str; *p != '\0'; p++) {
		if (!g_ascii_isspace(*p)) { return FALSE; }
	}
	return TRUE;
}

gchar *load_lora_triggers(const char *triggers_file_path)
{
	gchar *file_cont = NULL;
	gsize cont_len = 0;
	GError *error = NULL;

	if (!g_file_get_contents(triggers_file_path, &file_cont, &cont_len, &error)) {
		g_printerr("Failed to load '%s': %s.\n", triggers_file_path, error->message);
		g_error_free(error);
		return g_strdup("");
	}

	if (!g_utf8_validate(file_cont, cont_len, NULL)) {
		g_printerr("File '%s' is not valid UTF-8.\n", triggers_file_path);
		g_free(file_cont);
		return g_strdup("");
	}

	return file_cont;
}

char *normalize_word(const char *word)
{
	char *word_lowercase = g_utf8_strdown(word, -1);
	GString *final_word = g_string_new(NULL);
	for (const char *char_ptr = word_lowercase; *char_ptr; char_ptr = g_utf8_next_char(char_ptr)) {
		gunichar fixed_char = g_utf8_get_char(char_ptr);
		g_string_append_unichar(final_word, fixed_char == 0x2019 ? '\'' : fixed_char);
	}
	g_free(word_lowercase);
	return g_string_free(final_word, FALSE);
}

gboolean seed_entry_get_value(GtkEditable *editable, gint64 *out_value)
{
	const char *text = gtk_editable_get_text (editable);

	if (text[0] == '\0' || g_strcmp0 (text, "-") == 0) {
		*out_value = -1;
		return FALSE;
	}

	if (!g_ascii_string_to_signed (text, 10, -1, G_MAXINT64, out_value, NULL)) {
		*out_value = -1;
		return FALSE;
	}

	return TRUE;
}

gboolean string_equal(gconstpointer a, gconstpointer b)
{
	return g_strcmp0(a, b) == 0;
}

char *strip_model_extension(const char *filename)
{
	static const char *model_exts[] = {
		".safetensors",
		".sft",
		".ckpt",
		".pt",
		".pth",
		".bin",
		".gguf",
		".onnx",
		".pkl",
		".vae",
		NULL
	};

	char *result = g_strdup(filename);
	if (result == NULL) return NULL;

	gsize len = strlen(result);

	for (int i = 0; model_exts[i] != NULL; i++) {
		gsize ext_len = strlen(model_exts[i]);

		if (len > ext_len && g_ascii_strcasecmp(result + len - ext_len, model_exts[i]) == 0) {
			result[len - ext_len] = '\0';
			break;
		}
	}

	return result;
}
