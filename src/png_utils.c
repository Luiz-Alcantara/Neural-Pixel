#include <gtk/gtk.h>
#include <png.h>
#include <stdio.h>
#include "constants.h"
#include "file_utils.h"
#include "str_utils.h"
#include "structs.h"
#include "widgets_cb.h"

#define MAX_METADATA_PROMPT_LENGTH 2048
#define MAX_PROPERTY_LENGTH 512

static void set_png_metadata(gchar *path, gpointer user_data)
{
	LoadPNGData *data = user_data;

	FILE *fp = fopen(path, "rb");
	if (!fp) {
		fprintf(stderr, "File opening failed.\n");
		g_free(path);
		show_error_message (data->win, "Error loading PNG info", "File opening failed");
		return;
	}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fprintf(stderr, "png_create_read_struct failed.\n");
		fclose(fp);
		g_free(path);
		show_error_message (data->win, "Error loading PNG info", "png_create_read_struct failed");
		return;
	}

	png_infop info = png_create_info_struct(png);
	if (!info) {
		fprintf(stderr, "png_create_info_struct failed.\n");
		png_destroy_read_struct(&png, NULL, NULL);
		fclose(fp);
		g_free(path);
		show_error_message (data->win, "Error loading PNG info", "png_create_info_struct failed");
		return;
	}

	if (setjmp(png_jmpbuf(png))) {
		fprintf(stderr, "Error during PNG initialization.\n");
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		g_free(path);
		show_error_message (data->win, "Error loading PNG info", "Error during PNG initialization");
		return;
	}

	png_init_io(png, fp);
	png_read_info(png, info);

	png_textp text;
	int num_text;
	png_get_text(png, info, &text, &num_text);

	GString *l1 = g_string_new(NULL);

	for (int i = 0; i < num_text; i++) {
		g_string_append_printf(l1, "%s %s", text[i].key, text[i].text);
	}

	int n_err = 0;
	const char *ptr;

	//Set Positive Prompt
	ptr = strstr(l1->str, "parameters ");
	if (ptr) {
		const char *start = ptr + strlen("parameters ");
		const char *end = strstr(start, "\nNegative prompt:");
		if (start && end && start < end) {
			size_t len = end - start;
			if (len >= MAX_METADATA_PROMPT_LENGTH) {
				len = MAX_METADATA_PROMPT_LENGTH - 1;
			}
			char pprompt[MAX_METADATA_PROMPT_LENGTH];
			strncpy(pprompt, start, len);
			pprompt[len] = '\0';
			
			GtkTextBuffer *ptb = data->pos_tb;
			gtk_text_buffer_set_text (ptb, pprompt, -1);
		} else {
			fprintf(stderr, "Failed to parse positive prompt data.\n");
			n_err++;
		}
	} else {
		fprintf(stderr, "Failed to parse positive prompt data.\n");
		n_err++;
	}
	
	
	//Set Negative Prompt
	ptr = strstr(l1->str, "Negative prompt: ");
	if (ptr) {
		const char *start = ptr + strlen("Negative prompt: ");
		const char *end = strstr(start, "\nSteps:");
		if (start && end && start < end) {
			size_t len = end - start;
			if (len >= MAX_METADATA_PROMPT_LENGTH) {
				len = MAX_METADATA_PROMPT_LENGTH - 1;
			}
			char nprompt[MAX_METADATA_PROMPT_LENGTH];
			strncpy(nprompt, start, len);
			nprompt[len] = '\0';
			
			GtkTextBuffer *ntb = data->neg_tb;
			gtk_text_buffer_set_text (ntb, nprompt, -1);
		} else {
			fprintf(stderr, "Failed to parse negative prompt data.\n");
			n_err++;
		}
	} else {
		fprintf(stderr, "Failed to parse negative prompt data.\n");
		n_err++;
	}
	
	//Set Steps
	ptr = strstr(l1->str, "Steps: ");
	if (ptr) {
		int steps_int;
		if (sscanf(ptr + strlen("Steps: "), "%d", &steps_int) == 1) {
			
			GtkWidget *steps_spin = data->steps_spin;
			gtk_spin_button_set_value (GTK_SPIN_BUTTON(steps_spin), steps_int);
		} else {
			fprintf(stderr, "Failed to parse steps data.\n");
			n_err++;
		}
	} else {
		fprintf(stderr, "Failed to parse steps data.\n");
		n_err++;
	}
	
	//Set CFG Value
	ptr = strstr(l1->str, "CFG scale: ");
	if (ptr) {
		double cfg_double;
		if (sscanf(ptr + strlen("CFG scale: "), "%lf", &cfg_double) == 1) {
			GtkWidget *cfg_spin = data->cfg_spin;
			gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), cfg_double);
		} else {
			fprintf(stderr, "Failed to parse CFG data.\n");
			n_err++;
		}
	} else {
		fprintf(stderr, "Failed to parse CFG data.\n");
		n_err++;
	}
	
	//Set Seed Value
	ptr = strstr(l1->str, "Seed: ");
	if (ptr) {
		long long int seed_long;
		if (sscanf(ptr + strlen("Seed: "), "%lld", &seed_long) == 1) {
			GtkEntry *seed_entry = GTK_ENTRY(data->seed_entry);
			char *seed_str;
			seed_str = convert_long_long_int_to_string(seed_long);
			if (seed_str != NULL) {
				gtk_editable_set_text(GTK_EDITABLE(seed_entry), seed_str);
				free(seed_str);
			} else {
				fprintf(stderr, "Failed to parse seed data.\n");
				gtk_editable_set_text(GTK_EDITABLE(seed_entry), "-1");
			}
		} else {
			fprintf(stderr, "Failed to parse seed data.\n");
			n_err++;
		}
	} else {
		fprintf(stderr, "Failed to parse seed data.\n");
		n_err++;
	}
	
	//Set IMG Size
	ptr = strstr(l1->str, "Size: ");
	if (ptr) {
		int width_int;
		int heigth_int;
		if (sscanf(ptr + strlen("Size: "), "%dx%d", &width_int, &heigth_int) == 2) {
			char width_str[MAX_PROPERTY_LENGTH];
			snprintf(width_str, sizeof(width_str), "%d", width_int);
			
			char heigth_str[MAX_PROPERTY_LENGTH];
			snprintf(heigth_str, sizeof(heigth_str), "%d", heigth_int);
			
			int width_index = check_list_contains_item(LIST_RESOLUTIONS_STR, width_str);
			int height_index = check_list_contains_item(LIST_RESOLUTIONS_STR, heigth_str);
			
			GtkWidget *width_dd = data->width_dd;
			GtkWidget *height_dd = data->height_dd;
			gtk_drop_down_set_selected(GTK_DROP_DOWN(width_dd), width_index);
			gtk_drop_down_set_selected(GTK_DROP_DOWN(height_dd), height_index);
		} else {
			fprintf(stderr, "Failed to parse image size data.\n");
			n_err++;
		}
	} else {
		fprintf(stderr, "Failed to parse image size data.\n");
		n_err++;
	}
	
	//Set SD Checkpoint
	ptr = strstr(l1->str, "Model: ");
	if (ptr) {
		const char *start = ptr + strlen("Model: ");
		const char *end = strstr(start, ",");
		if (start && end && start < end) {
			size_t len = end - start;
			if (len >= MAX_PROPERTY_LENGTH) {
				len = MAX_PROPERTY_LENGTH - 1;
			}
			char checkpoint_str[MAX_PROPERTY_LENGTH];
			strncpy(checkpoint_str, start, len);
			checkpoint_str[len] = '\0';
			
			GtkWidget *checkpoint_dd = data->checkpoint_dd;
			GtkStringList *checkpoint_items = GTK_STRING_LIST(gtk_drop_down_get_model(GTK_DROP_DOWN(checkpoint_dd)));
			int checkpoint_i = check_gtk_list_contains_item(checkpoint_items, checkpoint_str);
			gtk_drop_down_set_selected(GTK_DROP_DOWN(checkpoint_dd), checkpoint_i);
		} else {
			fprintf(stderr, "Failed to parse checkpoint data.\n");
			n_err++;
		}
	} else {
		fprintf(stderr, "Failed to parse checkpoint data.\n");
		n_err++;
	}
	
	//Set Sampler and Scheduler
	ptr = strstr(l1->str, "Sampler: ");
	if (ptr) {
		char sampler_str[MAX_PROPERTY_LENGTH];
		char scheduler_str[MAX_PROPERTY_LENGTH];
		if (sscanf(ptr + strlen("Sampler: "), "%s %[^,],", &sampler_str, &scheduler_str) == 2) {
			GtkWidget *sampler_dd = data->sampler_dd;
			GtkWidget *scheduler_dd = data->scheduler_dd;
			
			int sampler_index = check_list_contains_item(LIST_SAMPLES, sampler_str);
			int scheduler_index = check_list_contains_item(LIST_SCHEDULES, scheduler_str);
			
			gtk_drop_down_set_selected(GTK_DROP_DOWN(sampler_dd), sampler_index);
			gtk_drop_down_set_selected(GTK_DROP_DOWN(scheduler_dd), scheduler_index);
		} else {
			fprintf(stderr, "Failed to parse sampler data.\n");
			n_err++;
		}
	} else {
		fprintf(stderr, "Failed to parse sampler data.\n");
		n_err++;
	}

	png_destroy_read_struct(&png, &info, NULL);
	fclose(fp);
	g_free(path);
	g_string_free(l1, TRUE);
}

#if GTK_CHECK_VERSION(4, 10, 0)
static void read_png_metadata(GObject* client, GAsyncResult* res, gpointer user_data)
{
	LoadPNGData *data = user_data;
	g_autoptr(GError) error = NULL;
	g_autoptr(GFile) png_file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(client), res, &error);
	
	gchar *img_path = g_file_get_path(png_file);
	
	if (error != NULL) {
		g_warning("Opening file for metadata failed: %s", error->message);
	}

	if (img_path != NULL) {
		set_png_metadata(img_path, user_data);
	}
	if (data->cancellable != NULL) {
		g_object_unref(data->cancellable);
		data->cancellable = NULL;
	}
}

static void set_file_path(GObject* client, GAsyncResult* res, gpointer user_data)
{
	LoadImg2ImgData *data = user_data;
	g_autoptr(GError) error = NULL;
	
	g_autoptr(GFile) png_file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(client), res, &error);
	
	if (error != NULL) {
		g_warning("Opening file failed: %s", error->message);
	}

	if (png_file != NULL) {
		char *path = g_file_get_path(png_file);
		if (path != NULL) {
			g_string_assign(data->img2img_file_path, path);
			GtkPicture *prev_img = GTK_PICTURE(data->image_wgt);
			if (check_file_exists(path, 0) == 1) {
				gtk_picture_set_filename(prev_img, path);
			} else {
				g_printerr(
					"Error loading image: The file '%s' is missing, corrupted, or invalid.\n",
					path
				);
				show_error_message(data->win,
					"Error loading image",
					"Error loading image: The selected file is missing, corrupted, or invalid.\n"
				);
				gtk_picture_set_filename(prev_img, DEFAULT_IMG_PATH);
			}
			g_free(path);
		}
	}
	if (data->cancellable != NULL) {
		g_object_unref(data->cancellable);
		data->cancellable = NULL;
	}
}

#else
static void read_png_metadata_deprecated(GtkDialog* dialog, int response, gpointer user_data)
{
	if (response == GTK_RESPONSE_ACCEPT)
	{
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		g_autoptr(GFile) png_file = gtk_file_chooser_get_file (chooser);

		gchar *img_path = g_file_get_path(png_file);

		if (img_path != NULL) {
			set_png_metadata(img_path, user_data);
		}
	}
	gtk_window_destroy (GTK_WINDOW (dialog));
}

static void set_file_path_deprecated(GtkDialog* dialog, int response, gpointer user_data)
{
	if (response == GTK_RESPONSE_ACCEPT)
	{
		LoadImg2ImgData *data = user_data;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
		g_autoptr(GFile) png_file = gtk_file_chooser_get_file (chooser);

		if (png_file != NULL) {
			char *path = g_file_get_path(png_file);
			if (path != NULL) {
				g_string_assign(data->img2img_file_path, path);
				GtkPicture *prev_img = GTK_PICTURE(data->image_wgt);
				if (check_file_exists(path, 0) == 1) {
					gtk_picture_set_filename(prev_img, path);
				} else {
					g_printerr(
						"Error loading image: The file '%s' is missing, corrupted, or invalid.\n",
						path
					);
					show_error_message(data->win,
						"Error loading image",
						"Error loading image: The selected file is missing, corrupted, or invalid.\n"
					);
					gtk_picture_set_filename(prev_img, DEFAULT_IMG_PATH);
				}
				g_free(path);
			}
		}
	}
	gtk_window_destroy (GTK_WINDOW (dialog));
}

#endif

void load_from_img_preview(GtkWidget *btn, gpointer user_data)
{
	LoadPNGData *data = user_data;
	
	gchar *img_path = g_strdup(g_ptr_array_index(data->image_files, *data->current_image_index));
	
	if (img_path != NULL) {
		set_png_metadata(img_path, user_data);
	} else {
		fprintf(stderr, "Failed to get image path.\n");
	}
}

void load_from_img_btn_cb(GtkWidget *btn, gpointer user_data)
{
	#if GTK_CHECK_VERSION(4, 10, 0)
		LoadPNGData *data = user_data;
		GtkWindow *win = GTK_WINDOW(data->win);

		GtkFileDialog* load_img_dialog = gtk_file_dialog_new();
		gtk_file_dialog_set_title(load_img_dialog, "Choose the image:");
		gtk_file_dialog_set_modal(load_img_dialog, TRUE);

		GtkFileFilter *load_img_filter = gtk_file_filter_new();
		gtk_file_filter_add_suffix (load_img_filter, "png");
		gtk_file_filter_set_name(load_img_filter,"PNG");

		GListStore* filter_list_store = g_list_store_new (GTK_TYPE_FILE_FILTER);
		g_list_store_append(filter_list_store, load_img_filter);
		gtk_file_dialog_set_filters(load_img_dialog, G_LIST_MODEL(filter_list_store));

		data->cancellable = g_cancellable_new();

		gtk_file_dialog_open(load_img_dialog, win, data->cancellable, (GAsyncReadyCallback)read_png_metadata, user_data);
	#else
		LoadPNGData *data = user_data;
		GtkWindow *win = GTK_WINDOW(data->win);

		GtkWidget *load_img_dialog = gtk_file_chooser_dialog_new ("Choose the image:",
						win,
						GTK_FILE_CHOOSER_ACTION_OPEN,
						"Cancel",
						GTK_RESPONSE_CANCEL,
						"Open",
						GTK_RESPONSE_ACCEPT,
						NULL);

		gtk_window_present (GTK_WINDOW (load_img_dialog));
		
		g_signal_connect (load_img_dialog, "response",
				G_CALLBACK (read_png_metadata_deprecated),
				user_data);

	#endif
}

void load_img2img_btn_cb(GtkWidget *btn, gpointer user_data)
{
	#if GTK_CHECK_VERSION(4, 10, 0)
		LoadImg2ImgData *data = user_data;
		GtkWindow *win = GTK_WINDOW(data->win);

		GtkFileDialog* load_img_dialog = gtk_file_dialog_new();
		gtk_file_dialog_set_title(load_img_dialog, "Choose the image file:");
		gtk_file_dialog_set_modal(load_img_dialog, TRUE);

		GtkFileFilter *load_img_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(load_img_filter,"Images (*.png, *.jpg)");
		gtk_file_filter_add_suffix (load_img_filter, "png");
		gtk_file_filter_add_suffix (load_img_filter, "jpg");

		GListStore *filter_list_store = g_list_store_new (GTK_TYPE_FILE_FILTER);
		g_list_store_append(filter_list_store, load_img_filter);
		gtk_file_dialog_set_filters(load_img_dialog, G_LIST_MODEL(filter_list_store));
		g_object_unref(filter_list_store);

		data->cancellable = g_cancellable_new();

		gtk_file_dialog_open(load_img_dialog, win, data->cancellable, (GAsyncReadyCallback)set_file_path, user_data);
	#else
		LoadImg2ImgData *data = user_data;
		GtkWindow *win = GTK_WINDOW(data->win);

		GtkWidget *load_img_dialog = gtk_file_chooser_dialog_new ("Choose the image:",
						win,
						GTK_FILE_CHOOSER_ACTION_OPEN,
						"Cancel",
						GTK_RESPONSE_CANCEL,
						"Open",
						GTK_RESPONSE_ACCEPT,
						NULL);

		gtk_window_present (GTK_WINDOW (load_img_dialog));
		
		g_signal_connect (load_img_dialog, "response",
				G_CALLBACK (set_file_path_deprecated),
				user_data);

	#endif
}
