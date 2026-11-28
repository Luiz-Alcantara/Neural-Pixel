#ifndef CUSTOM_WIDGETS_H
#define CUSTOM_WIDGETS_H

#include <gtk/gtk.h>

void factory_setup_cb (GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data);

void factory_bind_cb (GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data);

GtkWidget *gen_checkpoint_dd(const char* path, GString *dd_item_str, GtkWidget *gen_btn);

GtkWidget* gen_models_dd(const char* path, GString *dd_item_str);

GtkWidget* gen_const_dd(const char** items, int *def_item);

GtkWidget *gen_visibility_toggle_button(const char *btn_text, const char *css_class_str, GtkWidget *wgt);

#endif // CUSTOM_WIDGETS_H
