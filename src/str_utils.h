#ifndef STR_UTILS_H
#define STR_UTILS_H

int check_list_contains_item(const char* const* list, const char* item);

int check_gtk_list_contains_item(GtkStringList *list, const char* item);

int compare_strings(const void *a, const void *b);

int count_digits(double n);

char *convert_int_to_string(int value);

char *convert_double_to_string(double n, char *str_format);

char *convert_long_long_int_to_string(long long int x);

char *format_lora_embedding_string(const gchar* item, int tb_type);

char* generate_sd_seed();

char *get_time_str();

#endif // STR_UTILS_H
