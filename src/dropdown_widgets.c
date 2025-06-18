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
	GtkLabel *label;
	GtkStringObject *string_object;
	const gchar *string;

	string_object = GTK_STRING_OBJECT(gtk_list_item_get_item(list_item));
	string = gtk_string_object_get_string(string_object);
	label = GTK_LABEL(gtk_list_item_get_child(list_item));
	gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
	gtk_widget_set_hexpand (GTK_WIDGET(label), FALSE);
	size_t length = 28;

	size_t truncated_length = length;

	if (strlen(string) < length) {
		truncated_length = strlen(string);
		char *shortened_string = g_malloc(truncated_length + 1);
		strncpy(shortened_string, string, truncated_length);
		shortened_string[truncated_length] = '\0';
		gtk_label_set_text(label, shortened_string);
		g_free(shortened_string);
	} else {
		char *shortened_string = g_malloc(truncated_length + 4);
		strncpy(shortened_string, string, truncated_length);
		strcpy(shortened_string + truncated_length, "...");
		shortened_string[truncated_length + 3] = '\0';
		gtk_label_set_text(label, shortened_string);
		g_free(shortened_string);
	}
}

GtkWidget* gen_const_dd(const char** items, int *def_item)
{
	GtkWidget* dd = gtk_drop_down_new_from_strings(items);
	gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), *def_item);
	g_signal_connect(dd, "notify::selected-item", G_CALLBACK(set_dropdown_selected_const_item), def_item);
	g_signal_connect(dd, "destroy", G_CALLBACK(on_dropdown_destroy), NULL);
	return dd;
}

GtkWidget* gen_path_dd(const char* path, const int str_size, GtkTextBuffer *tb, int tb_type, GString *dd_item_str, GtkWidget *gen_btn, int is_req)
{
	int start_pos = 0;
	
	GtkStringList *my_list = get_files(path);
	GtkListItemFactory* fact = gtk_signal_list_item_factory_new();
	
	if (dd_item_str != NULL) {
		start_pos = check_gtk_list_contains_item(my_list, dd_item_str->str);
	}
	
	g_signal_connect (fact, "setup", G_CALLBACK (factory_setup_cb), NULL);
	g_signal_connect (fact, "bind", G_CALLBACK (factory_bind_cb), NULL);

	GtkWidget* dd = gtk_drop_down_new(G_LIST_MODEL(my_list), NULL);
	gtk_drop_down_set_factory(GTK_DROP_DOWN(dd), fact);

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
