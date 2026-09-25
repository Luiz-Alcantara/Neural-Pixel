#ifndef CUSTOM_WIDGETS_H
#define CUSTOM_WIDGETS_H

GtkWidget* gen_const_dd(const char** items, int *def_item);

GtkWidget* gen_path_dd(const char* path, GtkTextBuffer *tb, int tb_type, GString *dd_item_str, GtkWidget *gen_btn, GApplication *app, int is_req);

GtkWidget* gen_visibility_toggle_button(const char *btn_text, const char *css_class_str, GtkWidget *wgt);

#endif // CUSTOM_WIDGETS_H
