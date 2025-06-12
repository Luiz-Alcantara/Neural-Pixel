#ifndef PNG_UTILS_H
#define PNG_UTILS_H

static void read_png_metadata(GObject* client, GAsyncResult* res, gpointer user_data);

static void set_file_path(GObject* client, GAsyncResult* res, gpointer user_data);

static void read_png_metadata_deprecated(GtkDialog* dialog, int response, gpointer user_data);

static void set_file_path_deprecated(GObject* client, GAsyncResult* res, gpointer user_data);

void load_from_img_btn_cb(GtkWidget *btn, gpointer user_data);

void load_img2img_btn_cb(GtkWidget *btn, gpointer user_data);

#endif // PNG_UTILS_H
