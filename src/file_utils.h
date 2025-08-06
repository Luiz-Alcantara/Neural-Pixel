#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#endif

int is_file_empty(const char *fn);

int is_directory(const char *path);

int count_files(DIR* dir, const char * dir_path, const char* const* array);

int count_output_files();

int check_file_exists(char *filename, int is_text_file);

int has_files(const char *directory);

DIR* check_create_dir(const char* path);

int check_create_base_dirs();

GtkStringList* get_files(const char* path, GError **error);

#endif // FILE_UTILS_H
