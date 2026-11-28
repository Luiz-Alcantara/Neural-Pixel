#ifndef MODEL_ADAPTERS_H
#define MODEL_ADAPTERS_H

#include <gtk/gtk.h>

void add_adapter_card(GtkBox *box_model_adapters_cards, const char *name, double value, int adapter_type, int negative_embedding);

gboolean adapter_exists(GtkWidget *box, const char *adapter_name, int adapter_type);

GtkWidget *gen_adapter_dd(const char *path, GtkWidget *box_model_adapters_cards, int adapter_type);

#endif // MODEL_ADAPTERS_H
