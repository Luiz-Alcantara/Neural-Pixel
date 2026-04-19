#include <gtk/gtk.h>
#include <gio/gio.h>
#include "constants.h"
#include "cmd_generator.h"
#include "file_utils.h"
#include "structs.h"
#include "str_utils.h"
#include "widgets_cb.h"

static void handle_stderr(GObject* stream_obj, GAsyncResult* res, gpointer user_data)
{
	SDProcessErrorData *data = user_data;
	if (*data->sdpid == 0) {
		if (data->err_pipe_stream) {
			g_input_stream_close(G_INPUT_STREAM(data->err_pipe_stream), NULL, NULL);
			g_object_unref(data->err_pipe_stream);
			data->err_pipe_stream = NULL;
		}
		g_free(data);
		return;
	}
	
	GError *error = NULL;
	char *err_string = g_data_input_stream_read_line_finish(G_DATA_INPUT_STREAM(stream_obj), res, NULL, &error);
	
	if (error) {
		g_printerr("Error reading line: %s\n", error->message);
		g_error_free(error);
	} else if (err_string) {
		if (data->verbose_bool) {
			printf("%s\n", err_string);
		}
		
		int n_error;
		int n_error2;
		char file_name[256];
		
		if (sscanf(err_string, "[ERROR] stable-diffusion.cpp:%i  - init model loader from file failed: '%255[^']'",
		&n_error, file_name) == 2) {
			char error_dialog_text[16 + strlen(file_name)];
			strcpy(error_dialog_text, "Error loading: ");
			strcat(error_dialog_text, file_name);

			show_error_message(data->win, "Error loading file", error_dialog_text);
		}
		
		if (sscanf(err_string, "[ERROR] stable-diffusion.cpp:%i  - load tensors from model loader failed",
		&n_error2) == 1) {
			show_error_message(data->win, "Error loading model", "Error loading tensors from model");
		}
		
		g_free(err_string);
		g_data_input_stream_read_line_async(data->err_pipe_stream, G_PRIORITY_DEFAULT, NULL, handle_stderr, user_data);
	} else {
		if (data->err_pipe_stream) {
			g_input_stream_close(G_INPUT_STREAM(data->err_pipe_stream), NULL, NULL);
			g_object_unref(data->err_pipe_stream);
			data->err_pipe_stream = NULL;
		}
		g_free(data);
		return;
	}
}

static void show_progress(GObject* stream_obj, GAsyncResult* res, gpointer user_data)
{
	SDProcessOutputData *data = user_data;
	
	GError *error = NULL;
	gssize bytes_read = g_input_stream_read_finish(G_INPUT_STREAM(stream_obj), res, &error);
	
	if (error) {
		g_printerr("Error reading line: %s\n", error->message);
		g_error_free(error);
	} else if (bytes_read > 0) {
		g_string_append_len(data->stdout_string, data->read_buffer, bytes_read);

		while (TRUE) {
			char *clear_escape = strstr(data->stdout_string->str, "\033[K");
			char *return_escape = strchr(data->stdout_string->str, '\r');
			char *newline_escape = strchr(data->stdout_string->str, '\n');
			
			char *progress_escape = clear_escape ? clear_escape : return_escape;
			size_t progress_escape_len = clear_escape ? 3 : 1;

			if (progress_escape) {
				size_t end_index = (progress_escape - data->stdout_string->str) + progress_escape_len;
				char *raw_string = progress_escape_len == 3 ? g_strndup(data->stdout_string->str, end_index) : g_strdup(progress_escape + 1);
				
				char *final_string = progress_escape_len == 3 ? strrchr(raw_string, '\r') : raw_string;
				
				if (final_string) {
					if (data->verbose_bool) {
						printf("%s\n", final_string);
					}
					
					const char *last_pipe = strrchr(final_string, '|');
					if (last_pipe) {
						int step, steps;
						float time_or_speed;
						char unit_part1[10], unit_part2[10];

						if (sscanf(last_pipe + 1, " %i/%i - %f%9[^/]/%9[ -~]", &step, &steps, &time_or_speed, unit_part1, unit_part2) == 5) {
							int percentage = (int)(((float)step / steps) * 100 + 0.5);
							char progress_label[64];
							
							int written = 0;
							
							if (data->is_img2img_encoding) {
								written = snprintf(progress_label,
									sizeof(progress_label), "Encoding... %d%% (1/1) at %.2f%s/%s",
									percentage, time_or_speed, unit_part1, unit_part2);
							} else if (data->is_generating_latent) {
								written = snprintf(progress_label,
									sizeof(progress_label), "Generating... %d%% (%d/%d) at %.2f%s/%s",
									percentage, data->n_current_image, data->n_total_images, time_or_speed, unit_part1, unit_part2);
							} else if (data->is_decoding_latents) {
								written = snprintf(progress_label,
									sizeof(progress_label), "Decoding... %d%% (%d/%d) at %.2f%s/%s",
									percentage, data->n_current_latent, data->n_total_images, time_or_speed, unit_part1, unit_part2);
							} else if (data->is_upscaling) {
								written = snprintf(progress_label,
									sizeof(progress_label), "Upscaling... %d%% (%d/%d) at %.2f%s/%s",
									percentage, data->n_current_upscale, data->n_total_images, time_or_speed, unit_part1, unit_part2);
							}
							
							if (written > 0) {
								gtk_button_set_label(GTK_BUTTON(data->button), progress_label);
							} else {
								gtk_button_set_label(GTK_BUTTON(data->button), "Loading...");
								g_printerr("Error: Could not fetch progress from line: %s\n", final_string);
							}
						}
					}
				}

				g_free(raw_string);
				g_string_erase(data->stdout_string, 0, end_index);
				continue;
			}

			if (newline_escape) {
				size_t end_index = (newline_escape - data->stdout_string->str);
				char *line = g_strndup(data->stdout_string->str, end_index);
				
				// Avoid "unused lora tensor" span
				if (data->verbose_bool && strstr(line, "[WARN ] lora.hpp:") == NULL) {
					printf("%s\n", line);
				}
				
				if (strstr(line, "target") != NULL) {
					int x;
					int n_img2img_tiles;

					if (sscanf(line,
					"[INFO ] stable-diffusion.cpp:%i - target t_enc is %i steps",
					&x, &n_img2img_tiles) == 2) {
						gtk_button_set_label(GTK_BUTTON(data->button), "Encoding...");
						data->is_img2img_encoding = 1;
					}
				} else if (strstr(line, "encode_first_stage completed, taking") != NULL && data->is_img2img_encoding) {
					data->is_img2img_encoding = 0;
					data->img2img_enc_completed = 1;
					gtk_button_set_label(GTK_BUTTON(data->button), "Generating...");
				} else if (strstr(line, "generating image:") != NULL) {
					int x;
					int n_current_image;
					int n_total_images;
					long long int img_seed;

					if (sscanf(line,
					"[INFO ] stable-diffusion.cpp:%i - generating image: %i/%i - seed %lld",
					&x, &n_current_image, &n_total_images, &img_seed) == 4) {
						gtk_button_set_label(GTK_BUTTON(data->button), "Generating...");

						data->is_generating_latent = 1;
						data->n_current_image = n_current_image;
						data->n_total_images = n_total_images;
					} else {
						g_printerr("Error: Could not parse batch size from line: %s\n", line);
					}
				} else if (strstr(line, "latent images completed, taking") != NULL && data->n_current_image == data->n_total_images) {
					data->is_generating_latent = 0;
					data->gen_latent_completed = 1;
					gtk_button_set_label(GTK_BUTTON(data->button), "Decoding latent(s)...");
				} else if (strstr(line, "decoding") != NULL) {
					int x;
					int n_dec_latents;

					if (sscanf(line,
					"[INFO ] stable-diffusion.cpp:%i - decoding %i latents",
					&x, &n_dec_latents) == 2) {
						data->is_decoding_latents = 1;
						gtk_button_set_label(GTK_BUTTON(data->button), "Decoding...");
						data->n_current_latent++;
					}
				} else if (strstr(line, "decoded, taking") != NULL && data->is_decoding_latents) {
					int x;
					int y;
					double z;
					
					if (sscanf(line,
					"[INFO ] stable-diffusion.cpp:%i - latent %i decoded, taking %lfs",
					&x, &y, &z) == 3) {
						if (y < data->n_total_images) data->n_current_latent = y + 1;
					}
				} else if (strstr(line, "generate_image completed in") != NULL && data->is_decoding_latents) {
					data->is_decoding_latents = 0;
					data->dec_latents_completed = 1;
					
					int x;
					double gen_seconds;
					
					if (sscanf(line,
					"[INFO ] stable-diffusion.cpp:%i - generate_image completed in %lfs",
					&x, &gen_seconds) == 2) {
						*(data->total_time) = (int)gen_seconds;
					} else {
						g_printerr("Error: Could not extract time from string: \"%s\"\n", line);
					}
				} else if (strstr(line, "- upscaling from") != NULL) {
					data->is_upscaling = 1;
					int x;
					int ow;
					int oh;
					int nw;
					int nh;

					if (sscanf(line,
					"[INFO ] upscaler.cpp:%i   - upscaling from (%i x %i) to (%i x %i)",
					&x, &ow, &oh, &nw, &nh) == 5 ) {
						gtk_button_set_label(GTK_BUTTON(data->button), "Upscaling...");
						data->n_current_upscale++;
					}
				} else if (strstr(line, "- input_image_tensor upscaled, taking") != NULL) {
					int x;
					double upscale_seconds;
					
					if (sscanf(line,
					"[INFO ] upscaler.cpp:%i   - input_image_tensor upscaled, taking %lfs",
					&x, &upscale_seconds) == 2) {
						*(data->total_time) += (int)upscale_seconds;
					} else {
						g_printerr("Error: Could not extract time from string: \"%s\"\n", line);
					}
				}

				g_free(line);
				g_string_erase(data->stdout_string, 0, end_index + 1);
				continue;
			}

			break;
		}
		g_input_stream_read_async(G_INPUT_STREAM(stream_obj), data->read_buffer, sizeof(data->read_buffer), 
		G_PRIORITY_DEFAULT, NULL, 
		show_progress, user_data);
		
	} else {
		g_input_stream_close(G_INPUT_STREAM(stream_obj), NULL, NULL);
		data->out_pipe_stream = NULL;
		if (data->stdout_string != NULL) {
			g_string_free(data->stdout_string, TRUE);
			data->stdout_string = NULL;
		}
		g_free(data);
		return;
	}
	if (*data->sdpid == 0) {
		return;
	}
}

static void on_subprocess_end(GObject* source_object, GAsyncResult* res, gpointer user_data)
{
	GenerationSnapshotData *data = user_data;
	*data->sdpid = 0;
	const char *icon_n = gtk_button_get_icon_name (GTK_BUTTON(data->preview_image_toggle_visibility_btn));
	
	char *result_img_path;
	
	if (data->batch_count_value > 1) {
		size_t result_img_len = strlen(data->output_path);
		char *bn = g_strndup(data->output_path, result_img_len - 4);
		result_img_path = g_strdup_printf("%s_%d.png", bn, data->batch_count_value - 1);
		g_free(bn);
	} else {
		result_img_path = g_strdup(data->output_path);
	}
	
	if (check_file_exists(result_img_path, 0) == 1) {
		get_png_files(data->preview_image_files);
		
		set_current_image_index(result_img_path + 2,
			data->preview_label_string, data->preview_image_files,
			data->preview_image_index, data->total_time);
		
		if (data->preview_image_files->len > 0) {
			if (g_strcmp0 (icon_n, "view-conceal-symbolic") != 0) {
				gtk_picture_set_filename(GTK_PICTURE(data->preview_image_widget), result_img_path);
			} else {
				gtk_picture_set_filename(GTK_PICTURE(data->preview_image_widget), EMPTY_IMG_PATH);
			}
		} else {
			g_printerr("No images in 'outputs' directory.\n");
			gtk_picture_set_filename(GTK_PICTURE(data->preview_image_widget), DEFAULT_IMG_PATH);
		}
		
		gtk_label_set_label(GTK_LABEL(data->preview_label), data->preview_label_string->str);
	} else {
		g_printerr(
			"Error loading image: The file '%s' is missing, corrupted, or invalid.\n",
			result_img_path
		);
		gtk_picture_set_filename(GTK_PICTURE(data->preview_image_widget), DEFAULT_IMG_PATH);
	}
	
	gtk_button_set_label(GTK_BUTTON(data->gen_btn), "Generate");
	gtk_widget_set_sensitive(GTK_WIDGET(data->gen_btn), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(data->halt_btn), FALSE);
	g_ptr_array_set_size(data->sd_cmd_array, 0);
	g_free(result_img_path);
	g_free(data->output_path);
	g_free(data->img2img_file_path);
	g_free(data->positive_prompt);
	g_free(data->negative_prompt);
	g_free(data->checkpoint_filename);
	g_free(data->vae_filename);
	g_free(data->cnet_filename);
	g_free(data->upscaler_filename);
	g_free(data->clip_l_filename);
	g_free(data->clip_g_filename);
	g_free(data->text_enc_filename);
	g_free(data);
	g_object_unref(source_object);
}

static void start_reading_error(gpointer user_data)
{
	SDProcessErrorData *data = user_data;
	GDataInputStream *data_err_stream = data->err_pipe_stream;
	g_data_input_stream_read_line_async(
		data_err_stream,
		G_PRIORITY_DEFAULT,
		NULL,
		handle_stderr,
		user_data
	);
}

static void start_reading_output(gpointer user_data)
{
	SDProcessOutputData *data = user_data;
	GInputStream *stdout_stream = data->out_pipe_stream;
	g_input_stream_read_async(
		stdout_stream,
		data->read_buffer,
		sizeof(data->read_buffer),
		G_PRIORITY_DEFAULT,
		NULL,
		show_progress,
		user_data
	);
}

static void start_generation(gpointer user_data)
{
	GenerationSnapshotData *data = user_data;
	if (data == NULL) {
		g_printerr("Error: Data used to generate the images is corrupted.\n");
		
		show_error_message(data->win,
		"Error reading generation data",
		"Error reading generation data;\ntry restarting the app or deleting the \".cache\" folder.");
		
		return;
	}
	if (g_strcmp0 (gtk_button_get_label (GTK_BUTTON(data->gen_btn)), "Generate") == 0) {
		gtk_button_set_label (GTK_BUTTON(data->gen_btn), "Loading Files...");
		gtk_widget_set_sensitive(GTK_WIDGET(data->gen_btn), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(data->halt_btn), TRUE);
	}

	gen_sd_string(data);

	GSubprocessFlags sd_flags = G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE;
	GError *error = NULL;

	GSubprocess* sd_process = g_subprocess_newv((const gchar * const *)data->sd_cmd_array->pdata, sd_flags, &error);

	if (sd_process == NULL) {
		g_print("Error spawning process: %s\n", error->message);
		g_clear_error(&error);
		
		show_error_message(data->win,
		"Error spawning process",
		"Error spawning the sd.cpp process;\nlook at the terminal log for details.");
		
		g_ptr_array_set_size(data->sd_cmd_array, 0);
		
		gtk_button_set_label (GTK_BUTTON(data->gen_btn), "Generate");
		gtk_widget_set_sensitive(data->gen_btn, TRUE);
		gtk_widget_set_sensitive(data->halt_btn, FALSE);
		
		return;
	} else {
		data->total_time = 0;
		const gchar* sd_pid = g_subprocess_get_identifier(sd_process);
		char *endptr;
		int npid = strtol(sd_pid, &endptr, 10);

		if (*endptr == '\0') {
			*data->sdpid = npid;
		} else {
			*data->sdpid = 0;
		}
		
		printf("Binary launched in subprocess: %d\n", *data->sdpid);

		GInputStream *stdout_stream = g_subprocess_get_stdout_pipe(sd_process);
		GInputStream *stderr_stream = g_subprocess_get_stderr_pipe(sd_process);

		GDataInputStream *data_err_stream = g_data_input_stream_new(stderr_stream);
		
		g_data_input_stream_set_newline_type (data_err_stream, G_DATA_STREAM_NEWLINE_TYPE_LF);
		
		SDProcessOutputData *output_d = g_new0 (SDProcessOutputData, 1);
		output_d->is_img2img_encoding = 0;
		output_d->img2img_enc_completed = 0;
		output_d->is_generating_latent = 0;
		output_d->n_current_image = 0;
		output_d->n_total_images = 0;
		output_d->gen_latent_completed = 0;
		output_d->is_decoding_latents = 0;
		output_d->n_current_latent = 0;
		output_d->dec_latents_completed = 0;
		output_d->is_upscaling = 0;
		output_d->n_current_upscale = 0;
		output_d->verbose_bool = data->verbose_enabled;
		output_d->total_time = &data->total_time;
		output_d->button = data->gen_btn;
		output_d->sdpid = data->sdpid;
		output_d->out_pipe_stream = stdout_stream;
		output_d->stdout_string = g_string_new("");
		
		SDProcessErrorData *error_d = g_new0 (SDProcessErrorData, 1);
		error_d->verbose_bool = data->verbose_enabled;
		error_d->win = data->win;
		error_d->sdpid = data->sdpid;
		error_d->err_pipe_stream = data_err_stream;

		GCancellable* cnlb = g_cancellable_new ();

		start_reading_output(output_d);
		start_reading_error(error_d);
		g_subprocess_wait_async(sd_process, cnlb, on_subprocess_end, user_data);
		g_object_unref(cnlb);
	}
}

void prepare_gen_data(GtkWidget *gen_btn, gpointer user_data)
{
	GenerationData *data = user_data;
	AppStartData *app_data = data->app_data;
	
	GenerationSnapshotData *snapshot_data = g_new0 (GenerationSnapshotData, 1);
	
	snapshot_data->img2img_file_path = g_strdup(app_data->img2img_file_path->str);
	snapshot_data->kontext_enabled = app_data->kontext_bool;
	
	GtkTextBuffer *pos_tb = data->pos_p;
	GtkTextIter psi;
	GtkTextIter pei;
	gtk_text_buffer_get_bounds (pos_tb, &psi, &pei);
	snapshot_data->positive_prompt = gtk_text_buffer_get_text(pos_tb, &psi, &pei, FALSE);

	GtkTextBuffer *neg_tb = data->neg_p;
	GtkTextIter nsi;
	GtkTextIter nei;
	gtk_text_buffer_get_bounds (neg_tb, &nsi, &nei);
	snapshot_data->negative_prompt = gtk_text_buffer_get_text(neg_tb, &nsi, &nei, FALSE);
	
	snapshot_data->checkpoint_filename = g_strdup(app_data->checkpoint_string->str);
	
	snapshot_data->sd_based_enabled = app_data->sd_based_bool;
	
	snapshot_data->vae_filename = g_strdup(app_data->vae_string->str);
	snapshot_data->cnet_filename = g_strdup(app_data->cnet_string->str);
	snapshot_data->upscaler_filename = g_strdup(app_data->upscaler_string->str);
	snapshot_data->clip_l_filename = g_strdup(app_data->clip_l_string->str);
	snapshot_data->clip_g_filename  = g_strdup(app_data->clip_g_string->str);
	snapshot_data->text_enc_filename  = g_strdup(app_data->text_enc_string->str);
	
	snapshot_data->llm_mode_enabled = app_data->llm_bool;
	
	snapshot_data->width_index = app_data->w_index;
	snapshot_data->height_index = app_data->h_index;
	snapshot_data->step_count_value = (int)app_data->steps_value;
	snapshot_data->batch_count_value = (int)app_data->batch_count_value;
	snapshot_data->sampler_index = app_data->sampler_index;
	snapshot_data->scheduler_index = app_data->scheduler_index;
	snapshot_data->cfg_scale_value = app_data->cfg_value;
	snapshot_data->denoise_strength_value = app_data->denoise_value;
	snapshot_data->seed_value = app_data->seed_value;
	snapshot_data->clip_skip_value = (int)app_data->clip_skip_value;
	snapshot_data->upscale_passes_value = (int)app_data->up_repeat_value;
	snapshot_data->cnet_strength_value = app_data->cnet_value;
	snapshot_data->cpu_mode_enabled = app_data->cpu_bool;
	snapshot_data->vae_tiling_enabled = app_data->vt_bool;
	snapshot_data->ram_offload_enabled = app_data->ram_offload_bool;
	snapshot_data->keep_clip_cpu_enabled = app_data->k_clip_bool;
	snapshot_data->keep_cnet_cpu_enabled = app_data->k_cnet_bool;
	snapshot_data->keep_vae_cpu_enabled = app_data->k_vae_bool;
	snapshot_data->flash_att_enabled = app_data->fa_bool;
	snapshot_data->taesd_enabled = app_data->taesd_bool;
	snapshot_data->update_cache_enabled = app_data->update_cache_bool;
	snapshot_data->verbose_enabled = app_data->verbose_bool;
	snapshot_data->total_time = 0;
	snapshot_data->sdpid = &app_data->sdpid;
	
	char *out_timestamp = get_time_str();
	
	if (out_timestamp) {
		#ifdef G_OS_WIN32
			snapshot_data->output_path = g_strdup_printf(".\\outputs\\IMG_%s.png", out_timestamp);
		#else
			snapshot_data->output_path = g_strdup_printf("./outputs/IMG_%s.png", out_timestamp);
		#endif
		
		free(out_timestamp);
	} else {
		g_printerr("Failed to acquire timestamp. Using default value, which may overwrite the generated image. Try restarting the app or deleting the '.cache' folder. If the issue persists, please report it.\n");
		#ifdef G_OS_WIN32
			snapshot_data->output_path = g_strdup(".\\outputs\\IMG_00001.png");
		#else
			snapshot_data->output_path = g_strdup("./outputs/IMG_00001.png");
		#endif
	}
	
	snapshot_data->preview_image_index = &app_data->preview_image_index;
	snapshot_data->preview_label_string = app_data->preview_label_string;
	snapshot_data->preview_image_files = app_data->preview_image_files;
	snapshot_data->sd_cmd_array = app_data->sd_cmd_array;
	
	snapshot_data->preview_image_toggle_visibility_btn = data->preview_image_toggle_visibility_btn;
	snapshot_data->preview_image_widget = data->preview_image_widget;
	snapshot_data->preview_label = data->preview_label;
	snapshot_data->gen_btn = gen_btn;
	snapshot_data->halt_btn = data->halt_btn;
	snapshot_data->win = data->win;
	
	start_generation(snapshot_data);
}
