#include <gtk/gtk.h>
#include <ctype.h>
#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "constants.h"
#include "file_utils.h"
#include "str_utils.h"
#include "structs.h"

static void on_get_backend_info_end(GObject *source, GAsyncResult *res, gpointer user_data);
void show_info_message (GtkWidget *wgt, GtkWidget *main_win);
void show_simple_message (GtkWidget *win, char *msg_title_text, char *msg_text, int is_error);

void add_dropdown_selected_item_textview (GtkWidget* wgt, GParamSpec *pspec, gpointer user_data)
{
	if (GTK_IS_DROP_DOWN(wgt)) {
		DropDownTextBufferData *data = user_data;
		GtkDropDown *dd = GTK_DROP_DOWN(wgt);

		int tb_type = data->tb_type;
		GtkTextBuffer *tv_tb = data->textbuffer;

		GtkStringObject *selected_item = gtk_drop_down_get_selected_item(dd);
		const char* item_string = gtk_string_object_get_string(selected_item);

		if (strcmp(item_string, "None") != 0) {
			char *text = format_lora_embedding_string(item_string, tb_type);
			GtkTextIter si;
			GtkTextIter ei;
			GtkTextIter sel_i;
			gtk_text_buffer_get_bounds (tv_tb, &si, &ei);
			if (tb_type == 0) {
				sel_i = ei;
			} else {
				sel_i = si;
			}
			gtk_text_buffer_insert (tv_tb, &sel_i, text, -1);
			free(text);
		}
	}
}

void app_start_data_free (gpointer user_data)
{
	if (user_data == NULL) return;
	AppStartData *data = user_data;

	GenerationSnapshotData *left_jobs;
	while ((left_jobs = g_queue_pop_head(data->job_queue)) != NULL) {
		g_free(left_jobs->output_path);
		g_free(left_jobs->img2img_file_path);
		g_free(left_jobs->positive_prompt);
		g_free(left_jobs->negative_prompt);
		g_free(left_jobs->checkpoint_filename);
		g_free(left_jobs->detector_filename);
		g_free(left_jobs->vae_filename);
		g_free(left_jobs->cnet_filename);
		g_free(left_jobs->upscaler_filename);
		g_free(left_jobs->clip_l_filename);
		g_free(left_jobs->clip_g_filename);
		g_free(left_jobs->text_enc_filename);
		g_free(left_jobs);
	}
	g_queue_free(data->job_queue);
	
	if (data->checkpoint_string != NULL) {
		g_string_free(data->checkpoint_string, TRUE);
		data->checkpoint_string = NULL;
	}
	
	if (data->detector_string != NULL) {
		g_string_free(data->detector_string, TRUE);
		data->detector_string = NULL;
	}
	
	if (data->vae_string != NULL) {
		g_string_free(data->vae_string, TRUE);
		data->vae_string = NULL;
	}
	
	if (data->cnet_string != NULL) {
		g_string_free(data->cnet_string, TRUE);
		data->cnet_string = NULL;
	}
	
	if (data->upscaler_string != NULL) {
		g_string_free(data->upscaler_string, TRUE);
		data->upscaler_string = NULL;
	}
	
	if (data->clip_l_string != NULL) {
		g_string_free(data->clip_l_string, TRUE);
		data->clip_l_string = NULL;
	}
	
	if (data->clip_g_string != NULL) {
		g_string_free(data->clip_g_string, TRUE);
		data->clip_g_string = NULL;
	}
	
	if (data->text_enc_string != NULL) {
		g_string_free(data->text_enc_string, TRUE);
		data->text_enc_string = NULL;
	}
	
	if (data->img2img_file_path != NULL) {
		g_string_free(data->img2img_file_path, TRUE);
		data->img2img_file_path = NULL;
	}
	
	if (data->preview_label_string != NULL) {
		g_string_free(data->preview_label_string, TRUE);
		data->preview_label_string = NULL;
	}
	
	if (data->preview_image_files != NULL) {
		g_ptr_array_free(data->preview_image_files, TRUE);
		data->preview_image_files = NULL;
	}
	
	if (data->sd_cmd_array != NULL) {
		g_ptr_array_free(data->sd_cmd_array, TRUE);
		data->sd_cmd_array = NULL;
	}
}

void array_strings_free (const char **list)
{
	if (list != NULL) {
		for (int i = 0; list[i] != NULL; i++) {
			free((char*)list[i]);
		}
		free(list);
	}
}

void clear_img2img_overlay(GtkOverlay *overlay)
{
	GtkWidget *base_child = gtk_overlay_get_child(overlay);
	GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(overlay));
	
	while (child != NULL) {
		GtkWidget *next = gtk_widget_get_next_sibling(child);
		if (child != base_child) gtk_overlay_remove_overlay(overlay, child);
		child = next;
	}
}

void clear_mask_btn_cb (GtkButton *btn, gpointer user_data)
{
	LoadImg2ImgData *data = user_data;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(data->inpaint_check), FALSE);
	clear_img2img_overlay(GTK_OVERLAY(data->overlay_img2img));
}

void clear_img2img_btn_cb (GtkButton *btn, gpointer user_data)
{
	LoadImg2ImgData *data = user_data;
	gtk_widget_remove_css_class(data->img2img_expander, "img2img_active");
	GString *gstr = data->img2img_file_path;
	g_string_assign(gstr, "None");
	GtkPicture *preview_img = GTK_PICTURE(data->image_wgt);
	gtk_picture_set_filename(preview_img, EMPTY_IMG_PATH);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(data->detector_check), FALSE);
	clear_mask_btn_cb(NULL, user_data);
}

gboolean close_app_callback (GtkWindow *win, gpointer user_data)
{
	app_start_data_free(user_data);
	gtk_window_destroy (win);
}

void donate_btn_callback(GtkButton *btn, gpointer user_data)
{
	GError *error = NULL;

	g_app_info_launch_default_for_uri(DONATE_URL, NULL, &error);

	if (error != NULL) {
		g_printerr("Failed to open donate URL: %s\n", error->message);
		g_error_free(error);
	}
}

static gboolean steal_scroll_cb (GtkEventControllerScroll *controller, double dx, double dy, gpointer user_data)
{
	GtkScrolledWindow *properties_scrollable = GTK_SCROLLED_WINDOW(user_data);

	GtkAdjustment *vadjustment = gtk_scrolled_window_get_vadjustment(properties_scrollable);
	double current = gtk_adjustment_get_value(vadjustment);
	double step = gtk_adjustment_get_step_increment(vadjustment);
	gtk_adjustment_set_value(vadjustment, current + dy * step);

	return TRUE;
}

void dropdown_items_update (const char *path, GtkWidget *dd, GApplication *app)
{
	GError *err = NULL;
	GtkStringList *new_dd_items = get_files(path, &err);
	if (new_dd_items == NULL) {
		if (err != NULL) {
			g_printerr("Error: %s\n", err->message);
			g_error_free(err);
			GtkWindow *win = GTK_WINDOW(gtk_application_get_active_window(GTK_APPLICATION(app)));
			gtk_window_close(win);
		}
	} else {
		gtk_drop_down_set_model(GTK_DROP_DOWN(dd), G_LIST_MODEL(new_dd_items));
		g_object_unref(new_dd_items);
	}
}

void free_cache_data (MyCacheData *s)
{
	if (s == NULL) {
		return;
	}
	free(s->pos_p);
	free(s->neg_p);
	free(s->img_name);
	free(s);
}

void free_preview_data (gpointer data)
{
	PreviewImageData *preview_d = (PreviewImageData *)data;
	g_free(preview_d);
}

void get_backend_info(GtkButton *btn, gpointer user_data)
{
	gchar *sd_bin;

	#ifdef G_OS_WIN32
		gchar *current_dir = g_get_current_dir();
		sd_bin = g_strdup_printf("%s\\sd", current_dir);
		g_free(current_dir);
	#else
		sd_bin = g_strdup("./sd");
	#endif

	GError *error = NULL;
	GSubprocess *info_process = g_subprocess_new(G_SUBPROCESS_FLAGS_STDOUT_PIPE, &error, sd_bin, "--list-devices", NULL);

	g_free(sd_bin);

	if (info_process == NULL) {
		g_printerr("Failed to spawn: %s\n", error->message);
		g_error_free(error);
		return;
	}
	g_subprocess_communicate_async(info_process, NULL, NULL, on_get_backend_info_end, user_data);
}

guint get_dd_item_count(GtkDropDown *dropdown)
{
	GListModel *model = gtk_drop_down_get_model(dropdown);

	if (model == NULL) return 0;
	return g_list_model_get_n_items(model);
}

void kill_stable_diffusion_process (GtkButton *btn, gpointer user_data)
{
	int *sdpid = (int *)user_data;
	gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);
	
	if (*sdpid > 0) {
		#ifdef _WIN32
			HANDLE hp = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)*sdpid);
			if (hp == NULL) {
				g_printerr("Failed to open process %d. Error: %lu\n", *sdpid, GetLastError());
			} else {
				if (TerminateProcess(hp, 1)) {
					printf("Process %d terminated successfully.\n", *sdpid);
					*sdpid = 0;
				} else {
					g_printerr("Failed to terminate process %d. Error: %lu\n", *sdpid, GetLastError());
				}
				CloseHandle(hp);
			}
		#else
			if (kill(*sdpid, SIGKILL) == 0) {
				printf("Process %d killed successfully.\n", *sdpid);
				*sdpid = 0;
			} else {
				g_printerr("Error killing process.\n");
			}
		#endif
	} else {
		g_printerr("Error killing process.\n");
	}
}

void kill_cancel_all_btn_cb (GtkButton *btn, gpointer user_data)
{
	CancelAllData *data = user_data;
	GenerationSnapshotData *left_jobs;
	while ((left_jobs = g_queue_pop_head(data->job_queue)) != NULL) {
		g_free(left_jobs->output_path);
		g_free(left_jobs->img2img_file_path);
		g_free(left_jobs->positive_prompt);
		g_free(left_jobs->negative_prompt);
		g_free(left_jobs->checkpoint_filename);
		g_free(left_jobs->detector_filename);
		g_free(left_jobs->vae_filename);
		g_free(left_jobs->cnet_filename);
		g_free(left_jobs->upscaler_filename);
		g_free(left_jobs->clip_l_filename);
		g_free(left_jobs->clip_g_filename);
		g_free(left_jobs->text_enc_filename);
		g_free(left_jobs);
	}

	kill_stable_diffusion_process(GTK_BUTTON(data->halt_btn), data->sdpid);
}

static void navigate_images(PreviewImageData *data, int offset)
{
	gsize img_count = data->image_files->len;

	if (img_count <= 0) {
		g_printerr("Error: No images found.\n");
		return;
	}

	if (g_strcmp0 (gtk_button_get_icon_name(GTK_BUTTON(data->hide_img_btn)), "view-conceal-symbolic") == 0) {
		gtk_button_set_icon_name (GTK_BUTTON(data->hide_img_btn), "view-reveal-symbolic");
	}

	int new_index = (*data->current_image_index + offset) % (int)img_count;

	if (new_index < 0) {
		new_index += img_count;
	}

	*data->current_image_index = new_index;

	const gchar *current_image_path = g_ptr_array_index(data->image_files, *data->current_image_index);

	g_string_erase(data->img_index_string, 0, -1);
	g_string_append_printf(data->img_index_string, "(%d / %d) %s", *data->current_image_index + 1, (int)img_count, current_image_path + 8);

	gtk_label_set_label(GTK_LABEL(data->img_index_label), data->img_index_string->str);
	gtk_picture_set_filename(GTK_PICTURE(data->image_widget), current_image_path);
}

void navigate_10_img_prev(GtkButton* btn, gpointer user_data)
{
	navigate_images((PreviewImageData*)user_data, -10);
}

void navigate_img_prev(GtkButton* btn, gpointer user_data)
{
	navigate_images((PreviewImageData*)user_data, -1);
}

void navigate_img_next(GtkButton* btn, gpointer user_data)
{
	navigate_images((PreviewImageData*)user_data, 1);
}

void navigate_10_img_next(GtkButton* btn, gpointer user_data)
{
	navigate_images((PreviewImageData*)user_data, 10);
}

void on_mask_area_destroy (GtkWindow *win, gpointer user_data)
{
	MaskData *data = (MaskData *)user_data;
	
	if (data->surface) {
		cairo_surface_destroy(data->surface);
		data->surface = NULL;
	}
	
	g_free(data);
	
	//TODO: Search for better fix
	gtk_window_destroy(win);
}

void on_boxr_img_destroy (GtkWidget* wgt, gpointer user_data)
{
	PreviewBoxScrollData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_boxr_img_hover_destroy (GtkWidget* wgt, gpointer user_data)
{
	PreviewBoxHoverData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_cancel_all_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	CancelAllData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_clear_img2img_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	LoadImg2ImgData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_dd_const_destroy (GtkWidget* wgt, gpointer user_data)
{
	DropDownModelsNameData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_dd_path_destroy (GtkWidget* wgt, gpointer user_data)
{
	DropDownTextBufferData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_generate_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	GenerationData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

static void on_get_backend_info_end(GObject *source, GAsyncResult *res, gpointer user_data)
{
	GtkWidget *main_win = (GtkWidget *) user_data;
	GSubprocess *info_process = G_SUBPROCESS(source);
	GError *error = NULL;
	GBytes *stdout_buf = NULL;

	if (!g_subprocess_communicate_finish(info_process, res, &stdout_buf, NULL, &error)) {
		g_printerr("Error: %s\n", error->message);
		g_error_free(error);
		g_object_unref(info_process);
		return;
	}

	gsize size;
	const char *data = g_bytes_get_data(stdout_buf, &size);
	

	if (!data || size == 0) {
		show_simple_message(main_win, "Device Detection Failed", "Failed to detect available compute backends.", 1);
		g_bytes_unref(stdout_buf);
		g_object_unref(info_process);
		return;
	}

	GString *msg = g_string_new(NULL);
	char **lines = g_strsplit(data, "\n", -1);	

	for (int i = 0; lines[i] != NULL; i++) {
		char prefix[32];
		int index;
		
		if (strncmp(lines[i], "CPU", 3) == 0) {
			char *cpu_name = lines[i] + 3;
			g_string_append_printf(msg, "[ CPU ] -> %s\n", g_strstrip(cpu_name));
		} else if (sscanf(lines[i], "%31[A-Za-z]%d", prefix, &index) == 2) {
			int offset = 0;
			sscanf(lines[i], "%*31[A-Za-z]%*d%n", &offset);
			char *device_name = lines[i] + offset;
			g_string_append_printf(msg, "[ %s%d ] -> %s\n", prefix, index, g_strstrip(device_name));
		}
	}
	
	show_simple_message(main_win, "Available Backend(s):", msg->str, 0);
	
	g_string_free(msg, TRUE);
	g_strfreev(lines);
	g_bytes_unref(stdout_buf);
	g_object_unref(info_process);
}

void on_hide_img_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	PreviewImageData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_load_from_img_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	LoadPNGData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

static gboolean on_preview_box_scroll_timeout (gpointer user_data)
{
	PreviewBoxScrollData *data = user_data;
	navigate_images((PreviewImageData*)data->preview_d, data->scroll_steps);
	data->scroll_steps = 0;
	data->debounce_id = 0;
	return G_SOURCE_REMOVE;
}

gboolean on_preview_box_scroll (GtkEventControllerScroll *controller, gdouble dx, gdouble dy, gpointer user_data)
{
	PreviewBoxScrollData *data = user_data;
	data->scroll_steps += (dy > 0) - (dy < 0);

	if (data->debounce_id != 0) g_source_remove(data->debounce_id);
	data->debounce_id = g_timeout_add(200, on_preview_box_scroll_timeout, data);
	
	return TRUE;
}

gboolean on_main_win_key_pressed (GtkEventControllerKey *controller, guint keyv, guint keycode, GdkModifierType state, gpointer user_data)
{
	PreviewBoxHoverData *data = user_data;
	if (!data->is_hovering_preview) return FALSE;

	switch (keyv)
	{
		case GDK_KEY_Left:
			navigate_images((PreviewImageData*)data->preview_d, -1);
			break;
		case GDK_KEY_Right:
			navigate_images((PreviewImageData*)data->preview_d, 1);
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

gboolean on_preview_widget_enter(GtkEventControllerMotion *controller, double x, double y, gpointer user_data)
{
	PreviewBoxHoverData *data = user_data;
	data->is_hovering_preview = TRUE;
}

gboolean on_preview_widget_leave(GtkEventControllerMotion *controller, gpointer user_data)
{
	PreviewBoxHoverData *data = user_data;
	data->is_hovering_preview = FALSE;
}

void on_reload_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	ReloadDropDownData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_reset_default_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	ResetCbData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void on_set_img2img_from_preview_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	LoadImg2ImgFromPreviewData *data = user_data;
	if (data == NULL) return;
	g_free(data);
}

void quit_btn_callback (GtkWidget *wgt, GtkWidget *win)
{
	gtk_window_close(GTK_WINDOW(win));
}

void reload_dropdown(GtkWidget* wgt, gpointer user_data)
{
	ReloadDropDownData *data = user_data;
	dropdown_items_update(CHECKPOINTS_PATH, GTK_WIDGET(data->checkpoint_dd), data->app);
	dropdown_items_update(VAES_PATH, GTK_WIDGET(data->vae_dd), data->app);
	dropdown_items_update(CONTROLNET_PATH, GTK_WIDGET(data->cnet_dd), data->app);
	dropdown_items_update(UPSCALES_PATH, GTK_WIDGET(data->upscaler_dd), data->app);
	dropdown_items_update(CLIPS_PATH, GTK_WIDGET(data->clip_l_dd), data->app);
	dropdown_items_update(CLIPS_PATH, GTK_WIDGET(data->clip_g_dd), data->app);
	dropdown_items_update(TEXT_ENCODERS_PATH, GTK_WIDGET(data->text_enc_dd), data->app);
	dropdown_items_update(LORAS_PATH, GTK_WIDGET(data->lora_dd), data->app);
	dropdown_items_update(EMBEDDINGS_PATH, GTK_WIDGET(data->embedding_dd), data->app);
}

void reset_default_btn_cb (GtkWidget* btn, gpointer user_data)
{
	ResetCbData *data = user_data;

	GtkTextBuffer *pos_tb = data->pos_tb;
	gtk_text_buffer_set_text (pos_tb, POSITIVE_PROMPT, -1);

	GtkTextBuffer *neg_tb = data->neg_tb;
	gtk_text_buffer_set_text (neg_tb, NEGATIVE_PROMPT, -1);

	GtkWidget *checkpoint_dd = data->checkpoint_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(checkpoint_dd), DEFAULT_MODELS);
	
	GtkWidget *detector_dd = data->detector_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(detector_dd), DEFAULT_MODELS);

	GtkWidget *vae_dd = data->vae_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(vae_dd), DEFAULT_MODELS);

	GtkWidget *cnet_dd = data->cnet_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(cnet_dd), DEFAULT_MODELS);

	GtkWidget *upscaler_dd = data->upscaler_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(upscaler_dd), DEFAULT_MODELS);
	
	GtkWidget *clip_l_dd = data->clip_l_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(clip_l_dd), DEFAULT_MODELS);
	
	GtkWidget *clip_g_dd = data->clip_g_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(clip_g_dd), DEFAULT_MODELS);
	
	GtkWidget *text_enc_dd = data->text_enc_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(text_enc_dd), DEFAULT_MODELS);

	GtkWidget *cfg_spin = data->cfg_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), DEFAULT_CFG);
	
	GtkWidget *cnet_strength_spin = data->cnet_strength_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(cnet_strength_spin), DEFAULT_CNET_STRENGTH);

	GtkWidget *denoise_spin = data->denoise_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(denoise_spin), DEFAULT_DENOISE);

	GtkWidget *seed_entry = data->seed_entry;
	char seed_str[LONGLONG_STR_SIZE];
	snprintf(seed_str, sizeof(seed_str), "%lld", DEFAULT_SEED);
	gtk_editable_set_text(GTK_EDITABLE(seed_entry), seed_str);

	GtkWidget *upscale_passes_spin = data->upscale_passes_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(upscale_passes_spin), DEFAULT_RP_UPSCALE);

	GtkWidget *lora_dd = data->lora_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(lora_dd), DEFAULT_MODELS);

	GtkWidget *embedding_dd = data->embedding_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(embedding_dd), DEFAULT_MODELS);

	GtkWidget *sampler_dd = data->sampler_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(sampler_dd), DEFAULT_SAMPLER);

	GtkWidget *scheduler_dd = data->scheduler_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(scheduler_dd), DEFAULT_SCHEDULER);

	GtkWidget *width_dd = data->width_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(width_dd), DEFAULT_SIZE);

	GtkWidget *height_dd = data->height_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(height_dd), DEFAULT_SIZE);
	
	GtkWidget *hires_upscaler_dd = data->hires_upscaler_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(hires_upscaler_dd), DISABLED_OPT);
	
	GtkWidget *detector_confidence_spin = data->detector_confidence_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(detector_confidence_spin), DEFAULT_DETECTOR_CONFIDENCE);
	
	GtkWidget *detector_denoise_spin = data->detector_denoise_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(detector_denoise_spin), DEFAULT_DETECTOR_DENOISE);
	
	GtkWidget *detector_inpaint_padding_spin = data->detector_inpaint_padding_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(detector_inpaint_padding_spin), DEFAULT_DETECTOR_INPAINT_PADDING);
	
	GtkWidget *detector_input_size_spin = data->detector_input_size_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(detector_input_size_spin), DEFAULT_DETECTOR_INPUT_SIZE);
	
	GtkWidget *detector_mask_blur_spin = data->detector_mask_blur_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(detector_mask_blur_spin), DEFAULT_DETECTOR_MASK_BLUR);
	
	GtkWidget *hires_scale_spin = data->hires_scale_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(hires_scale_spin), DEFAULT_HIRES_SCALE);
	
	GtkWidget *hires_steps_spin = data->hires_steps_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(hires_steps_spin), DEFAULT_HIRES_STEPS);
	
	GtkWidget *hires_denoise_spin = data->hires_denoise_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(hires_denoise_spin), DEFAULT_HIRES_DENOISE_STR);

	GtkWidget *steps_spin = data->steps_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(steps_spin), DEFAULT_N_STEPS);

	GtkWidget *batch_count_spin = data->batch_count_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(batch_count_spin), DEFAULT_BATCH_COUNT);
	
	GtkWidget *kontext_check = data->kontext_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(kontext_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *detector_check = data->detector_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(detector_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *inpaint_check = data->inpaint_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(inpaint_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *sd_based_check = data->sd_based_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(sd_based_check), ENABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *llm_check = data->llm_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(llm_check), DISABLED_OPT == 1 ? TRUE : FALSE);

	GtkWidget *mmap_check = data->mmap_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(mmap_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *fa_off_btn = data->fa_off_btn;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fa_off_btn), ENABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *taesd_check = data->taesd_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(taesd_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *update_cache_check = data->update_cache_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(update_cache_check), ENABLED_OPT == 1 ? TRUE : FALSE);

	GtkWidget *chroma_dit_mask_check = data->chroma_dit_mask_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(chroma_dit_mask_check), ENABLED_OPT == 1 ? TRUE : FALSE);

	GtkWidget *qwen_zero_cond_t_check = data->qwen_zero_cond_t_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(qwen_zero_cond_t_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *verbose_check = data->verbose_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(verbose_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *vae_tiling_dd = data->vae_tiling_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(vae_tiling_dd), DEFAULT_MODELS);
	
	GtkWidget *model_runtime_backend_dd = data->model_runtime_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(model_runtime_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *model_parameter_backend_dd = data->model_parameter_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(model_parameter_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *te_runtime_backend_dd = data->te_runtime_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(te_runtime_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *te_parameter_backend_dd = data->te_parameter_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(te_parameter_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *vae_runtime_backend_dd = data->vae_runtime_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(vae_runtime_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *vae_parameter_backend_dd = data->vae_parameter_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(vae_parameter_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *cnet_runtime_backend_dd = data->cnet_runtime_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(cnet_runtime_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *cnet_parameter_backend_dd = data->cnet_parameter_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(cnet_parameter_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *upscaler_runtime_backend_dd = data->upscaler_runtime_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(upscaler_runtime_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *upscaler_parameter_backend_dd = data->upscaler_parameter_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(upscaler_parameter_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *detector_runtime_backend_dd = data->detector_runtime_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(detector_runtime_backend_dd), DEFAULT_BACKEND);
	
	GtkWidget *detector_parameter_backend_dd = data->detector_parameter_backend_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(detector_parameter_backend_dd), DEFAULT_BACKEND);
}

void seed_entry_int_filter(GtkEditable *editable, const char *text, int length, int *position, gpointer user_data)
{
	SeedEntryData *data = user_data;
	long long int *seed_ptr = (long long int *)data->seed;
	
	g_signal_stop_emission_by_name (editable, "insert-text");
	
	const char *entry_text = gtk_editable_get_text(editable);
	
	char full_text[32];
	snprintf(full_text, sizeof(full_text), "%s%s", entry_text, text);
	
	char *endptr;
	errno = 0;
	long long int potential_seed = strtoll(full_text, &endptr, 10);
	
	if (*endptr != '\0' || potential_seed < -1 || errno == ERANGE) {
		show_simple_message(data->win,
			"Seed Input Error",
			"The seed must be numeric only;\nits value must be from -1 to 9223372036854775807.", 1);
		
		g_printerr("Invalid seed input, using default value.\n");
		gtk_editable_set_text(editable, "-1");
		*seed_ptr = DEFAULT_SEED;
	} else {
		g_signal_handlers_block_by_func (editable,(gpointer) seed_entry_int_filter, user_data);
		gtk_editable_insert_text (editable, text, length, position);
		g_signal_handlers_unblock_by_func (editable,(gpointer) seed_entry_int_filter, user_data);
		*seed_ptr = potential_seed;
	}
}

static void on_send_to_trash_finish(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
	SendTrashData *data = user_data;
	GFile *file = G_FILE(source_object);
	GError *error = NULL;

	if (!g_file_trash_finish(file, res, &error)) {
		g_printerr("Error trashing file: %s\n", error->message);
		g_error_free(error);
	} else {
		get_png_files(data->image_files);
		set_current_image_index(data->new_img_path, data->img_index_string, data->image_files, data->current_image_index, -1);

		gtk_label_set_label(GTK_LABEL(data->img_index_label), data->img_index_string->str);

		if (data->image_files->len > 0) {
			gtk_picture_set_filename(GTK_PICTURE(data->image_widget), data->new_img_path);
		} else {
			g_printerr("No images in 'outputs' directory.\n");
			gtk_picture_set_filename(GTK_PICTURE(data->image_widget), DEFAULT_IMG_PATH);
		}
		printf("File moved to trash successfully.\n");
	}
	gtk_widget_set_sensitive(data->to_trash_btn, TRUE);
	g_free(data->new_img_path);
	g_free(data);
}

void send_to_trash(GtkWidget* btn, gpointer user_data)
{
	PreviewImageData *data = user_data;
	gsize img_count = data->image_files->len;
	
	if (img_count > 0) {
		gchar *file_to_trash_path = g_ptr_array_index(data->image_files, *data->current_image_index);

		if (check_file_exists(file_to_trash_path, 0) == 1) {
			gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);

			char *new_img_path = NULL;

			if (img_count > 1) {
				gint new_index = *data->current_image_index + 1 == img_count ?
					*data->current_image_index - 1 :
					*data->current_image_index + 1;

				gchar *new_img_path_ptr = g_ptr_array_index(data->image_files, new_index);
				new_img_path = g_strdup(new_img_path_ptr);
			}

			GFile *file_to_trash = g_file_new_for_path(file_to_trash_path);

			SendTrashData *trash_d = g_new0 (SendTrashData, 1);
			trash_d->new_img_path = new_img_path;
			trash_d->current_image_index = data->current_image_index;
			trash_d->image_files = data->image_files;
			trash_d->img_index_string = data->img_index_string;
			trash_d->hide_img_btn = data->hide_img_btn;
			trash_d->image_widget = data->image_widget;
			trash_d->img_index_label = data->img_index_label;
			trash_d->to_trash_btn = btn;

			g_file_trash_async(file_to_trash, G_PRIORITY_DEFAULT, NULL, (GAsyncReadyCallback)on_send_to_trash_finish, trash_d);
			g_object_unref(file_to_trash);
		} else {
			g_printerr("Error: File not found.\n");
		}
	} else {
		g_printerr("Error: There are no images in the 'output' directory.\n");
	}
}

void set_dropdown_selected_item (GtkWidget* wgt, GParamSpec *pspec, gpointer user_data)
{
	if (GTK_IS_DROP_DOWN(wgt)) {
		DropDownModelsNameData *data = user_data;
		guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(wgt));
		if (data->req_int == 1 &&
		(g_strcmp0(gtk_button_get_label(GTK_BUTTON(data->g_btn)), "Add to Queue") == 0 ||
		g_strcmp0(gtk_button_get_label(GTK_BUTTON(data->g_btn)), "Select a checkpoint first.") == 0)) {
			gboolean none_selected = (sel == 0);
			gtk_button_set_label(GTK_BUTTON(data->g_btn), none_selected ? "Select a checkpoint first." : "Add to Queue");
			gtk_widget_set_sensitive(GTK_WIDGET(data->g_btn), !none_selected);
		}
	}
}

void show_detector_message(GtkWidget *btn, gpointer user_data)
{
	GtkWidget *main_win = (GtkWidget *) user_data;

	GtkWidget *info_win = gtk_window_new ();
	gtk_widget_add_css_class (info_win, "info_box");
	gtk_window_set_transient_for (GTK_WINDOW(info_win), GTK_WINDOW(main_win));
	gtk_window_set_title (GTK_WINDOW(info_win), "About ADetailer");
	gtk_window_set_default_size (GTK_WINDOW(info_win), 500, 100);
	gtk_window_set_resizable (GTK_WINDOW(info_win), TRUE);
	gtk_window_set_deletable (GTK_WINDOW(info_win), TRUE);
	gtk_window_set_decorated (GTK_WINDOW(info_win), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(info_win), TRUE);
	
	GtkWidget *info_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, ZERO_SPACING);
	gtk_widget_set_margin_bottom (info_box, MEDIUM_SPACING);
	gtk_widget_set_margin_end (info_box, MEDIUM_SPACING);
	gtk_widget_set_margin_start (info_box, MEDIUM_SPACING);
	gtk_widget_set_margin_top (info_box, MEDIUM_SPACING);
	gtk_widget_add_css_class(info_box, "info_box");
	
	GtkWidget *message_01_lab = gtk_label_new (ADETAILER_DESC_01);
	gtk_widget_add_css_class(message_01_lab, "info_label");
	gtk_label_set_justify(GTK_LABEL(message_01_lab), GTK_JUSTIFY_CENTER);
	gtk_widget_set_margin_top (message_01_lab, LARGE_SPACING);
	gtk_box_append (GTK_BOX (info_box), message_01_lab);
	
	GtkWidget *adetailer_info_link_btn = gtk_link_button_new_with_label (ADETAILER_INFO_URL, "ADetailer Documentation");
	gtk_widget_set_hexpand (adetailer_info_link_btn, FALSE);
	gtk_widget_set_halign(adetailer_info_link_btn, GTK_ALIGN_CENTER);
	gtk_box_append (GTK_BOX (info_box), adetailer_info_link_btn);
	
	GtkWidget *message_02_lab = gtk_label_new (ADETAILER_DESC_02);
	gtk_widget_add_css_class(message_02_lab, "info_label");
	gtk_label_set_justify(GTK_LABEL(message_02_lab), GTK_JUSTIFY_CENTER);
	gtk_widget_set_margin_top (message_02_lab, LARGE_SPACING);
	gtk_widget_set_margin_bottom (message_02_lab, LARGE_SPACING);
	gtk_box_append (GTK_BOX (info_box), message_02_lab);

	GtkWidget *close_window_btn = gtk_button_new_with_label ("Close");
	gtk_widget_add_css_class(close_window_btn, "custom_btn");
	gtk_widget_set_hexpand(close_window_btn, TRUE);
	g_signal_connect_swapped(close_window_btn, "clicked", G_CALLBACK (gtk_window_destroy), info_win);
	gtk_box_append(GTK_BOX(info_box), close_window_btn);
	
	gtk_window_set_child (GTK_WINDOW(info_win), info_box);
	gtk_window_present (GTK_WINDOW(info_win));
}

void show_info_message (GtkWidget *wgt, GtkWidget *main_win)
{
	GtkWidget *info_win = gtk_window_new ();
	gtk_widget_add_css_class(info_win, "info_box");
	gtk_window_set_transient_for(GTK_WINDOW(info_win), GTK_WINDOW(main_win));
	gtk_window_set_title (GTK_WINDOW(info_win), "About Neural Pixel");
	gtk_window_set_default_size (GTK_WINDOW(info_win), 400, 100);
	gtk_window_set_resizable (GTK_WINDOW(info_win), TRUE);
	gtk_window_set_deletable (GTK_WINDOW(info_win), TRUE);
	gtk_window_set_decorated (GTK_WINDOW(info_win), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(info_win), TRUE);
	
	GtkWidget *info_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, ZERO_SPACING);
	gtk_widget_set_margin_bottom (info_box, MEDIUM_SPACING);
	gtk_widget_set_margin_end (info_box, MEDIUM_SPACING);
	gtk_widget_set_margin_start (info_box, MEDIUM_SPACING);
	gtk_widget_set_margin_top (info_box, MEDIUM_SPACING);
	gtk_widget_add_css_class(info_box, "info_box");
	
	GtkWidget *title_lab = gtk_label_new (APP_NAME_VERSION);
	gtk_widget_add_css_class(title_lab, "info_title_label");
	gtk_box_append (GTK_BOX (info_box), title_lab);
	
	GtkWidget *desc_lab = gtk_label_new (APP_DESC);
	gtk_widget_add_css_class(desc_lab, "info_label");
	gtk_box_append (GTK_BOX (info_box), desc_lab);
	
	GtkWidget *copyright_lab = gtk_label_new (APP_AUTHOR);
	gtk_widget_add_css_class(copyright_lab, "info_label");
	gtk_box_append (GTK_BOX (info_box), copyright_lab);
	
	GtkWidget *np_github_link_btn = gtk_link_button_new_with_label (NP_GITHUB, "Neural Pixel source");
	gtk_widget_set_hexpand (np_github_link_btn, FALSE);
	gtk_widget_set_halign(np_github_link_btn, GTK_ALIGN_CENTER);
	gtk_box_append (GTK_BOX (info_box), np_github_link_btn);
	
	GtkWidget *credits_lab = gtk_label_new ("Credits:");
	gtk_widget_add_css_class(credits_lab, "info_credits_label");
	gtk_box_append (GTK_BOX (info_box), credits_lab);
	
	GtkWidget *sdcpp_lab = gtk_label_new (APP_DESC2);
	gtk_widget_add_css_class(sdcpp_lab, "info_label");
	gtk_label_set_justify(GTK_LABEL(sdcpp_lab), GTK_JUSTIFY_CENTER);
	gtk_box_append (GTK_BOX (info_box), sdcpp_lab);
	
	GtkWidget *sdcpp_github_link_btn = gtk_link_button_new_with_label (SDCPP_GITHUB, "sd.cpp source");
	gtk_widget_set_hexpand (sdcpp_github_link_btn, FALSE);
	gtk_widget_set_halign(sdcpp_github_link_btn, GTK_ALIGN_CENTER);
	gtk_box_append (GTK_BOX (info_box), sdcpp_github_link_btn);

	GtkWidget *close_window_btn = gtk_button_new_with_label ("Close");
	gtk_widget_add_css_class(close_window_btn, "custom_btn");
	gtk_widget_set_hexpand(close_window_btn, TRUE);
	g_signal_connect_swapped(close_window_btn, "clicked", G_CALLBACK (gtk_window_destroy), info_win);
	gtk_box_append(GTK_BOX(info_box), close_window_btn);
	
	gtk_window_set_child (GTK_WINDOW(info_win), info_box);
	gtk_window_present (GTK_WINDOW(info_win));
}

void show_no_models_message(GtkWidget *main_win)
{
	GtkWidget *info_win = gtk_window_new ();
	gtk_widget_add_css_class (info_win, "info_box");
	gtk_window_set_transient_for (GTK_WINDOW(info_win), GTK_WINDOW(main_win));
	gtk_window_set_title (GTK_WINDOW(info_win), "No Models Found");
	gtk_window_set_default_size (GTK_WINDOW(info_win), 500, 100);
	gtk_window_set_resizable (GTK_WINDOW(info_win), TRUE);
	gtk_window_set_deletable (GTK_WINDOW(info_win), TRUE);
	gtk_window_set_decorated (GTK_WINDOW(info_win), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(info_win), TRUE);
	
	GtkWidget *info_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, ZERO_SPACING);
	gtk_widget_set_margin_bottom (info_box, MEDIUM_SPACING);
	gtk_widget_set_margin_end (info_box, MEDIUM_SPACING);
	gtk_widget_set_margin_start (info_box, MEDIUM_SPACING);
	gtk_widget_set_margin_top (info_box, MEDIUM_SPACING);
	gtk_widget_add_css_class(info_box, "info_box");
	
	GtkWidget *message_01_lab = gtk_label_new (APP_MSG_01);
	gtk_widget_add_css_class(message_01_lab, "info_label");
	gtk_label_set_justify(GTK_LABEL(message_01_lab), GTK_JUSTIFY_CENTER);
	gtk_widget_set_margin_top (message_01_lab, LARGE_SPACING);
	gtk_box_append (GTK_BOX (info_box), message_01_lab);

	GtkWidget *links_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (links_box), TRUE);
	gtk_box_append (GTK_BOX (info_box), links_box);

	GtkWidget *civarchive_link_btn = gtk_link_button_new_with_label (CIVARCHIVE_URL, "CivArchive");
	gtk_widget_set_hexpand (civarchive_link_btn, FALSE);
	gtk_widget_set_halign(civarchive_link_btn, GTK_ALIGN_END);
	gtk_box_append (GTK_BOX (links_box), civarchive_link_btn);

	GtkWidget *civitai_link_btn = gtk_link_button_new_with_label (CIVITAI_URL, "CivitAI");
	gtk_widget_set_hexpand (civitai_link_btn, FALSE);
	gtk_widget_set_halign(civitai_link_btn, GTK_ALIGN_CENTER);
	gtk_box_append (GTK_BOX (links_box), civitai_link_btn);
	
	GtkWidget *huggingface_link_btn = gtk_link_button_new_with_label (HUGGINGFACE_URL, "Hugging Face");
	gtk_widget_set_hexpand (huggingface_link_btn, FALSE);
	gtk_widget_set_halign(huggingface_link_btn, GTK_ALIGN_START);
	gtk_box_append (GTK_BOX (links_box), huggingface_link_btn);
	
	GtkWidget *message_02_lab = gtk_label_new (APP_MSG_02);
	gtk_widget_add_css_class(message_02_lab, "info_label");
	gtk_label_set_justify(GTK_LABEL(message_02_lab), GTK_JUSTIFY_CENTER);
	gtk_widget_set_margin_top (message_02_lab, LARGE_SPACING);
	gtk_widget_set_margin_bottom (message_02_lab, LARGE_SPACING);
	gtk_box_append (GTK_BOX (info_box), message_02_lab);

	GtkWidget *close_window_btn = gtk_button_new_with_label ("Close");
	gtk_widget_add_css_class(close_window_btn, "custom_btn");
	gtk_widget_set_hexpand(close_window_btn, TRUE);
	g_signal_connect_swapped(close_window_btn, "clicked", G_CALLBACK (gtk_window_destroy), info_win);
	gtk_box_append(GTK_BOX(info_box), close_window_btn);
	
	gtk_window_set_child (GTK_WINDOW(info_win), info_box);
	gtk_window_present (GTK_WINDOW(info_win));
}

void show_simple_message (GtkWidget *win, char *msg_title_text, char *msg_text, int is_error)
{
	#if GTK_CHECK_VERSION(4, 10, 0)
		GtkAlertDialog *simple_dialog = gtk_alert_dialog_new ("%s", msg_title_text);
		gtk_alert_dialog_set_detail (simple_dialog, msg_text);
		gtk_alert_dialog_show (simple_dialog, GTK_WINDOW(win));
		g_object_unref(simple_dialog);
	#else
		GtkWidget *simple_dialog = gtk_message_dialog_new(
		GTK_WINDOW(win),
		GTK_DIALOG_MODAL,
		is_error ? GTK_MESSAGE_ERROR : GTK_MESSAGE_INFO,
		GTK_BUTTONS_CLOSE,
		"%s", msg_title_text
		);

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(simple_dialog), "%s", msg_text);
		g_signal_connect (simple_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
		gtk_widget_show(simple_dialog);
	#endif
}

void stop_spinbutton_scroll(GtkWidget *btn, GtkWidget *properties_scrollable)
{
	GtkEventController *sc;
	sc = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_BOTH_AXES);

	gtk_event_controller_set_propagation_phase(sc, GTK_PHASE_CAPTURE);

	g_signal_connect(sc, "scroll", G_CALLBACK (steal_scroll_cb), GTK_SCROLLED_WINDOW(properties_scrollable));
	gtk_widget_add_controller (btn, sc);
}

void random_seed_btn_toggle(GtkWidget *entry_wgt, GtkEntryIconPosition position, GdkEvent *event, gpointer user_data)
{
	if (position == GTK_ENTRY_ICON_SECONDARY) {
		char seed_str[LONGLONG_STR_SIZE];
		snprintf(seed_str, sizeof(seed_str), "%lld", DEFAULT_SEED);
	        gtk_editable_set_text(GTK_EDITABLE(entry_wgt), seed_str);
	}
}

void hide_img_btn_cb (GtkButton *btn, gpointer user_data)
{
	PreviewImageData *data = user_data;
	GtkPicture *img = GTK_PICTURE(data->image_widget);
	if (g_strcmp0 (gtk_button_get_icon_name(btn), "view-reveal-symbolic") == 0) {
		gtk_button_set_icon_name (btn, "view-conceal-symbolic");
		gtk_picture_set_filename(img, EMPTY_IMG_PATH);
	} else {
		gtk_button_set_icon_name (btn, "view-reveal-symbolic");
		if (data->image_files->len > 0) {
			gtk_picture_set_filename(img, g_ptr_array_index(data->image_files, *data->current_image_index));
		} else {
			gtk_picture_set_filename(img, DEFAULT_IMG_PATH);
		}
	}
}

void toggle_img2img_mode(GtkWidget *btn, gpointer user_data)
{
	gboolean pressed_btn_state = gtk_check_button_get_active(GTK_CHECK_BUTTON(btn));
	gboolean alt_btn_state = gtk_check_button_get_active(GTK_CHECK_BUTTON(user_data));
	
	if (pressed_btn_state == TRUE && alt_btn_state == TRUE) {
		gtk_check_button_set_active(GTK_CHECK_BUTTON(user_data), FALSE);
	}
}

void toggle_extra_options(GtkCheckButton *btn, gpointer user_data)
{
	int *i = (int *)user_data;
	*i = gtk_check_button_get_active(btn) == TRUE ? 1 : 0;
}

void toggle_fa_options(GtkToggleButton *btn, gpointer user_data)
{
	int *i = (int *)user_data;
	
	if (gtk_toggle_button_get_active(btn) == FALSE) return;
	
	if (g_strcmp0(gtk_button_get_label(GTK_BUTTON(btn)), "Full") == 0) {
		*i = 2;
	} else if (g_strcmp0(gtk_button_get_label(GTK_BUTTON(btn)), "Diffusion") == 0) {
		*i = 1;
	} else {
		*i = 0;
	}
}
