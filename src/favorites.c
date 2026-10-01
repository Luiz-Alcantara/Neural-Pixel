#include <gtk/gtk.h>
#include <string.h>
#include "constants.h"
#include "file_utils.h"
#include "png_utils.h"
#include "structs.h"
#include "widgets_cb.h"

static void on_delete_favorite(GtkButton *btn, gpointer user_data);
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
	gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(fav_image_frame), 640);
	gtk_scrolled_window_set_max_content_width(GTK_SCROLLED_WINDOW(fav_image_frame), 640);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(fav_image_frame), 640);
	gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(fav_image_frame), 640);
	gtk_widget_set_size_request(fav_image_frame, 640, 640);
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
	GtkStringObject *obj = gtk_list_item_get_item(item);
	const char *path = gtk_string_object_get_string(obj);

	GtkWidget *fav_card = gtk_list_item_get_child(item);
	GtkWidget *fav_image = g_object_get_data(G_OBJECT(fav_card), "fav-image");

	if (check_file_exists(path, 0)) {
		gtk_picture_set_filename(GTK_PICTURE(fav_image), path);
	} else {
		gtk_picture_set_filename(GTK_PICTURE(fav_image), EMPTY_IMG_PATH);
		g_printerr("Failed to load file: '%s'.\n", path);
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
	g_signal_connect(factory, "bind",  G_CALLBACK(favorites_factory_bind_cb),  favorites_d);

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
