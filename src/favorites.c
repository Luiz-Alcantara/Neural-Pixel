#include <gtk/gtk.h>
#include <gio/gio.h>
#include <string.h>
#include "constants.h"
#include "file_utils.h"
#include "png_utils.h"
#include "structs.h"
#include "widgets_cb.h"

#define IMG_DIMENSION 640

static void load_scaled_image_thread(GTask *task, gpointer source_obj, gpointer task_data, GCancellable *task_can);
static void on_delete_favorite(GtkButton *btn, gpointer user_data);
static void on_image_loaded_cb(GObject *source_obj, GAsyncResult *res, gpointer user_data);
static void save_favorites(GtkStringList *store);

static void favorites_factory_setup_cb(GtkSignalListItemFactory *factory, GtkListItem *item, gpointer user_data)
{
	FavoritesWindowData *favorites_d = user_data;

	gtk_list_item_set_selectable(item, FALSE);
	gtk_list_item_set_activatable(item, FALSE);

	GtkWidget *fav_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_widget_add_css_class(fav_card, "info_box");
	gtk_widget_set_margin_start(fav_card, SMALL_SPACING);
	gtk_widget_set_margin_end(fav_card, SMALL_SPACING);
	gtk_widget_set_margin_top(fav_card, MEDIUM_SPACING);
	gtk_widget_set_margin_bottom(fav_card, LARGE_SPACING);
	gtk_widget_set_valign(fav_card, GTK_ALIGN_START);

	GtkWidget *fav_image = gtk_picture_new();
	gtk_picture_set_can_shrink(GTK_PICTURE(fav_image), TRUE);
	gtk_picture_set_content_fit(GTK_PICTURE(fav_image), GTK_CONTENT_FIT_CONTAIN);

	GtkWidget *fav_image_frame = gtk_scrolled_window_new();
	gtk_widget_add_css_class(fav_image_frame, "img_preview_box");
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(fav_image_frame), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
	gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(fav_image_frame), IMG_DIMENSION);
	gtk_scrolled_window_set_max_content_width(GTK_SCROLLED_WINDOW(fav_image_frame), IMG_DIMENSION);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(fav_image_frame), IMG_DIMENSION);
	gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(fav_image_frame), IMG_DIMENSION);
	gtk_widget_set_size_request(fav_image_frame, IMG_DIMENSION, IMG_DIMENSION);
	gtk_widget_set_hexpand(fav_image_frame, FALSE);
	gtk_widget_set_vexpand(fav_image_frame, FALSE);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(fav_image_frame), fav_image);

	GtkWidget *box_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_buttons), TRUE);

	GtkWidget *load_info_btn = gtk_button_new_from_icon_name ("insert-image-symbolic");
	gtk_widget_add_css_class(load_info_btn, "custom_btn");
	gtk_widget_set_hexpand (load_info_btn, TRUE);
	gtk_widget_set_focusable(load_info_btn, FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(load_info_btn),
	"Load the prompt, model, sampler settings, and other parameters\nfrom the embedded metadata from this image.");
	g_object_set_data(G_OBJECT(load_info_btn), "favorites_d", favorites_d);
	g_signal_connect(load_info_btn, "clicked", G_CALLBACK(load_metadata_from_favorites), fav_image);
	gtk_box_append(GTK_BOX(box_buttons), load_info_btn);

	GtkWidget *delete_btn = gtk_button_new_from_icon_name ("edit-delete-symbolic");
	gtk_widget_add_css_class(delete_btn, "custom_btn");
	gtk_widget_set_hexpand (delete_btn, TRUE);
	gtk_widget_set_focusable(delete_btn, FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(delete_btn), "Remove this image from favorites.");
	g_object_set_data(G_OBJECT(delete_btn), "favorites_d", favorites_d);
	g_signal_connect(delete_btn, "clicked", G_CALLBACK(on_delete_favorite), item);
	gtk_box_append(GTK_BOX(box_buttons), delete_btn);

	gtk_box_append(GTK_BOX(fav_card), fav_image_frame);
	gtk_box_append (GTK_BOX (fav_card), box_buttons);

	g_object_set_data(G_OBJECT(fav_card), "fav-image", fav_image);

	gtk_list_item_set_child(item, fav_card);
}

static void favorites_factory_bind_cb(GtkSignalListItemFactory *factory, GtkListItem *item, gpointer user_data)
{
	GdkTexture *empty_texture = GDK_TEXTURE(user_data);
	GtkStringObject *obj = gtk_list_item_get_item(item);
	if (obj == NULL) return;
	
	const char *path = gtk_string_object_get_string(obj);
	GtkWidget *fav_card = gtk_list_item_get_child(item);
	GtkWidget *fav_image = g_object_get_data(G_OBJECT(fav_card), "fav-image");

	gtk_picture_set_paintable(GTK_PICTURE(fav_image), GDK_PAINTABLE(empty_texture));
	g_object_set_data_full(G_OBJECT(fav_image), "image-path", path ? g_strdup(path) : NULL, g_free);

	GCancellable *old_can = g_object_get_data(G_OBJECT(item), "load-cancellable");
	if (old_can) g_cancellable_cancel(old_can);
	g_object_set_data(G_OBJECT(item), "load-cancellable", NULL);

	if (!path || !check_file_exists(path, 0)) {
		g_printerr("Failed to load file: '%s'.\n", path ? path : "(null)");
		return;
	}

	GCancellable *new_can = g_cancellable_new();
	g_object_set_data_full(G_OBJECT(item), "load-cancellable", new_can, g_object_unref);

	GTask *task = g_task_new(item, new_can, on_image_loaded_cb, NULL);
	g_task_set_task_data(task, g_strdup(path), g_free);
	g_task_run_in_thread(task, load_scaled_image_thread);
	g_object_unref(task);
}

static void favorites_factory_unbind_cb(GtkSignalListItemFactory *factory, GtkListItem *item, gpointer user_data)
{
	GCancellable *item_can = g_object_get_data(G_OBJECT(item), "load-cancellable");
	if (item_can) {
		g_cancellable_cancel(item_can);
		g_object_set_data(G_OBJECT(item), "load-cancellable", NULL);
	}

	GtkWidget *fav_card = gtk_list_item_get_child(item);
	if (fav_card) {
		GtkWidget *fav_image = g_object_get_data(G_OBJECT(fav_card), "fav-image");
		if (fav_image) {
			gtk_picture_set_paintable(GTK_PICTURE(fav_image), NULL);
		}
	}
}

static void favorites_window_update(FavoritesWindowData *favorites_d)
{
	guint n = g_list_model_get_n_items(G_LIST_MODEL(favorites_d->store));
	const char *target_child = (n > 0) ? "list" : "empty";

	if (strcmp(gtk_stack_get_visible_child_name(GTK_STACK(favorites_d->stack)), target_child) != 0) {
		gtk_stack_set_visible_child_name(GTK_STACK(favorites_d->stack), target_child);
	}
}

static GtkStringList *load_favorites()
{
	GtkStringList *store = gtk_string_list_new(NULL);
	gchar *contents = NULL;

	check_file_exists(".cache/favorites", 1);

	if (!g_file_get_contents(".cache/favorites", &contents, NULL, NULL)) return store;

	gchar **lines = g_strsplit(contents, "\n", -1);

	for (gint i = 0; lines[i] != NULL; i++) {
		g_strstrip(lines[i]);

		if (*lines[i] != '\0') gtk_string_list_append(store, lines[i]);
	}

	g_strfreev(lines);
	g_free(contents);

	return store;
}

static void load_scaled_image_thread(GTask *task, gpointer source_obj, gpointer task_data, GCancellable *task_can)
{
	const char *path = (const char *)task_data;
	GError *error = NULL;

	if (g_cancellable_is_cancelled(task_can)) {
		g_task_return_new_error(task, G_IO_ERROR, G_IO_ERROR_CANCELLED, "Load cancelled");
		return;
	}

	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(path, IMG_DIMENSION, IMG_DIMENSION, TRUE, &error);
	if (!pixbuf) { g_task_return_error(task, error); return; }
	g_task_return_pointer(task, pixbuf, g_object_unref);
}

static void on_delete_favorite(GtkButton *btn, gpointer user_data)
{
	GtkListItem *item = GTK_LIST_ITEM(user_data);
	FavoritesWindowData *favorites_d = g_object_get_data(G_OBJECT(btn), "favorites_d");

	guint pos = gtk_list_item_get_position(item);
	if (pos == GTK_INVALID_LIST_POSITION) return;

	gtk_string_list_remove(favorites_d->store, pos);
	favorites_d->modified = TRUE;
}

static void on_favorites_changed(GListModel *model, guint position, guint removed, guint added, gpointer user_data)
{
	favorites_window_update(user_data);
}

static gboolean on_favorites_window_close_request(GtkWindow *window, gpointer user_data)
{
	FavoritesWindowData *favorites_d = user_data;
	if (favorites_d->modified) {
		save_favorites(favorites_d->store);
		favorites_d->modified = FALSE;
	}
	return FALSE;
}

static void on_favorites_window_destroy (gpointer user_data)
{
    FavoritesWindowData *data = user_data;
    g_object_unref(data->store);
    g_free(data);
}

static void on_image_loaded_cb(GObject *source_obj, GAsyncResult *res, gpointer user_data)
{
	GtkListItem *item = GTK_LIST_ITEM(source_obj);
	const char *img_path = g_task_get_task_data(G_TASK(res));
	GError *error = NULL;

	GdkPixbuf *pixbuf = g_task_propagate_pointer(G_TASK(res), &error);

	GtkStringObject *current_obj = gtk_list_item_get_item(item);
	gboolean is_same_row = FALSE;

	if (current_obj != NULL) {
		const char *current_path = gtk_string_object_get_string(current_obj);
		if (current_path && g_strcmp0(current_path, img_path) == 0) is_same_row = TRUE;
	}

	GtkWidget *fav_card = gtk_list_item_get_child(item);
	GtkWidget *fav_image = fav_card ? g_object_get_data(G_OBJECT(fav_card), "fav-image") : NULL;

	if (is_same_row && fav_image) {
		if (pixbuf) {
			GdkTexture *texture = gdk_texture_new_for_pixbuf(pixbuf);
			gtk_picture_set_paintable(GTK_PICTURE(fav_image), GDK_PAINTABLE(texture));
			g_object_unref(texture);
		} else {
			gtk_picture_set_filename(GTK_PICTURE(fav_image), EMPTY_IMG_PATH);
			if (error && !g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
				g_printerr("Failed to load file: '%s'. Error: %s\n", img_path, error->message);
			}
		}
	}

	if (pixbuf) g_object_unref(pixbuf);
	if (error) g_clear_error(&error);
}

static void save_favorites(GtkStringList *store)
{
	GString *out = g_string_new("");
	guint n = g_list_model_get_n_items(G_LIST_MODEL(store));

	for (guint i = 0; i < n; i++) {
		g_string_append(out, gtk_string_list_get_string(store, i));
		g_string_append_c(out, '\n');
	}

	GError *error = NULL;
	if (!g_file_set_contents(".cache/favorites", out->str, -1, &error)) {
		g_warning("Could not write %s: %s", ".cache/favorites", error->message);
		g_clear_error(&error);
	}

	g_string_free(out, TRUE);
}

void show_favorites_manager(GtkButton *btn, gpointer user_data)
{
	LoadPNGData *data = user_data;

	FavoritesWindowData *favorites_d = g_new0(FavoritesWindowData, 1);
	favorites_d->load_png_info_d = data;
	favorites_d->store = load_favorites();

	GError *error = NULL;
	GdkTexture *empty_texture = gdk_texture_new_from_filename(EMPTY_IMG_PATH, &error);
	if (!empty_texture) { g_printerr("Failed to load placeholder: %s\n", error->message); g_clear_error(&error); }

	GtkWidget *favorites_win = gtk_window_new();
	gtk_widget_add_css_class(favorites_win, "info_box");
	gtk_window_set_transient_for(GTK_WINDOW(favorites_win), GTK_WINDOW(data->win));
	gtk_window_set_title(GTK_WINDOW(favorites_win), "Favorites");
	gtk_window_set_default_size(GTK_WINDOW(favorites_win), 1280, 640);
	gtk_window_set_modal(GTK_WINDOW(favorites_win), TRUE);
	gtk_window_set_resizable (GTK_WINDOW(favorites_win), FALSE);
	gtk_window_set_deletable (GTK_WINDOW(favorites_win), TRUE);
	gtk_window_set_decorated (GTK_WINDOW(favorites_win), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(favorites_win), TRUE);

	g_object_set_data_full(G_OBJECT(favorites_win), "favorites-data", favorites_d, on_favorites_window_destroy);

	GtkListItemFactory *factory = gtk_signal_list_item_factory_new();
	g_signal_connect(factory, "setup", G_CALLBACK(favorites_factory_setup_cb), favorites_d);
	g_signal_connect_data(factory, "bind", G_CALLBACK(favorites_factory_bind_cb), empty_texture, (GClosureNotify)g_object_unref, 0);
	g_signal_connect(factory, "unbind", G_CALLBACK(favorites_factory_unbind_cb), NULL);

	GtkNoSelection *selection = gtk_no_selection_new(G_LIST_MODEL(g_object_ref(favorites_d->store)));

	GtkWidget *list_view = gtk_list_view_new(GTK_SELECTION_MODEL(selection), factory);
	gtk_widget_add_css_class(list_view, "favorites_listview");
	gtk_orientable_set_orientation(GTK_ORIENTABLE(list_view), GTK_ORIENTATION_HORIZONTAL);

	GtkWidget *scrolled = gtk_scrolled_window_new();
	gtk_widget_add_css_class(scrolled, "info_box");
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), list_view);

	GtkWidget *empty_label = gtk_label_new("No Favorites!");
	gtk_widget_set_halign(empty_label, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(empty_label, GTK_ALIGN_CENTER);

	favorites_d->stack = gtk_stack_new();
	gtk_widget_add_css_class(favorites_d->stack, "info_box");
	gtk_stack_add_named(GTK_STACK(favorites_d->stack), scrolled, "list");
	gtk_stack_add_named(GTK_STACK(favorites_d->stack), empty_label, "empty");
	favorites_window_update(favorites_d);

	g_signal_connect(favorites_d->store, "items-changed", G_CALLBACK(on_favorites_changed), favorites_d);
	g_signal_connect(favorites_win, "close-request", G_CALLBACK(on_favorites_window_close_request), favorites_d);

	gtk_window_set_child(GTK_WINDOW(favorites_win), favorites_d->stack);
	gtk_window_present(GTK_WINDOW(favorites_win));
}
