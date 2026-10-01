#ifndef PNG_UTILS_H
#define PNG_UTILS_H

int get_png_dimensions(const char *filename, uint32_t *w, uint32_t *h);

void load_metadata_from_favorites(GtkWidget *btn, gpointer user_data);

void load_from_img_preview(GtkWidget *btn, gpointer user_data);

void load_from_img_btn_cb(GtkWidget *btn, gpointer user_data);

void load_img2img_btn_cb(GtkWidget *btn, gpointer user_data);

void set_current_preview_to_img2img(GtkWidget *btn, gpointer user_data);

#endif // PNG_UTILS_H
