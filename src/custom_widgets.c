#include <gtk/gtk.h>
#include <string.h>
#include "constants.h"
#include "file_utils.h"
#include "str_utils.h"
#include "structs.h"
#include "widgets_cb.h"

void factory_setup_cb (GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data)
{
	GtkWidget *label = gtk_label_new(NULL);
	gtk_list_item_set_child (list_item, label);
}

void factory_bind_cb (GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data)
{
	GtkStringObject *strobj = GTK_STRING_OBJECT(gtk_list_item_get_item(list_item));
	const char *text = gtk_string_object_get_string(strobj);
	GtkWidget *label = gtk_list_item_get_child(list_item);

	gtk_label_set_label(GTK_LABEL(label), text);
	gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
	gtk_widget_set_halign(label, GTK_ALIGN_START);
}

static void enforce_checkpoint(GtkWidget* wgt, GParamSpec *pspec, gpointer user_data)
{
	if (GTK_IS_DROP_DOWN(wgt)) {
		GtkWidget *gen_btn = (GtkWidget *)user_data;
		guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(wgt));
		if (g_strcmp0(gtk_button_get_label(GTK_BUTTON(gen_btn)), "Add to Queue") == 0 ||
		g_strcmp0(gtk_button_get_label(GTK_BUTTON(gen_btn)), "Select a checkpoint first.") == 0) {
			gboolean none_selected = (sel == 0);
			gtk_button_set_label(GTK_BUTTON(gen_btn), none_selected ? "Select a checkpoint first." : "Add to Queue");
			gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), !none_selected);
		}
	}
}

GtkWidget *gen_checkpoint_dd(const char* path, GString *dd_item_str, GtkWidget *gen_btn)
{
	int start_pos = 0;
	GError *error = NULL;
	GtkStringList *my_list = get_files(path, &error);

	if (my_list == NULL) {
		g_printerr("Error: %s\n", error ? error->message : "unknown error");
		g_clear_error(&error);
		return NULL;
	}

	GtkListItemFactory *fact = gtk_signal_list_item_factory_new();
	g_signal_connect(fact, "setup", G_CALLBACK(factory_setup_cb), NULL);
	g_signal_connect(fact, "bind", G_CALLBACK(factory_bind_cb), NULL);

	GtkWidget *dd = gtk_drop_down_new(NULL, NULL);
	gtk_drop_down_set_model(GTK_DROP_DOWN(dd), G_LIST_MODEL(my_list));
	g_object_unref(my_list);

	GtkExpression *expr = gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, NULL, "string");
	gtk_drop_down_set_expression(GTK_DROP_DOWN(dd), expr);
	gtk_expression_unref(expr);

	gtk_drop_down_set_enable_search(GTK_DROP_DOWN(dd), TRUE);
	#if GTK_CHECK_VERSION(4, 12, 0)
		gtk_drop_down_set_search_match_mode(GTK_DROP_DOWN(dd), GTK_STRING_FILTER_MATCH_MODE_SUBSTRING);
	#endif
	gtk_widget_add_css_class(dd, "custom_dd");

	gtk_drop_down_set_factory(GTK_DROP_DOWN(dd), fact);
	g_object_unref(fact);

	start_pos = check_gtk_list_contains_item(my_list, dd_item_str->str);
	if (start_pos == 0 && strcmp(dd_item_str->str, "None") != 0) g_string_assign(dd_item_str, "None");

	gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), start_pos);
	gtk_button_set_label (GTK_BUTTON(gen_btn), start_pos == 0 ? "Select a checkpoint first." : "Add to Queue");
	gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), start_pos == 0 ? FALSE : TRUE);

	g_signal_connect(dd, "notify::selected-item", G_CALLBACK(enforce_checkpoint), gen_btn);

	return dd;
}

GtkWidget *gen_models_dd(const char* path, GString *dd_item_str)
{
	int start_pos = 0;
	GError *error = NULL;
	GtkStringList *my_list = get_files(path, &error);

	if (my_list == NULL) {
		g_printerr("Error: %s\n", error ? error->message : "unknown error");
		g_clear_error(&error);
		return NULL;
	}

	GtkListItemFactory *fact = gtk_signal_list_item_factory_new();
	g_signal_connect(fact, "setup", G_CALLBACK(factory_setup_cb), NULL);
	g_signal_connect(fact, "bind", G_CALLBACK(factory_bind_cb), NULL);

	GtkWidget *dd = gtk_drop_down_new(NULL, NULL);
	gtk_drop_down_set_model(GTK_DROP_DOWN(dd), G_LIST_MODEL(my_list));
	g_object_unref(my_list);

	GtkExpression *expr = gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, NULL, "string");
	gtk_drop_down_set_expression(GTK_DROP_DOWN(dd), expr);
	gtk_expression_unref(expr);

	gtk_drop_down_set_enable_search(GTK_DROP_DOWN(dd), TRUE);
	#if GTK_CHECK_VERSION(4, 12, 0)
		gtk_drop_down_set_search_match_mode(GTK_DROP_DOWN(dd), GTK_STRING_FILTER_MATCH_MODE_SUBSTRING);
	#endif
	gtk_widget_add_css_class(dd, "custom_dd");

	gtk_drop_down_set_factory(GTK_DROP_DOWN(dd), fact);
	g_object_unref(fact);

	start_pos = check_gtk_list_contains_item(my_list, dd_item_str->str);
	if (start_pos == 0 && strcmp(dd_item_str->str, "None") != 0) g_string_assign(dd_item_str, "None");

	gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), start_pos);

	return dd;
}

GtkWidget *gen_const_dd(const char** items, int *def_item)
{
	GtkWidget* dd = gtk_drop_down_new_from_strings(items);
	gtk_widget_add_css_class(dd, "custom_dd");
	gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), *def_item);
	return dd;
}

GtkWidget *gen_visibility_toggle_button(const char *btn_text, const char *css_class_str, GtkWidget *wgt)
{
	GtkWidget *btn = gtk_button_new();
	gtk_widget_add_css_class(btn, "custom_btn");

	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_widget_set_halign(box, GTK_ALIGN_CENTER);

	gboolean is_visible = gtk_widget_get_visible(wgt);
	const char *icon_name = is_visible ? "pan-down-symbolic" : "pan-end-symbolic";

	GtkWidget *icon = gtk_image_new_from_icon_name(icon_name);
	GtkWidget *label = gtk_label_new(btn_text);
	gtk_widget_set_halign(label, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(label, css_class_str);

	gtk_box_append(GTK_BOX(box), icon);
	gtk_box_append(GTK_BOX(box), label);
	gtk_button_set_child(GTK_BUTTON(btn), box);

	g_object_set_data(G_OBJECT(btn), "toggle-icon", icon);
	g_signal_connect(btn, "clicked", G_CALLBACK(toggle_wgt_visibility), wgt);
	return btn;
}
