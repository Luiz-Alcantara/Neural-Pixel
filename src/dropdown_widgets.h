#ifndef DROPDOWN_WIDGETS_H
#define DROPDOWN_WIDGETS_H

static void factory_setup_cb (GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data);

static void factory_bind_cb (GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data);

GtkWidget* gen_const_dd(const char** items, int *def_item);

GtkWidget* gen_path_dd(const char* path, GtkTextBuffer *tb, int tb_type, GString *dd_item_str, GtkWidget *gen_btn, GApplication *app, int is_req);

#endif // DROPDOWN_WIDGETS_H
