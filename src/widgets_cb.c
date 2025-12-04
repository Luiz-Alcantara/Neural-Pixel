#include <gtk/gtk.h>
#include <ctype.h>
#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "constants.h"
#include "file_utils.h"
#include "str_utils.h"
#include "global.h"
#include "structs.h"

void show_error_message (GtkWidget *win, char *err_title_text, char *err_text)
{
	#if GTK_CHECK_VERSION(4, 10, 0)
		GtkAlertDialog *error_dialog = gtk_alert_dialog_new (err_title_text);
		gtk_alert_dialog_set_detail (error_dialog, err_text);
		gtk_alert_dialog_show (error_dialog, GTK_WINDOW(win));
		g_object_unref(error_dialog);
	#else
		GtkWidget *error_dialog = gtk_message_dialog_new(
		GTK_WINDOW(win),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_CLOSE,
		err_title_text
		);

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(error_dialog), err_text);
		g_signal_connect (error_dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
		gtk_widget_show(error_dialog);
	#endif
}

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
	
	if (data->checkpoint_string != NULL) {
		g_string_free(data->checkpoint_string, TRUE);
		data->checkpoint_string = NULL;
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
	
	if (data->t5xxl_string != NULL) {
		g_string_free(data->t5xxl_string, TRUE);
		data->t5xxl_string = NULL;
	}
	
	if (data->img2img_file_path != NULL) {
		g_string_free(data->img2img_file_path, TRUE);
		data->img2img_file_path = NULL;
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

void clear_img2img_btn_cb (GtkWindow *wgt, gpointer user_data)
{
	LoadImg2ImgData *data = user_data;
	GString *gstr = data->img2img_file_path;
	g_string_assign(gstr, "None");
	GtkPicture *prev_img = GTK_PICTURE(data->image_wgt);
	gtk_picture_set_filename(prev_img, DEFAULT_IMG_PATH);
}

gboolean close_app_callback (GtkWindow *win, gpointer user_data)
{
	app_start_data_free(user_data);
	gtk_window_destroy (win);
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

void kill_stable_diffusion_process (GtkButton *btn, gpointer user_data)
{
	gtk_widget_set_sensitive(GTK_WIDGET(btn), FALSE);

	#ifdef _WIN32
		HANDLE hp = OpenProcess(PROCESS_TERMINATE, FALSE, (DWORD)globalSDPID);
		if (hp == NULL) {
			fprintf(stderr, "Failed to open process %d. Error: %lu\n", globalSDPID, GetLastError());
		} else {
			if (TerminateProcess(hp, 1)) {
				printf("Process %d terminated successfully.\n", globalSDPID);
				globalSDPID = 0;
			} else {
				fprintf(stderr, "Failed to terminate process %d. Error: %lu\n", globalSDPID, GetLastError());
			}
			CloseHandle(hp);
		}
	#else
		if (kill(globalSDPID, SIGKILL) == 0) {
			printf("Process %d killed successfully.\n", globalSDPID);
			globalSDPID = 0;
		} else {
			fprintf(stderr, "Error killing process.\n");
		}
	#endif
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

void on_dropdown_destroy (GtkWidget* wgt, gpointer user_data)
{
	const char** di = (const char**)user_data;
	array_strings_free(di);
}

void on_generate_btn_destroy (GtkWidget* wgt, gpointer user_data)
{
	GenerationData *data = user_data;
	if (data == NULL) return;
	g_free(data);
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
	dropdown_items_update(TEXT_ENCODERS_PATH, GTK_WIDGET(data->t5xxl_dd), data->app);
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
	
	GtkWidget *t5xxl_dd = data->t5xxl_dd;
	gtk_drop_down_set_selected(GTK_DROP_DOWN(t5xxl_dd), DEFAULT_MODELS);

	GtkWidget *cfg_spin = data->cfg_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), DEFAULT_CFG);

	GtkWidget *denoise_spin = data->denoise_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(denoise_spin), DEFAULT_DENOISE);

	GtkWidget *seed_entry = data->seed_entry;
	char seed_str[LONGLONG_STR_SIZE];
	snprintf(seed_str, sizeof(seed_str), "%lld", DEFAULT_SEED);
	gtk_editable_set_text(GTK_EDITABLE(seed_entry), seed_str);

	GtkWidget *upscale_spin = data->upscale_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(upscale_spin), DEFAULT_RP_UPSCALE);

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
	
	GtkWidget *steps_spin = data->steps_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(steps_spin), DEFAULT_N_STEPS);

	GtkWidget *batch_count_spin = data->batch_count_spin;
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(batch_count_spin), DEFAULT_BATCH_COUNT);
	
	GtkWidget *sd_based_check = data->sd_based_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(sd_based_check), ENABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *cpu_check = data->cpu_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(cpu_check), DISABLED_OPT == 1 ? TRUE : FALSE);

	GtkWidget *tiling_check = data->tiling_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(tiling_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *ram_offload_check = data->ram_offload_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(ram_offload_check), DISABLED_OPT == 1 ? TRUE : FALSE);

	GtkWidget *clip_check = data->clip_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(clip_check), DISABLED_OPT == 1 ? TRUE : FALSE);

	GtkWidget *cnet_check = data->cnet_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(cnet_check), DISABLED_OPT == 1 ? TRUE : FALSE);

	GtkWidget *vae_check = data->vae_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(vae_check), DISABLED_OPT == 1 ? TRUE : FALSE);

	GtkWidget *flash_check = data->flash_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(flash_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *taesd_check = data->taesd_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(taesd_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *llm_check = data->llm_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(llm_check), DISABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *update_cache_check = data->update_cache_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(update_cache_check), ENABLED_OPT == 1 ? TRUE : FALSE);
	
	GtkWidget *verbose_check = data->verbose_check;
	gtk_check_button_set_active(GTK_CHECK_BUTTON(verbose_check), DISABLED_OPT == 1 ? TRUE : FALSE);
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
		show_error_message(data->win,
			"Seed Input Error",
			"The seed must be numeric only;\nits value must be from -1 to 9223372036854775807.");
		
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

void set_dropdown_selected_const_item(GtkWidget* wgt, GParamSpec *pspec, int *i1)
{
	GtkDropDown *dd = GTK_DROP_DOWN(wgt);
	guint s = gtk_drop_down_get_selected(dd);
	*i1 = (int)s;
}

void set_dropdown_selected_item (GtkWidget* wgt, GParamSpec *pspec, gpointer user_data)
{
	if (GTK_IS_DROP_DOWN(wgt)) {
		DropDownModelsNameData *data = user_data;
		GString *var_string = data->dd_item_str;
		int is_req = data->req_int;
		GtkDropDown *dd = GTK_DROP_DOWN(wgt);
		guint s = gtk_drop_down_get_selected(dd);
		GtkStringObject *dd_string_obj = gtk_drop_down_get_selected_item(dd);
		const char *dd_string = gtk_string_object_get_string(dd_string_obj);
		g_string_assign(var_string, dd_string);
		if (is_req == 1) {
			GtkWidget *gen_btn = data->g_btn;
			if (g_strcmp0(var_string->str, "None") == 0) {
				gtk_button_set_label (GTK_BUTTON(gen_btn), "Select a checkpoint first.");
				gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), FALSE);
			} else {
				gtk_button_set_label (GTK_BUTTON(gen_btn), "Generate");
				gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), TRUE);
			}
		}
	}
}

void set_spin_value_to_var (GtkWidget *w, double *v)
{
	double value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(w));
	*v = value;
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
	const char *i;
	i = gtk_button_get_icon_name (btn);
	if (g_strcmp0 (i, "view-reveal-symbolic") == 0) {
		gtk_button_set_icon_name (btn, "view-conceal-symbolic");
		gtk_picture_set_filename(img, EMPTY_IMG_PATH);
	} else {
		gtk_button_set_icon_name (btn, "view-reveal-symbolic");
	}
}

void toggle_extra_options (GtkCheckButton *btn, gpointer user_data)
{
	int *i = (int *)user_data;
	*i = gtk_check_button_get_active(btn) == TRUE ? 1 : 0;
}
