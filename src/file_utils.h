#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <gtk/gtk.h>

int check_file_exists(const char *filename, int is_text_file);

int check_create_base_dirs(void);

GtkStringList *get_files(const char *path, GError **error);

void get_png_files(GPtrArray *image_files);

char *get_unique_filepath(char *path);

void set_current_image_index(const char *img_str, GString *img_index_string, GPtrArray *image_files, gint *current_image_index, int total_time);

#endif
