#include <gtk/gtk.h>
#include <string.h>
#include "file_utils.h"
#include "str_utils.h"
#include "structs.h"
#include "widgets_cb.h"

static void factory_setup_cb (GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data)
{
	GtkWidget *label = gtk_label_new(NULL);
	gtk_list_item_set_child (list_item, label);
}

static void factory_bind_cb (GtkListItemFactory *factory, GtkListItem *list_item, gpointer user_data)
{
	GtkStringObject *strobj = GTK_STRING_OBJECT(gtk_list_item_get_item(list_item));
	const char *text = gtk_string_object_get_string(strobj);
	GtkWidget *label = gtk_list_item_get_child(list_item);

	gtk_label_set_label(GTK_LABEL(label), text);
	gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
	gtk_widget_set_halign(label, GTK_ALIGN_START);
}

GtkWidget* gen_const_dd(const char** items, int *def_item)
{
	GtkWidget* dd = gtk_drop_down_new_from_strings(items);
	gtk_widget_add_css_class(dd, "custom_dd");
	gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), *def_item);
	g_signal_connect(dd, "notify::selected-item", G_CALLBACK(set_dropdown_selected_const_item), def_item);
	g_signal_connect(dd, "destroy", G_CALLBACK(on_dropdown_destroy), NULL);
	return dd;
}

GtkWidget* gen_path_dd(const char* path, GtkTextBuffer *tb, int tb_type, GString *dd_item_str, GtkWidget *gen_btn, GApplication *app, int is_req)
{
	int start_pos = 0;
	
	GError *err = NULL;
	GtkStringList *my_list = get_files(path, &err);
	
	if (my_list == NULL) {
		if (err != NULL) {
			g_printerr("Error: %s\n", err->message);
			g_error_free(err);
			GtkWindow *win = GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(app)));
			gtk_window_close(win);
		}
	} else {
		GtkListItemFactory* fact = gtk_signal_list_item_factory_new();
		
		if (dd_item_str != NULL) {
			start_pos = check_gtk_list_contains_item(my_list, dd_item_str->str);
		}
		
		g_signal_connect (fact, "setup", G_CALLBACK (factory_setup_cb), NULL);
		g_signal_connect (fact, "bind", G_CALLBACK (factory_bind_cb), NULL);

		g_object_ref_sink(my_list);
		
		GtkWidget* dd = gtk_drop_down_new(G_LIST_MODEL(my_list), NULL);
		
		GtkExpression *dd_expression = gtk_property_expression_new (GTK_TYPE_STRING_OBJECT, NULL, "string");
		gtk_drop_down_set_expression (GTK_DROP_DOWN(dd), dd_expression);
		gtk_drop_down_set_enable_search(GTK_DROP_DOWN(dd), TRUE);
		#if GTK_CHECK_VERSION(4, 12, 0)
			gtk_drop_down_set_search_match_mode(GTK_DROP_DOWN(dd), GTK_STRING_FILTER_MATCH_MODE_SUBSTRING);
		#endif
		gtk_widget_add_css_class(dd, "custom_dd");
		gtk_expression_unref (dd_expression);
		g_object_unref(my_list);
		
		gtk_drop_down_set_factory(GTK_DROP_DOWN(dd), fact);
		g_object_unref(fact);

		if (tb != NULL) {
			DropDownTextBufferData *dd_path_data = g_new0 (DropDownTextBufferData, 1);
			dd_path_data->tb_type = tb_type;
			dd_path_data->textbuffer = tb;
			g_signal_connect(dd, "notify::selected-item", G_CALLBACK(add_dropdown_selected_item_textview), dd_path_data);
			g_signal_connect(dd, "destroy", G_CALLBACK(on_dd_path_destroy), dd_path_data);
			gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), 0);
		} else {
			DropDownModelsNameData *dd_const_data = g_new0 (DropDownModelsNameData, 1);
			dd_const_data->dd_item_str = dd_item_str;
			dd_const_data->req_int = is_req;
			dd_const_data->g_btn = gen_btn;
			g_signal_connect(dd, "notify::selected-item", G_CALLBACK(set_dropdown_selected_item), dd_const_data);
			if (is_req == 1) {
				if (g_strcmp0(dd_item_str->str, "None") == 0) {
					gtk_button_set_label (GTK_BUTTON(gen_btn), "Select a model.");
					gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), FALSE);
				} else {
					gtk_button_set_label (GTK_BUTTON(gen_btn), "Generate");
					gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), TRUE);
				}
			}
			gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), start_pos);
			g_signal_connect(dd, "destroy", G_CALLBACK(on_dd_const_destroy), dd_const_data);
		}
		return dd;
	}
}
