#ifndef HANDLE_CACHE_H
#define HANDLE_CACHE_H

#include "structs.h"

void create_cache(char *n, GError **error);

char* ini_file_get_value(const char *filename, const char *search_key);

void load_pp_cache(GtkTextBuffer *pos_tb);

void load_np_cache(GtkTextBuffer *neg_tb);

void load_cache_fallback(gpointer user_data);

void load_cache(gpointer user_data);

void update_cache(GenerationSnapshotData *data);

#endif // HANDLE_CACHE_H
