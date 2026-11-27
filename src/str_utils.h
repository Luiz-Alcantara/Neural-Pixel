#ifndef STR_UTILS_H
#define STR_UTILS_H

#include <gtk/gtk.h>

gchar *ascii_format_double(const char *format, double value);

int check_file_has_line(const char *file_path, const char *line_to_search);

int check_list_contains_item(const char* const* list, const char* item);

int check_gtk_list_contains_item(GtkStringList *list, const char* item);

gboolean check_word_contains_num(const char *word);

int compare_strings(const void *a, const void *b);

int count_digits(double n);

char *convert_int_to_string(int value);

char *convert_double_to_string(double n, char *str_format);

char *convert_long_long_int_to_string(long long int x);

char *extract_adapters_from_prompt(GtkBox *box_model_adapters_cards, GtkWidget *adapter_dd, int adapter_type, int negative_embedding, const char *prompt);

char* generate_sd_seed();

char *get_time_str();

gboolean is_string_blank(const gchar *str);

gchar *load_lora_triggers(const char *triggers_file_path);

char *normalize_word(const char *word);

gboolean seed_entry_get_value(GtkEditable *editable, gint64 *out_value);

gboolean string_equal(gconstpointer a, gconstpointer b);

char *strip_model_extension(const char *filename);

#endif // STR_UTILS_H
