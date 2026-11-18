#include <gtk/gtk.h>
#include <gio/gio.h>

#include "constants.h"
#include "file_utils.h"
#include "png_utils.h"
#include "str_utils.h"
#include "structs.h"
#include "toast_notification.h"
#include "widgets_cb.h"

static void preview_set_empty_placeholder(GtkWidget *preview_widget, GdkTexture *texture)
{
	if (!preview_widget || !texture) return;
	gtk_picture_set_paintable(GTK_PICTURE(preview_widget), GDK_PAINTABLE(texture));
}

static void preview_cancel_pending_load(PreviewImageData *data)
{
	if (!data) return;

	if (data->load_cancellable) {
		g_cancellable_cancel(data->load_cancellable);
		g_clear_object(&data->load_cancellable);
	}
}

static void update_preview_label(PreviewImageData *data)
{
	if (!data || !data->image_files || !data->img_index_string || !data->img_index_label) return;

	guint img_count = data->image_files->len;
	g_string_erase(data->img_index_string, 0, -1);

	if (img_count == 0) {
		*data->current_image_index = 0;
		g_string_append(data->img_index_string, "0 / 0");
	} else {
		if (*data->current_image_index < 0 || (guint)*data->current_image_index >= img_count) *data->current_image_index = 0;
		const gchar *current_image_path = g_ptr_array_index(data->image_files, *data->current_image_index);
		gchar *basename = current_image_path ? g_path_get_basename(current_image_path) : g_strdup("");
		g_string_append_printf(data->img_index_string, "(%d / %u) %s", *data->current_image_index + 1, img_count, basename);
		g_free(basename);
	}

	gtk_label_set_label(GTK_LABEL(data->img_index_label), data->img_index_string->str);
}

static void load_preview_image_thread(GTask *task, gpointer source_obj, gpointer task_data, GCancellable *task_can)
{
	const char *img_path = (const char *)task_data;
	GError *error = NULL;

	if (g_cancellable_is_cancelled(task_can)) {
		g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_CANCELLED, "Load cancelled");
		return;
	}

	GFile *img_file = g_file_new_for_path(img_path);
	GFileInputStream *img_file_stream = g_file_read(img_file, task_can, &error);
	g_object_unref(img_file);
	if (!img_file_stream) {
		g_task_return_error(task, error);
		return;
	}

	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_stream(G_INPUT_STREAM(img_file_stream), task_can, &error);
	g_object_unref(img_file_stream);
	if (!pixbuf) {
		g_task_return_error(task, error);
		return;
	}

	if (g_cancellable_is_cancelled(task_can)) {
		g_object_unref(pixbuf);
		g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_CANCELLED, "Load cancelled");
		return;
	}

	GdkTexture *texture = get_texture_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);

	g_task_return_pointer(task, texture, g_object_unref);
}

static void on_preview_image_loaded_cb(GObject *source_obj, GAsyncResult *res, gpointer user_data)
{
	GTask *task = G_TASK(res);
	const char *img_path = g_task_get_task_data(task);
	GCancellable *task_can = g_task_get_cancellable(task);
	GError *error = NULL;

	GdkTexture *texture = g_task_propagate_pointer(task, &error);
	PreviewImageData *data = g_object_get_data(source_obj, "preview-data");

	if (data && data->image_widget && !g_cancellable_is_cancelled(task_can) && task_can == data->load_cancellable) {
		gboolean is_hidden = (g_strcmp0(gtk_button_get_icon_name(GTK_BUTTON(data->hide_img_btn)), "view-conceal-symbolic") == 0);
		gboolean is_current = FALSE;

		if (data->image_files && data->image_files->len > 0 &&
		*data->current_image_index >= 0 &&
		(guint)*data->current_image_index < data->image_files->len) {
			const char *current_path = g_ptr_array_index(data->image_files, *data->current_image_index);
			if (g_strcmp0(current_path, img_path) == 0) is_current = TRUE;
		}

		if (!is_hidden && is_current) {
			if (texture) {
				gtk_picture_set_paintable(GTK_PICTURE(data->image_widget), GDK_PAINTABLE(texture));
			} else {
				preview_set_empty_placeholder(data->image_widget, data->empty_texture);
				if (error && !g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
					g_printerr("Failed to load file: '%s'. Error: %s\n", img_path ? img_path : "(null)", error->message);
				}
			}
		}
	}

	if (texture) g_clear_object(&texture);
	if (error) g_clear_error(&error);
}

static void preview_load_current_image(PreviewImageData *data)
{
	if (!data || !data->image_files || !data->image_widget) return;

	preview_cancel_pending_load(data);

	if (g_strcmp0(gtk_button_get_icon_name(GTK_BUTTON(data->hide_img_btn)), "view-conceal-symbolic") == 0) {
		gtk_button_set_icon_name(GTK_BUTTON(data->hide_img_btn), "view-reveal-symbolic");
	}

	update_preview_label(data);

	if (data->image_files->len == 0) {
		gtk_picture_set_filename(GTK_PICTURE(data->image_widget), DEFAULT_IMG_PATH);
		return;
	}

	const gchar *current_image_path = g_ptr_array_index(data->image_files, *data->current_image_index);

	preview_set_empty_placeholder(data->image_widget, data->loading_texture);

	if (!current_image_path || !check_file_exists(current_image_path, 0)) {
		g_printerr("Failed to load file: '%s'.\n", current_image_path ? current_image_path : "(null)");
		return;
	}

	data->load_cancellable = g_cancellable_new();
	GTask *task = g_task_new(data->image_widget, data->load_cancellable, on_preview_image_loaded_cb, NULL);
	g_task_set_task_data(task, g_strdup(current_image_path), g_free);
	g_task_run_in_thread(task, load_preview_image_thread);
	g_object_unref(task);
}

void preview_image_visibility_toggle(GtkButton *btn, gpointer user_data)
{
	PreviewImageData *data = user_data;
	if (!data || !data->image_widget) return;
	
	if (g_strcmp0 (gtk_button_get_icon_name(btn), "view-reveal-symbolic") == 0) {
		preview_cancel_pending_load(data);
		gtk_button_set_icon_name (btn, "view-conceal-symbolic");
		preview_set_empty_placeholder(data->image_widget, data->empty_texture);
	} else {
		gtk_button_set_icon_name (btn, "view-reveal-symbolic");
		preview_load_current_image(data);
	}
}

void navigate_preview(PreviewImageData *data, int offset)
{
	if (!data || !data->image_files) return;

	guint img_count = data->image_files->len;

	if (img_count == 0) {
		g_printerr("Error: No images found.\n");
		return;
	}

	int count = (int)img_count;
	int current_index = *data->current_image_index;
	if (current_index < 0 || current_index >= count) {
		current_index = 0;
	}

	int step = offset % count;
	int new_index = (current_index + step) % count;

	if (new_index < 0) {
		new_index += count;
	}

	*data->current_image_index = new_index;
	preview_load_current_image(data);
}

static void on_send_to_trash_finish(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
	SendTrashData *trash_d = user_data;
	GFile *file = G_FILE(source_object);
	GError *error = NULL;

	gboolean ui_alive = (trash_d->hide_img_btn != NULL);
	if (ui_alive) {
		g_object_remove_weak_pointer(G_OBJECT(trash_d->hide_img_btn), (gpointer *)&trash_d->hide_img_btn);
	}

	if (!g_file_trash_finish(file, res, &error)) {
		g_printerr("Error trashing file '%s': %s\n",
			trash_d->trashed_file_path ? trash_d->trashed_file_path : "(null)",
			error ? error->message : "Unknown error"
		);
		g_clear_error(&error);

		if (ui_alive && trash_d->preview_d) {
			PreviewImageData *data = trash_d->preview_d;
			
			if (data->main_overlay) show_toast_notification(GTK_OVERLAY(data->main_overlay), "Failed to move image to trash.");

			if (trash_d->trashed_file_path && check_file_exists(trash_d->trashed_file_path, 0) == 1) {
				guint existing_idx = 0;
				if (!g_ptr_array_find_with_equal_func(data->image_files, trash_d->trashed_file_path, string_equal, &existing_idx)) {
					gchar *current_path = NULL;
					if (data->image_files->len > 0 &&
					*data->current_image_index >= 0 &&
					(guint)*data->current_image_index < data->image_files->len) {
						current_path = g_strdup(g_ptr_array_index(data->image_files, *data->current_image_index));
					}

					g_ptr_array_add(data->image_files, g_strdup(trash_d->trashed_file_path));
					g_ptr_array_sort(data->image_files, (GCompareFunc)compare_strings);

					if (current_path) {
						guint found_idx = 0;
						if (g_ptr_array_find_with_equal_func(data->image_files, current_path, string_equal, &found_idx)) {
							*data->current_image_index = (gint)found_idx;
						}
						g_free(current_path);
						update_preview_label(data);
					} else {
						*data->current_image_index = 0;
						preview_load_current_image(data);
					}
				}
			}
		}
	} else {
		printf("File moved to trash successfully.\n");

		if (ui_alive && trash_d->preview_d && trash_d->trashed_file_path) {
			PreviewImageData *data = trash_d->preview_d;
			guint stale_idx = 0;
			if (g_ptr_array_find_with_equal_func(data->image_files, trash_d->trashed_file_path, string_equal, &stale_idx)) {
				gboolean was_current = ((gint)stale_idx == *data->current_image_index);
				g_ptr_array_remove_index(data->image_files, stale_idx);

				if (data->image_files->len == 0) {
					*data->current_image_index = 0;
					preview_load_current_image(data);
				} else if (was_current) {
					if ((guint)*data->current_image_index >= data->image_files->len) {
						*data->current_image_index = (gint)data->image_files->len - 1;
					}
					preview_load_current_image(data);
				} else {
					if ((gint)stale_idx < *data->current_image_index) (*data->current_image_index)--;
					update_preview_label(data);
				}
			}
		}
	}

	g_free(trash_d->trashed_file_path);
	g_free(trash_d);
}

void send_to_trash(gpointer user_data)
{
	PreviewImageData *data = user_data;
	if (!data || !data->image_files) return;

	guint img_count = data->image_files->len;
	if (img_count == 0) {
		g_printerr("Error: There are no images in the 'output' directory.\n");
		return;
	}

	if (*data->current_image_index < 0 || (guint)*data->current_image_index >= img_count) {
		*data->current_image_index = 0;
	}

	guint remove_index = (guint)*data->current_image_index;
	gchar *file_to_trash_path = g_strdup(g_ptr_array_index(data->image_files, remove_index));
	if (!file_to_trash_path) return;

	gboolean exists = (check_file_exists(file_to_trash_path, 0) == 1);

	g_ptr_array_remove_index(data->image_files, remove_index);

	if (data->image_files->len == 0) {
		*data->current_image_index = 0;
	} else if ((guint)*data->current_image_index >= data->image_files->len) {
		*data->current_image_index = (gint)data->image_files->len - 1;
	}

	preview_load_current_image(data);

	if (!exists) {
		g_printerr("Error: File not found: '%s'.\n", file_to_trash_path);
		g_free(file_to_trash_path);
		return;
	}

	GFile *file_to_trash = g_file_new_for_path(file_to_trash_path);

	SendTrashData *trash_d = g_new0 (SendTrashData, 1);
	trash_d->trashed_file_path = file_to_trash_path;
	trash_d->preview_d = data;
	trash_d->hide_img_btn = data->hide_img_btn;
	g_object_add_weak_pointer(G_OBJECT(trash_d->hide_img_btn), (gpointer *)&trash_d->hide_img_btn);

	g_file_trash_async(file_to_trash, G_PRIORITY_DEFAULT, NULL, (GAsyncReadyCallback)on_send_to_trash_finish, trash_d);
	g_object_unref(file_to_trash);
}
