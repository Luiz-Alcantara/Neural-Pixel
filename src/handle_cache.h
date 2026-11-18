#ifndef HANDLE_CACHE_H
#define HANDLE_CACHE_H

#include "structs.h"

char* ini_file_get_value(const char *filename, const char *search_key);

void load_cache_fallback(gpointer user_data);

void load_cache(gpointer user_data);

void load_prompt_text(GtkTextBuffer *text_buffer, const char *prompt_file_path, const char *default_prompt);

void update_cache(GenerationSnapshotData *data);

#endif // HANDLE_CACHE_H
