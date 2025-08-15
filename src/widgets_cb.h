#ifndef WIDGETS_CB_H
#define WIDGETS_CB_H

void add_dropdown_selected_item_textview(GtkWidget* wgt, GParamSpec *pspec, gpointer user_data);

void app_start_data_free (gpointer user_data);

void array_strings_free(const char **list);

void clear_img2img_btn_cb (GtkWindow *wgt, gpointer user_data);

gboolean close_app_callback (GtkWindow *win, gpointer user_data);

void dropdown_items_update(const char *path, GtkWidget *dd, GApplication *app);

void free_cache_data(MyCacheData *s);

void free_preview_data(gpointer data);

void kill_stable_diffusion_process(GtkButton *btn, gpointer user_data);

void on_clear_img2img_btn_destroy (GtkWidget* wgt, gpointer user_data);

void on_dd_const_destroy (GtkWidget* wgt, gpointer user_data);

void on_dd_path_destroy (GtkWidget* wgt, gpointer user_data);

void on_dropdown_destroy(GtkWidget* wgt, gpointer user_data);

void on_generate_btn_destroy (GtkWidget* wgt, gpointer user_data);

void on_hide_img_btn_destroy (GtkWidget* wgt, gpointer user_data);

void on_load_from_img_btn_destroy (GtkWidget* wgt, gpointer user_data);

void on_reload_btn_destroy(GtkWidget* wgt, gpointer user_data);

void on_reset_default_btn_destroy (GtkWidget* wgt, gpointer user_data);

void quit_btn_callback (GtkWidget *wgt, GtkWidget *win);

void reload_dropdown(GtkWidget* wgt, gpointer user_data);

void reset_default_btn_cb (GtkWidget* btn, gpointer user_data);

gboolean seed_entry_int_filter(GtkEditable *editable, gpointer user_data);

void set_dropdown_selected_const_item(GtkWidget* wgt, GParamSpec *pspec, int *i1);

void set_dropdown_selected_item(GtkWidget* wgt, GParamSpec *pspec, gpointer user_data);

void set_spin_value_to_var(GtkWidget *w, double *v);

void random_seed_btn_toggle(GtkWidget *entry_wgt, GtkEntryIconPosition position, GdkEvent *event, gpointer user_data);

void show_error_message (GtkWidget *win, char *err_title_text, char *err_text);

void hide_img_btn_cb(GtkButton *btn, gpointer user_data);

void toggle_extra_options(GtkCheckButton *btn, gpointer user_data);

#endif // WIDGETS_CB_H
