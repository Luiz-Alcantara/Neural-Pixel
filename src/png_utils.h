#ifndef PNG_UTILS_H
#define PNG_UTILS_H

int get_png_dimensions(const char *filename, uint32_t *w, uint32_t *h);

static void set_png_metadata(gchar *path, gpointer user_data);

static void read_png_metadata(GObject* client, GAsyncResult* res, gpointer user_data);

static void set_file_path(GObject* client, GAsyncResult* res, gpointer user_data);

static void read_png_metadata_deprecated(GtkDialog* dialog, int response, gpointer user_data);

static void set_file_path_deprecated(GtkDialog* dialog, int response, gpointer user_data);

void load_from_img_preview(GtkWidget *btn, gpointer user_data);

void load_from_img_btn_cb(GtkWidget *btn, gpointer user_data);

void load_img2img_btn_cb(GtkWidget *btn, gpointer user_data);

void set_current_preview_to_img2img(GtkWidget *btn, gpointer user_data);

#endif // PNG_UTILS_H
