#include <gtk/gtk.h>
#include <string.h>
#include "constants.h"
#include "file_utils.h"
#include "str_utils.h"
#include "structs.h"
#include "widgets_cb.h"

static void on_toggle_embedding_type_btn_clicked(GtkButton *btn, gpointer user_data)
{
	const char *btn_label = gtk_button_get_label(btn);
	gtk_button_set_label(btn, g_strcmp0(btn_label, "NP") == 0 ? "PP" : "NP");
}

static void on_remove_adapter_btn_clicked(GtkButton *btn, gpointer user_data)
{
	GtkWidget *adapter_card = GTK_WIDGET(user_data);
	GtkWidget *box_model_adapters_cards = gtk_widget_get_parent(adapter_card);
	gtk_box_remove(GTK_BOX(box_model_adapters_cards), adapter_card);
}

void add_adapter_card(GtkBox *box_model_adapters_cards, const char *name, double value, int adapter_type, int negative_embedding)
{
	if (name == NULL) return;

	GtkWidget *ancestor_sw = gtk_widget_get_ancestor(GTK_WIDGET(box_model_adapters_cards), GTK_TYPE_SCROLLED_WINDOW);

	GtkWidget *adapter_card = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MEDIUM_SPACING);
	gtk_widget_add_css_class(adapter_card, "inner_box");
	gtk_widget_set_margin_bottom(adapter_card, MEDIUM_SPACING);
	gtk_widget_set_margin_end(adapter_card, MEDIUM_SPACING);
	gtk_widget_set_margin_start(adapter_card, MEDIUM_SPACING);
	gtk_widget_set_margin_top(adapter_card, MEDIUM_SPACING);

	GtkWidget *adapter_card_inner = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, MEDIUM_SPACING);
	gtk_widget_set_margin_bottom(adapter_card_inner, MEDIUM_SPACING);
	gtk_widget_set_margin_end(adapter_card_inner, MEDIUM_SPACING);
	gtk_widget_set_margin_start(adapter_card_inner, MEDIUM_SPACING);
	gtk_widget_set_margin_top(adapter_card_inner, MEDIUM_SPACING);
	gtk_box_append(GTK_BOX(adapter_card), adapter_card_inner);

	char *adapter_label_string = g_strdup_printf("%s: %s", adapter_type == 1 ? "LoRA" : "Embedding", name);
	GtkWidget *adapter_label = gtk_label_new(adapter_label_string);
	gtk_widget_add_css_class(adapter_label, "param_label");
	gtk_widget_set_tooltip_text(adapter_label, adapter_label_string);
	gtk_widget_set_margin_start(adapter_label, MEDIUM_SPACING);
	gtk_label_set_ellipsize(GTK_LABEL(adapter_label), PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand(adapter_label, TRUE);
	gtk_widget_set_halign(adapter_label, GTK_ALIGN_CENTER);
	gtk_box_append(GTK_BOX(adapter_card_inner), adapter_label);
	g_free(adapter_label_string);

	if (adapter_type == 1) {
		GtkWidget *adapter_spin = gtk_spin_button_new_with_range(-5.0, 5.0, 0.05);
		gtk_widget_add_css_class(adapter_spin, "custom_spin");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(adapter_spin), value);
		gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(adapter_spin), TRUE);
		gtk_spin_button_set_digits(GTK_SPIN_BUTTON(adapter_spin), 2);
		gtk_widget_set_tooltip_text(adapter_spin, "Sets the adapter weight.");
		if (ancestor_sw) stop_spinbutton_scroll(adapter_spin, ancestor_sw);
		gtk_box_append(GTK_BOX(adapter_card_inner), adapter_spin);
		g_object_set_data(G_OBJECT(adapter_card), "adapter-spin", adapter_spin);
	} else if (adapter_type == 0) {
		GtkWidget *toggle_embedding_type_btn = gtk_button_new_with_label(negative_embedding == 1 ? "NP" : "PP");
		gtk_widget_add_css_class(toggle_embedding_type_btn, "custom_btn");
		gtk_widget_set_tooltip_text(toggle_embedding_type_btn,
		"Toggles the prompt field that will receive the embedding.\nNP = Negative Prompt\nPP = Positive Prompt.");
		g_signal_connect(toggle_embedding_type_btn, "clicked", G_CALLBACK(on_toggle_embedding_type_btn_clicked), NULL);
		gtk_box_append(GTK_BOX(adapter_card_inner), toggle_embedding_type_btn);
		g_object_set_data(G_OBJECT(adapter_card), "adapter-prompt-toggle-btn", toggle_embedding_type_btn);
	}

	GtkWidget *remove_adapter_btn = gtk_button_new_from_icon_name("edit-delete-symbolic");
	gtk_widget_add_css_class(remove_adapter_btn, "custom_btn");
	gtk_widget_set_tooltip_text(remove_adapter_btn, "Removes the adapter.");
	gtk_box_append(GTK_BOX(adapter_card_inner), remove_adapter_btn);
	
	g_object_set_data_full(G_OBJECT(adapter_card), "adapter-name", g_strdup(name), g_free);
	g_object_set_data(G_OBJECT(adapter_card), "adapter-type", GINT_TO_POINTER(adapter_type));
	
	g_signal_connect(remove_adapter_btn, "clicked", G_CALLBACK(on_remove_adapter_btn_clicked), adapter_card);
	
	gtk_box_append(box_model_adapters_cards, adapter_card);
}

gboolean adapter_exists(GtkWidget *box, const char *adapter_name, int adapter_type)
{
	for (GtkWidget *card = gtk_widget_get_first_child(box); card != NULL; card = gtk_widget_get_next_sibling(card))
	{
		const char *existing = g_object_get_data(G_OBJECT(card), "adapter-name");
		int existing_type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(card), "adapter-type"));
		if (g_strcmp0(existing, adapter_name) == 0 && existing_type == adapter_type) return TRUE;
	}
	return FALSE;
}

static void add_dropdown_selected_adapter(GtkWidget* wgt, GParamSpec *pspec, gpointer user_data)
{
	if (user_data == NULL) return;

	if (GTK_IS_DROP_DOWN(wgt)) {
		GtkDropDown *dropdown = GTK_DROP_DOWN(wgt);
		DropDownAdaptersData *data = user_data;

		GtkStringObject *selected_item = gtk_drop_down_get_selected_item(dropdown);
		if (selected_item == NULL || !GTK_IS_STRING_OBJECT(selected_item)) return;
		const char* item_string = gtk_string_object_get_string(selected_item);
		if (item_string == NULL) return;
		char *basename = strip_model_extension(item_string);

		if (strcmp(item_string, "None") != 0) {
			if (!adapter_exists(data->box_model_adapters_cards, basename, data->adapter_type)) {
				add_adapter_card(GTK_BOX(data->box_model_adapters_cards), basename, 1.0, data->adapter_type, 1);
			}
		}
		g_free(basename);
	}
}

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
	return dd;
}

GtkWidget* gen_path_dd(const char* path, GtkWidget *box_model_adapters_cards, int adapter_type, GString *dd_item_str, GtkWidget *gen_btn, GApplication *app, int is_req)
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
			if (start_pos == 0 && strcmp(dd_item_str->str, "None") != 0) g_string_assign(dd_item_str, "None");
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

		if (box_model_adapters_cards != NULL) {
			DropDownAdaptersData *dd_adapters_data = g_new0 (DropDownAdaptersData, 1);
			dd_adapters_data->adapter_type = adapter_type;
			dd_adapters_data->box_model_adapters_cards = box_model_adapters_cards;
			g_signal_connect(dd, "notify::selected-item", G_CALLBACK(add_dropdown_selected_adapter), dd_adapters_data);
			g_signal_connect(dd, "destroy", G_CALLBACK(on_dd_path_destroy), dd_adapters_data);
			gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), 0);
		} else {
			DropDownModelsNameData *dd_const_data = g_new0 (DropDownModelsNameData, 1);
			dd_const_data->req_int = is_req;
			dd_const_data->g_btn = gen_btn;
			g_signal_connect(dd, "notify::selected-item", G_CALLBACK(set_dropdown_selected_item), dd_const_data);
			
			if (is_req == 1 && start_pos == 0) {
				gtk_button_set_label (GTK_BUTTON(gen_btn), "Select a checkpoint first.");
				gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), FALSE);
			} else if (is_req == 1) {
				gtk_button_set_label (GTK_BUTTON(gen_btn), "Add to Queue");
				gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), TRUE);
			}

			gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), start_pos);
			g_signal_connect(dd, "destroy", G_CALLBACK(on_dd_const_destroy), dd_const_data);
		}
		return dd;
	}
	return NULL;
}

GtkWidget* gen_visibility_toggle_button(const char *btn_text, const char *css_class_str, GtkWidget *wgt)
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
