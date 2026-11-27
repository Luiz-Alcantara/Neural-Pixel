#ifndef CUSTOM_WIDGETS_H
#define CUSTOM_WIDGETS_H

#include <gtk/gtk.h>

void add_adapter_card(GtkBox *box_model_adapters_cards, const char *name, double value, int adapter_type, int negative_embedding);

gboolean adapter_exists(GtkWidget *box, const char *adapter_name, int adapter_type);

GtkWidget* gen_const_dd(const char** items, int *def_item);

GtkWidget* gen_path_dd(const char* path, GtkWidget *box_model_adapters_inner, int adapter_type, GString *dd_item_str, GtkWidget *gen_btn, GApplication *app, int is_req);

GtkWidget* gen_visibility_toggle_button(const char *btn_text, const char *css_class_str, GtkWidget *wgt);

#endif // CUSTOM_WIDGETS_H
