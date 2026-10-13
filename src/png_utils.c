#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <png.h>
#include <stdio.h>
#ifdef _WIN32
	#include <winsock2.h>
	#include <stdint.h>
#else
	#include <arpa/inet.h>
#endif
#include "constants.h"
#include "file_utils.h"
#include "str_utils.h"
#include "structs.h"
#include "toast_notification.h"
#include "widgets_cb.h"

#define MAX_METADATA_PROMPT_LENGTH 2048
#define MAX_PROPERTY_LENGTH 512

int get_png_dimensions(const char *filename, uint32_t *w, uint32_t *h)
{
	const uint8_t png_signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
	uint8_t buffer[8];
	uint32_t data[2];

	FILE *f = g_fopen(filename, "rb");
	if (!f) { g_printerr("Could not open png file: '%s'\n", filename); return -1; }

	if (fread(buffer, 1, 8, f) != 8) { fclose(f); return -1; }

	if (memcmp(buffer, png_signature, 8) != 0) { fclose(f); g_printerr("The file is not a png file."); return -1; }

	if (fseek(f, 8, SEEK_CUR) != 0) { fclose(f); return -1; }

	if (fread(data, 4, 2, f) != 2) { fclose(f); return -1; }

	fclose(f);

	*w = ntohl(data[0]);
	*h = ntohl(data[1]);
	
	printf("Ref image dimensions: W=%dx%d\n", *w, *h);

	return 0;
}

static void set_png_metadata(gchar *path, int copy_info, gpointer user_data)
{
	gboolean set_widgets = TRUE;
	gboolean copy_all = FALSE;
	gboolean copy_pp = FALSE;
	gboolean copy_np = FALSE;

	if (copy_info > 0 && copy_info <= 3) set_widgets = FALSE;

	if (copy_info == 1) {
		copy_all = TRUE;
		copy_pp = TRUE;
		copy_np = TRUE;
	} else if (copy_info == 2) {
		copy_pp = TRUE;
	} else if (copy_info == 3) {
		copy_np = TRUE;
	}
	
	LoadPNGData *data = user_data;

	FILE *fp = g_fopen(path, "rb");
	if (!fp) {
		fprintf(stderr, "File opening failed.\n");
		g_free(path);
		show_simple_message(data->win, "Error loading PNG info", "File opening failed", 1);
		return;
	}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fprintf(stderr, "png_create_read_struct failed.\n");
		fclose(fp);
		g_free(path);
		show_simple_message(data->win, "Error loading PNG info", "png_create_read_struct failed", 1);
		return;
	}

	png_infop info = png_create_info_struct(png);
	if (!info) {
		fprintf(stderr, "png_create_info_struct failed.\n");
		png_destroy_read_struct(&png, NULL, NULL);
		fclose(fp);
		g_free(path);
		show_simple_message(data->win, "Error loading PNG info", "png_create_info_struct failed", 1);
		return;
	}

	if (setjmp(png_jmpbuf(png))) {
		fprintf(stderr, "Error during PNG initialization.\n");
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		g_free(path);
		show_simple_message(data->win, "Error loading PNG info", "Error during PNG initialization", 1);
		return;
	}

	png_init_io(png, fp);
	png_read_info(png, info);

	png_textp text;
	int num_text;
	png_get_text(png, info, &text, &num_text);

	GString *l1 = g_string_new(NULL);
	GString *l2 = g_string_new(NULL);

	for (int i = 0; i < num_text; i++) {
		g_string_append_printf(l1, "%s %s", text[i].key, text[i].text);
	}

	int n_err = 0;
	const char *ptr;

	//Set Positive Prompt
	ptr = strstr(l1->str, "parameters ");
	if (ptr && (set_widgets || copy_pp)) {
		const char *start = ptr + strlen("parameters ");
		const char *neg_end = strstr(start, "\nNegative prompt:");
		const char *steps_end = strstr(start, "\nSteps:");
		const char *end = neg_end ? neg_end : steps_end;
		if (start && end && start < end) {
			size_t len = end - start;
			if (len >= MAX_METADATA_PROMPT_LENGTH) {
				len = MAX_METADATA_PROMPT_LENGTH - 1;
			}
			char pprompt[MAX_METADATA_PROMPT_LENGTH];
			strncpy(pprompt, start, len);
			pprompt[len] = '\0';

			if (set_widgets) {
				GtkTextBuffer *ptb = data->pos_tb;
				gtk_text_buffer_set_text (ptb, pprompt, -1);
			} else if (copy_all) {
				g_string_append_printf(l2, "Prompt: %s\n", pprompt);
			} else if (copy_pp) {
				GdkClipboard *clipboard = gtk_widget_get_clipboard(data->win);
				gdk_clipboard_set_text (clipboard, pprompt);
			}
		} else {
			fprintf(stderr, "Failed to parse positive prompt data.\n");
			n_err++;
		}
	} else if (!ptr) {
		fprintf(stderr, "Failed to parse positive prompt data.\n");
		n_err++;
	}
	
	
	//Set Negative Prompt
	ptr = strstr(l1->str, "Negative prompt: ");
	if (ptr && (set_widgets || copy_np)) {
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

			if (set_widgets) {
				GtkTextBuffer *ntb = data->neg_tb;
				gtk_text_buffer_set_text (ntb, nprompt, -1);
			} else if (copy_all) {
				g_string_append_printf(l2, "Negative Prompt: %s\n", nprompt);
			} else if (copy_np) {
				GdkClipboard *clipboard = gtk_widget_get_clipboard(data->win);
				gdk_clipboard_set_text (clipboard, nprompt);
			}
		} else {
			fprintf(stderr, "Failed to parse negative prompt data.\n");
			n_err++;
		}
	} else if (!ptr) {
		if (n_err > 0) {
			fprintf(stderr, "Failed to parse negative prompt data.\n");
			n_err++;
		} else if (n_err == 0 && set_widgets) {
			GtkTextBuffer *ntb = data->neg_tb;
			gtk_text_buffer_set_text(ntb, "", -1);
		}
	}
	
	//Set Steps
	ptr = strstr(l1->str, "Steps: ");
	if (ptr && (set_widgets || copy_all)) {
		int steps_int;
		if (sscanf(ptr + strlen("Steps: "), "%d", &steps_int) == 1) {
			if (set_widgets) {
				GtkWidget *steps_spin = data->steps_spin;
				gtk_spin_button_set_value (GTK_SPIN_BUTTON(steps_spin), steps_int);
			} else if (copy_all) {
				g_string_append_printf(l2, "Steps: %d\n", steps_int);
			}
		} else {
			fprintf(stderr, "Failed to parse steps data.\n");
			n_err++;
		}
	} else if (!ptr) {
		fprintf(stderr, "Failed to parse steps data.\n");
		n_err++;
	}
	
	//Set CFG Value
	ptr = strstr(l1->str, "CFG scale: ");
	if (ptr && (set_widgets || copy_all)) {
		double cfg_double;
		if (sscanf(ptr + strlen("CFG scale: "), "%lf", &cfg_double) == 1) {
			if (set_widgets) {
				GtkWidget *cfg_spin = data->cfg_spin;
				gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), cfg_double);
			} else if (copy_all) {
				g_string_append_printf(l2, "CFG scale: %.1f\n", cfg_double);
			}
		} else {
			fprintf(stderr, "Failed to parse CFG data.\n");
			n_err++;
		}
	} else if (!ptr) {
		fprintf(stderr, "Failed to parse CFG data.\n");
		n_err++;
	}
	
	//Set Seed Value
	ptr = strstr(l1->str, "Seed: ");
	if (ptr && (set_widgets || copy_all)) {
		long long int seed_long;
		if (sscanf(ptr + strlen("Seed: "), "%lld", &seed_long) == 1) {
			GtkEntry *seed_entry = GTK_ENTRY(data->seed_entry);
			char *seed_str;
			seed_str = convert_long_long_int_to_string(seed_long);
			if (seed_str != NULL) {
				if (set_widgets) {
					gtk_editable_set_text(GTK_EDITABLE(seed_entry), seed_str);
				} else if (copy_all) {
					g_string_append_printf(l2, "Seed: %lld\n", seed_long);
				}
				free(seed_str);
			} else {
				fprintf(stderr, "Failed to parse seed data.\n");
				gtk_editable_set_text(GTK_EDITABLE(seed_entry), "-1");
			}
		} else {
			fprintf(stderr, "Failed to parse seed data.\n");
			n_err++;
		}
	} else if (!ptr) {
		fprintf(stderr, "Failed to parse seed data.\n");
		n_err++;
	}
	
	//Set IMG Size
	ptr = strstr(l1->str, "Size: ");
	if (ptr && (set_widgets || copy_all)) {
		int width_int;
		int heigth_int;
		if (sscanf(ptr + strlen("Size: "), "%dx%d", &width_int, &heigth_int) == 2) {
			if (set_widgets) {
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
			} else if (copy_all) {
				g_string_append_printf(l2, "Width: %d, Height: %d\n", width_int, heigth_int);
			}
		} else {
			fprintf(stderr, "Failed to parse image size data.\n");
			n_err++;
		}
	} else if (!ptr) {
		fprintf(stderr, "Failed to parse image size data.\n");
		n_err++;
	}
	
	//Set SD Checkpoint
	ptr = strstr(l1->str, "Model: ");
	if (ptr && (set_widgets || copy_all)) {
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

			if (set_widgets) {
				GtkWidget *checkpoint_dd = data->checkpoint_dd;
				GtkStringList *checkpoint_items = GTK_STRING_LIST(gtk_drop_down_get_model(GTK_DROP_DOWN(checkpoint_dd)));
				int checkpoint_i = check_gtk_list_contains_item(checkpoint_items, checkpoint_str);
				gtk_drop_down_set_selected(GTK_DROP_DOWN(checkpoint_dd), checkpoint_i);
			} else if (copy_all) {
				g_string_append_printf(l2, "Checkpoint: %s\n", checkpoint_str);
			}
		} else {
			fprintf(stderr, "Failed to parse checkpoint data.\n");
			n_err++;
		}
	} else if (!ptr) {
		fprintf(stderr, "Failed to parse checkpoint data.\n");
		n_err++;
	}
	
	//Set Sampler and Scheduler
	ptr = strstr(l1->str, "Sampler: ");
	if (ptr && (set_widgets || copy_all)) {
		char sampler_str[MAX_PROPERTY_LENGTH];
		char scheduler_str[MAX_PROPERTY_LENGTH];
		if (sscanf(ptr + strlen("Sampler: "), "%s %[^,],", sampler_str, scheduler_str) == 2) {
			if (set_widgets) {
				GtkWidget *sampler_dd = data->sampler_dd;
				GtkWidget *scheduler_dd = data->scheduler_dd;
				
				int sampler_index = check_list_contains_item(LIST_SAMPLES, sampler_str);
				int scheduler_index = check_list_contains_item(LIST_SCHEDULES, scheduler_str);
				
				gtk_drop_down_set_selected(GTK_DROP_DOWN(sampler_dd), sampler_index);
				gtk_drop_down_set_selected(GTK_DROP_DOWN(scheduler_dd), scheduler_index);
			} else if (copy_all) {
				g_string_append_printf(l2, "Sampler: %s, Scheduler: %s\n", sampler_str, scheduler_str);
			}
		} else {
			fprintf(stderr, "Failed to parse sampler data.\n");
			n_err++;
		}
	} else if (!ptr) {
		fprintf(stderr, "Failed to parse sampler data.\n");
		n_err++;
	}
	
	//Set VAE
	ptr = strstr(l1->str, ", VAE: ");
	if (ptr && (set_widgets || copy_all)) {
		const char *start = ptr + strlen(", VAE: ");
		const char *end = strstr(start, ",");
		if (start && end && start < end) {
			size_t len = end - start;
			if (len >= MAX_PROPERTY_LENGTH) {
				len = MAX_PROPERTY_LENGTH - 1;
			}
			char vae_str[MAX_PROPERTY_LENGTH];
			strncpy(vae_str, start, len);
			vae_str[len] = '\0';

			if (set_widgets) {
				GtkStringList *vae_items = GTK_STRING_LIST(gtk_drop_down_get_model(GTK_DROP_DOWN(data->vae_dd)));
				int vae_i = check_gtk_list_contains_item(vae_items, vae_str);
				gtk_drop_down_set_selected(GTK_DROP_DOWN(data->vae_dd), vae_i);
			} else if (copy_all) {
				g_string_append_printf(l2, "VAE: %s\n", vae_str);
			}
		} else {
			fprintf(stderr, "Failed to parse VAE data.\n");
			if (n_err == 0) gtk_drop_down_set_selected(GTK_DROP_DOWN(data->vae_dd), 0);
			n_err++;
		}
	} else if (!ptr) {
		if (n_err > 0) {
			fprintf(stderr, "Failed to parse VAE data.\n");
			n_err++;
		} else if (n_err == 0 && set_widgets) {
			gtk_drop_down_set_selected(GTK_DROP_DOWN(data->vae_dd), 0);
		}
	}
	
	//Set Clip Skip
	ptr = strstr(l1->str, ", Clip skip: ");
	if (ptr && (set_widgets || copy_all)) {
		int clip_skip_value;
		if (sscanf(ptr + strlen(", Clip skip: "), "%d", &clip_skip_value) == 1) {
			if (set_widgets) {
				gtk_spin_button_set_value (GTK_SPIN_BUTTON(data->clip_skip_spin), clip_skip_value);
			} else if (copy_all) {
				g_string_append_printf(l2, "Clip skip: %d\n", clip_skip_value);
			}
		} else {
			fprintf(stderr, "Failed to parse Clip skip data.\n");
			n_err++;
		}
	} else if (!ptr) {
		fprintf(stderr, "Failed to parse Clip skip data.\n");
		n_err++;
	}
	
	//Set Hires upscaler
	ptr = strstr(l1->str, ", Hires upscale: ");
	if (ptr && (set_widgets || copy_all)) {
		const char *start = ptr + strlen(", Hires upscale: ");
		const char *end = strstr(start, ",");
		if (start && end && start < end) {
			size_t len = end - start;
			if (len >= MAX_PROPERTY_LENGTH) {
				len = MAX_PROPERTY_LENGTH - 1;
			}
			char hires_upscaler_str[MAX_PROPERTY_LENGTH];
			strncpy(hires_upscaler_str, start, len);
			hires_upscaler_str[len] = '\0';
			
			if (set_widgets) {
				GtkStringList *hires_upscaler_items = GTK_STRING_LIST(gtk_drop_down_get_model(GTK_DROP_DOWN(data->hires_upscaler_dd)));
				int hires_upscaler_i = check_gtk_list_contains_item(hires_upscaler_items, hires_upscaler_str);
				gtk_drop_down_set_selected(GTK_DROP_DOWN(data->hires_upscaler_dd), hires_upscaler_i);
			} else if (copy_all) {
				g_string_append_printf(l2, "Hires upscale: %s\n", hires_upscaler_str);
			}
		} else {
			fprintf(stderr, "Failed to parse Hires upscale data.\n");
			if (n_err == 0) gtk_drop_down_set_selected(GTK_DROP_DOWN(data->hires_upscaler_dd), 0);
			n_err++;
		}
	} else if (!ptr) {
		if (n_err > 0) {
			fprintf(stderr, "Failed to parse Hires upscale data.\n");
			n_err++;
		} else if (n_err == 0 && set_widgets) {
			gtk_drop_down_set_selected(GTK_DROP_DOWN(data->hires_upscaler_dd), 0);
		}
	}
	
	//Set Hires Scale Value
	ptr = strstr(l1->str, ", Hires scale: ");
	if (ptr && (set_widgets || copy_all)) {
		double hires_scale_double;
		if (sscanf(ptr + strlen(", Hires scale: "), "%lf", &hires_scale_double) == 1) {
			if (set_widgets) {
				GtkWidget *hires_scale_spin = data->hires_scale_spin;
				gtk_spin_button_set_value (GTK_SPIN_BUTTON(hires_scale_spin), hires_scale_double);
			} else if (copy_all) {
				g_string_append_printf(l2, "Hires scale: %.2f\n", hires_scale_double);
			}
		} else {
			fprintf(stderr, "Failed to parse Hires scale data.\n");
			n_err++;
		}
	} else if (!ptr) {
		if (n_err > 0) { fprintf(stderr, "Failed to parse Hires scale data.\n"); n_err++; }
	}
	
	//Set Hires Resize Value
	/*ptr = strstr(l1->str, ", Hires resize: ");
	if (ptr) {
		int hires_width_int;
		int hires_heigth_int;
	} else if (!ptr) {
		fprintf(stderr, "Failed to parse Hires resize data.\n");
		n_err++;
	}*/
	
	//Set Hires Steps
	ptr = strstr(l1->str, ", Hires steps: ");
	if (ptr && (set_widgets || copy_all)) {
		int hires_steps_int;
		if (sscanf(ptr + strlen(", Hires steps: "), "%d", &hires_steps_int) == 1) {
			if (set_widgets) {
				GtkWidget *hires_steps_spin = data->hires_steps_spin;
				gtk_spin_button_set_value (GTK_SPIN_BUTTON(hires_steps_spin), hires_steps_int);
			} else if (copy_all) {
				g_string_append_printf(l2, "Hires steps: %d\n", hires_steps_int);
			}
		} else {
			fprintf(stderr, "Failed to parse Hires steps data.\n");
			n_err++;
		}
	} else if (!ptr) {
		if (n_err > 0) { fprintf(stderr, "Failed to parse Hires steps data.\n"); n_err++; }
	}
	
	//Set Hires Denoising strength
	ptr = strstr(l1->str, ", Denoising strength: ");
	if (ptr && (set_widgets || copy_all)) {
		double hires_denoising_str_double;
		if (sscanf(ptr + strlen(", Denoising strength: "), "%lf", &hires_denoising_str_double) == 1) {
			if (set_widgets) {
				GtkWidget *hires_denoise_spin = data->hires_denoise_spin;
				gtk_spin_button_set_value (GTK_SPIN_BUTTON(hires_denoise_spin), hires_denoising_str_double);
			} else if (copy_all) {
				g_string_append_printf(l2, "Hires Denoising strength: %.2f\n", hires_denoising_str_double);
			}
		} else {
			fprintf(stderr, "Failed to parse Hires denoising strength data.\n");
			n_err++;
		}
	} else if (!ptr) {
		if (n_err > 0) { fprintf(stderr, "Failed to parse Hires denoising strength data.\n"); n_err++; }
	}

	//Set Denoise strength
	ptr = strstr(l1->str, ",\"strength\":");
	if (ptr && (set_widgets || copy_all)) {
		double denoise_str_value;
		if (sscanf(ptr + strlen(",\"strength\":"), "%lf", &denoise_str_value) == 1) {
			if (denoise_str_value >= 0 && denoise_str_value <= 1) {
				if (set_widgets) {
					GtkWidget *denoise_spin = data->denoise_spin;
					gtk_spin_button_set_value (GTK_SPIN_BUTTON(denoise_spin), denoise_str_value);
				} else if (copy_all) {
					g_string_append_printf(l2, "Denoising strength: %.2f\n", denoise_str_value);
				}
			}
		} else {
			fprintf(stderr, "Failed to parse Denoise strength data.\n");
			n_err++;
		}
	} else if (!ptr) {
		fprintf(stderr, "Failed to parse Denoise strength data.\n");
		n_err++;
	}

	if (copy_all) {
		g_string_append_printf(l2, "Application: %s (stable-diffusion.cpp)", APP_NAME_VERSION);
		GdkClipboard *clipboard = gtk_widget_get_clipboard(data->win);
		gdk_clipboard_set_text (clipboard, l2->str);
		show_toast_notification(GTK_OVERLAY(data->main_overlay), "Generation Info Copied!");
	} else if (copy_pp) {
		show_toast_notification(GTK_OVERLAY(data->main_overlay), "Positive Prompt Copied!");
	} else if (copy_np) {
		show_toast_notification(GTK_OVERLAY(data->main_overlay), "Negative Prompt Copied!");
	} else if (set_widgets) {
		show_toast_notification(GTK_OVERLAY(data->main_overlay), "Generation Info Loaded!");
	}

	png_destroy_read_struct(&png, &info, NULL);
	fclose(fp);
	g_free(path);
	g_string_free(l1, TRUE);
	g_string_free(l2, TRUE);
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
		set_png_metadata(img_path, 0, user_data);
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
				gtk_widget_add_css_class(data->img2img_visibility_toggle_btn, "custom_btn_img2img");
				clear_mask_btn_cb(NULL, user_data);
				show_toast_notification(GTK_OVERLAY(data->main_overlay), "Input image set.");
			} else {
				g_printerr(
					"Error loading image: The file '%s' is missing, corrupted, or invalid.\n",
					path
				);
				show_simple_message(data->win,
					"Error loading image",
					"Error loading image: The selected file is missing, corrupted, or invalid.\n", 1
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
			set_png_metadata(img_path, 0, user_data);
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
					gtk_widget_add_css_class(data->img2img_visibility_toggle_btn, "custom_btn_img2img");
					clear_mask_btn_cb(NULL, user_data);
					show_toast_notification(GTK_OVERLAY(data->main_overlay), "Input image set.");
				} else {
					g_printerr(
						"Error loading image: The file '%s' is missing, corrupted, or invalid.\n",
						path
					);
					show_simple_message(data->win,
						"Error loading image",
						"Error loading image: The selected file is missing, corrupted, or invalid.\n", 1
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

void load_metadata_from_favorites(GtkWidget *btn, gpointer user_data)
{
	if (user_data == NULL) { g_printerr("Critical error in function 'load_metadata_from_favorites'.\n"); return; }
	
	GtkWidget *fav_image = GTK_WIDGET(user_data);
	FavoritesWindowData *favorites_d = g_object_get_data(G_OBJECT(btn), "favorites_d");

	if (favorites_d == NULL || fav_image == NULL) { g_printerr("Critical error in function 'load_metadata_from_favorites'.\n"); return; }
	
	gchar *img_path = g_strdup(g_object_get_data(G_OBJECT(fav_image), "image-path"));
	if (img_path == NULL) { g_printerr("Failed to get image path.\n"); return; }

	if (check_file_exists(img_path, 0) == 1) {
		const char *btn_label = gtk_button_get_label(GTK_BUTTON(btn));

		if (btn_label == NULL) {
			set_png_metadata(img_path, 0, favorites_d->load_png_info_d);
		} else if (g_strcmp0 (btn_label, "Copy Negative Prompt") == 0) {
			set_png_metadata(img_path, 3, favorites_d->load_png_info_d);
		} else if (g_strcmp0 (btn_label, "Copy Positive Prompt") == 0) {
			set_png_metadata(img_path, 2, favorites_d->load_png_info_d);
		} else if (g_strcmp0 (btn_label, "Copy All") == 0) {
			set_png_metadata(img_path, 1, favorites_d->load_png_info_d);
		} else {
			g_printerr("Error in function 'load_metadata_from_favorites': The label '%s' is not recognized.\n", btn_label);
			set_png_metadata(img_path, 0, favorites_d->load_png_info_d);
		}
	} else {
		g_printerr("File '%s' does not exist or cannot be accessed.\n", img_path);
		g_free(img_path);
	}

	GtkWidget *menu_btn_popover = gtk_widget_get_ancestor(btn, GTK_TYPE_POPOVER);
	if (menu_btn_popover) gtk_popover_popdown(GTK_POPOVER(menu_btn_popover));
}

void load_from_img_preview(GtkWidget *btn, gpointer user_data)
{
	LoadPNGData *data = user_data;
	
	if (data->image_files->len > 0) {
		gchar *img_path = g_strdup(g_ptr_array_index(data->image_files, *data->current_image_index));
		
		if (img_path != NULL) {
			const char *btn_label = gtk_button_get_label(GTK_BUTTON(btn));
			
			if (btn_label == NULL) {
				set_png_metadata(img_path, 0, user_data);
			} else if (g_strcmp0 (btn_label, "Copy Negative Prompt") == 0) {
				set_png_metadata(img_path, 3, user_data);
			} else if (g_strcmp0 (btn_label, "Copy Positive Prompt") == 0) {
				set_png_metadata(img_path, 2, user_data);
			} else if (g_strcmp0 (btn_label, "Copy All") == 0) {
				set_png_metadata(img_path, 1, user_data);
			} else {
				g_printerr("Error in function 'load_from_img_preview': The label '%s' is not recognized.\n", btn_label);
				set_png_metadata(img_path, 0, user_data);
			}
		} else {
			g_printerr("Failed to get image path.\n");
		}
	} else {
		g_printerr("No images in the 'output' directory, using default image.\n");
		gchar *img_path = g_strdup(DEFAULT_IMG_PATH);
		set_png_metadata(img_path, 0, user_data);
	}
	GtkWidget *menu_btn_popover = gtk_widget_get_ancestor(btn, GTK_TYPE_POPOVER);
	if (menu_btn_popover) gtk_popover_popdown(GTK_POPOVER(menu_btn_popover));
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
	LoadImg2ImgData *data = user_data;
	GtkWindow *win = GTK_WINDOW(data->win);

	#if GTK_CHECK_VERSION(4, 10, 0)
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

void set_current_preview_to_img2img(GtkWidget *btn, gpointer user_data)
{
	LoadImg2ImgFromPreviewData *data = user_data;
	
	gtk_check_button_set_active(GTK_CHECK_BUTTON(data->inpaint_check), FALSE);
	clear_img2img_overlay(GTK_OVERLAY(data->overlay_img2img));

	char *path = g_ptr_array_index(data->image_files, *data->current_image_index);
	if (path != NULL && data->image_files->len > 0 && check_file_exists(path, 0) == 1) {
		g_string_assign(data->img2img_file_path, path);
		gtk_picture_set_filename(GTK_PICTURE(data->image_wgt), path);
		gtk_widget_add_css_class(data->img2img_visibility_toggle_btn, "custom_btn_img2img");
		show_toast_notification(GTK_OVERLAY(data->main_overlay), "Input image set.");
	} else {
		g_printerr("Error loading current preview image into img2img.\n");
	}
}

