#ifndef HANDLE_CACHE_H
#define HANDLE_CACHE_H

#include "structs.h"

char* ini_file_get_value(const char *filename, const char *search_key);

void load_cache_fallback(gpointer user_data);

void load_cache(gpointer user_data);

void load_prompt_text(GtkTextBuffer *text_buffer, GtkWidget *box_model_adapters_cards, GtkWidget *lora_dd, GtkWidget *embedding_dd, const char *prompt_file_path, const char *default_prompt, int negative);

void update_cache(GenerationSnapshotData *data);

#endif // HANDLE_CACHE_H
