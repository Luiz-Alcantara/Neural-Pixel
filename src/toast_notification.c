#include <gtk/gtk.h>
#include "constants.h"
#include "structs.h"

static void free_toast_data(gpointer user_data)
{
	ToastNotificationData *data = user_data;
	if (data->toast_timeout_id > 0) g_source_remove(data->toast_timeout_id);
	if (data->remove_timeout_id > 0) g_source_remove(data->remove_timeout_id);
	g_free(data);
}

static gboolean remove_toast_wgt(gpointer user_data)
{
	ToastNotificationData *data = user_data;
	if (data->dismissing_rev) {
		gtk_overlay_remove_overlay(data->overlay, data->dismissing_rev);
		data->dismissing_rev = NULL;
	}
	data->remove_timeout_id = 0;
	return G_SOURCE_REMOVE;
}

static gboolean toast_dismiss(gpointer user_data)
{
	ToastNotificationData *data = user_data;
	if (data->active_rev) {
		gtk_revealer_set_reveal_child(GTK_REVEALER(data->active_rev), FALSE);
		data->dismissing_rev = data->active_rev;
		data->active_rev = NULL;
		data->remove_timeout_id = g_timeout_add(250, remove_toast_wgt, data);
	}
	data->toast_timeout_id = 0;
	return G_SOURCE_REMOVE;
}

void show_toast_notification(GtkOverlay *overlay, const char *msg_text)
{
	ToastNotificationData *data = g_object_get_data(G_OBJECT(overlay), "toast-data");
	if (!data) {
		data = g_new0 (ToastNotificationData, 1);
		data->overlay = overlay;
		g_object_set_data_full(G_OBJECT(overlay), "toast-data", data, free_toast_data);
	}

	if (data->toast_timeout_id > 0) { g_source_remove(data->toast_timeout_id); data->toast_timeout_id = 0; }
	if (data->remove_timeout_id > 0) { g_source_remove(data->remove_timeout_id); data->remove_timeout_id = 0; }
	if (data->dismissing_rev) { gtk_overlay_remove_overlay(overlay, data->dismissing_rev); data->dismissing_rev = NULL; }
	if (data->active_rev) { gtk_overlay_remove_overlay(overlay, data->active_rev); data->active_rev = NULL; }

	GtkWidget *rev = gtk_revealer_new();
	gtk_revealer_set_transition_type(GTK_REVEALER(rev), GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
	gtk_revealer_set_transition_duration(GTK_REVEALER(rev), 250);

	GtkWidget *toast_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MEDIUM_SPACING);
	gtk_widget_set_margin_bottom (toast_box, LARGE_SPACING);
	gtk_widget_set_margin_end (toast_box, LARGE_SPACING);
	gtk_widget_set_margin_start (toast_box, LARGE_SPACING);
	gtk_widget_set_margin_top (toast_box, LARGE_SPACING);
	gtk_widget_add_css_class(toast_box, "custom_toast");

	GtkWidget *toast_label = gtk_label_new(msg_text);
	gtk_widget_add_css_class(toast_label, "toast_label");
	gtk_widget_set_margin_bottom (toast_label, MEDIUM_SPACING);
	gtk_widget_set_margin_end (toast_label, HUGE_SPACING);
	gtk_widget_set_margin_start (toast_label, HUGE_SPACING);
	gtk_widget_set_margin_top (toast_label, MEDIUM_SPACING);
	gtk_box_append(GTK_BOX(toast_box), toast_label);
	gtk_revealer_set_child(GTK_REVEALER(rev), toast_box);

	gtk_widget_set_halign(rev, GTK_ALIGN_END);
	gtk_widget_set_valign(rev, GTK_ALIGN_START);

	gtk_overlay_add_overlay(overlay, rev);
	data->active_rev = rev;

	gtk_revealer_set_reveal_child(GTK_REVEALER(rev), TRUE);
	data->toast_timeout_id = g_timeout_add(3000, toast_dismiss, data);
}
