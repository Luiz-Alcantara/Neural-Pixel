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
			char *newline_escape = strchr(data->stdout_string->str, '\n');

			if (clear_escape) {
				size_t end_index = (clear_escape - data->stdout_string->str) + 3;

				char *raw_string = g_strndup(data->stdout_string->str, end_index);

				char *clean_string = strrchr(raw_string, '\r');
				if (clean_string) {
					if (data->verbose_bool) {
						printf("%s\n", clean_string);
					}
					
					const char *last_pipe = strrchr(clean_string, '|');
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
								g_printerr("Error: Could not fetch progress from line: %s\n", clean_string);
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
					data->is_decoding_latents = 1;
					int x;
					int n_dec_latents;

					if (sscanf(line,
					"[INFO ] stable-diffusion.cpp:%i - decoding %i latents",
					&x, &n_dec_latents) == 2) {
						gtk_button_set_label(GTK_BUTTON(data->button), "Decoding...");
						data->n_current_latent++;
					}
				} else if (strstr(line, "generate_image completed in") != NULL && data->is_decoding_latents) {
					data->is_decoding_latents = 0;
					data->dec_latents_completed = 1;
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
	EndGenerationData *data = user_data;
	*data->sdpid = 0;
	const char *icon_n = gtk_button_get_icon_name (GTK_BUTTON(data->show_img_btn));
	GtkPicture *prev_img = GTK_PICTURE(data->image_widget);
	
	if (check_file_exists(data->result_img_path, 0) == 1) {
		get_png_files(data->image_files);
		set_current_image_index(data->result_img_path, data->img_index_string, data->image_files, data->current_image_index);
		
		if (data->image_files->len > 0) {
			if (g_strcmp0 (icon_n, "view-conceal-symbolic") != 0) {
				gtk_picture_set_filename(prev_img, data->result_img_path);
			} else {
				gtk_picture_set_filename(prev_img, EMPTY_IMG_PATH);
			}
		} else {
			g_printerr("No images in 'outputs' directory.\n");
			gtk_picture_set_filename(prev_img, DEFAULT_IMG_PATH);
		}
		
		gtk_label_set_label(GTK_LABEL(data->img_index_label), data->img_index_string->str);
	} else {
		g_printerr(
			"Error loading image: The file '%s' is missing, corrupted, or invalid.\n",
			data->result_img_path
		);
		gtk_picture_set_filename(prev_img, DEFAULT_IMG_PATH);
	}
	
	gtk_button_set_label(GTK_BUTTON(data->button), "Generate");
	gtk_widget_set_sensitive(GTK_WIDGET(data->button), TRUE);
	gtk_widget_set_sensitive(GTK_WIDGET(data->halt_btn), FALSE);
	g_strfreev(data->cmd_chunks);
	g_string_free(data->cmd_string, TRUE);
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

void generate_cb(GtkButton *gen_btn, gpointer user_data)
{
	GenerationData *data = user_data;
	if (data == NULL) {
		g_printerr("Error: Data used to generate the images is corrupted.\n");
		
		show_error_message(data->win,
		"Error reading generation data",
		"Error reading generation data;\ntry restarting the app or deleting the \".cache\" folder.");
		
		return;
	}
	if (g_strcmp0 (gtk_button_get_label (gen_btn), "Generate") == 0) {
		gtk_button_set_label (gen_btn, "Loading Files...");
		gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(data->halt_btn), TRUE);
	}

	GString *cmd_string = gen_sd_string(data);

	//TODO: make gen_sd_string return a gchar** array
	gchar **cmd_chunks = g_strsplit(cmd_string->str, "|", -1);
	gchar *result_img_path = NULL;
	
	gint chunk_count = g_strv_length(cmd_chunks);
	if (chunk_count > 0) {
		result_img_path = cmd_chunks[chunk_count - 1];
	}

	GSubprocessFlags sd_flags = G_SUBPROCESS_FLAGS_STDOUT_PIPE | G_SUBPROCESS_FLAGS_STDERR_PIPE;
	GError *error = NULL;

	GSubprocess* sd_process = g_subprocess_newv((const gchar * const *)cmd_chunks, sd_flags, &error);

	if (sd_process == NULL) {
		g_print("Error spawning process: %s\n", error->message);
		g_clear_error(&error);
		
		show_error_message(data->win,
		"Error spawning process",
		"Error spawning the sd.cpp process;\nlook at the terminal log for details.");
		
		g_strfreev(cmd_chunks);
		g_string_free(cmd_string, TRUE);
		
		gtk_button_set_label (gen_btn, "Generate");
		gtk_widget_set_sensitive(GTK_WIDGET(gen_btn), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(data->halt_btn), FALSE);
		
		return;
	} else {
		const gchar* sd_pid = g_subprocess_get_identifier(sd_process);
		char *endptr;
		int npid = strtol(sd_pid, &endptr, 10);

		if (*endptr == '\0') {
			*data->sdpid = npid;
		} else {
			*data->sdpid = 0;
		}

		GInputStream *stdout_stream = g_subprocess_get_stdout_pipe(sd_process);
		GInputStream *stderr_stream = g_subprocess_get_stderr_pipe(sd_process);

		GDataInputStream *data_err_stream = g_data_input_stream_new(stderr_stream);
		
		g_data_input_stream_set_newline_type (data_err_stream, G_DATA_STREAM_NEWLINE_TYPE_LF);

		EndGenerationData *check_d = g_new0 (EndGenerationData, 1);
		check_d->sdpid = data->sdpid;
		check_d->image_files = data->image_files;
		check_d->current_image_index = data->current_image_index;
		check_d->result_img_path = result_img_path;
		check_d->cmd_chunks = cmd_chunks;
		check_d->cmd_string = cmd_string;
		check_d->img_index_string = data->img_index_string;
		check_d->button = GTK_WIDGET(gen_btn);
		check_d->image_widget = data->image_widget;
		check_d->img_index_label = data->img_index_label;
		check_d->show_img_btn = data->show_img_btn;
		check_d->halt_btn = data->halt_btn;
		
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
		output_d->verbose_bool = *data->verbose_bool;
		output_d->button = GTK_WIDGET(gen_btn);
		output_d->sdpid = data->sdpid;
		output_d->out_pipe_stream = stdout_stream;
		output_d->stdout_string = g_string_new("");
		
		SDProcessErrorData *error_d = g_new0 (SDProcessErrorData, 1);
		error_d->verbose_bool = *data->verbose_bool;
		error_d->win = data->win;
		error_d->sdpid = data->sdpid;
		error_d->err_pipe_stream = data_err_stream;

		printf("Binary launched in subprocess: %d\n", *data->sdpid);

		GCancellable* cnlb = g_cancellable_new ();

		start_reading_output(output_d);
		start_reading_error(error_d);
		g_subprocess_wait_async(sd_process, cnlb, on_subprocess_end, check_d);
		g_object_unref(cnlb);
	}
}
