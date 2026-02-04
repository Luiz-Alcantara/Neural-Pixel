#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <time.h>
#include "constants.h"
#include "file_utils.h"
#include "structs.h"
#include "handle_cache.h"
#include "str_utils.h"
#include "widgets_cb.h"

void gen_sd_string(GenerationData *data)
{
	GtkTextBuffer *pos_tb = data->pos_p;
	GtkTextIter psi;
	GtkTextIter pei;
	gtk_text_buffer_get_bounds (pos_tb, &psi, &pei);
	char *p_text = gtk_text_buffer_get_text(pos_tb, &psi, &pei, FALSE);

	GtkTextBuffer *neg_tb = data->neg_p;
	GtkTextIter nsi;
	GtkTextIter nei;
	gtk_text_buffer_get_bounds (neg_tb, &nsi, &nei);
	char *n_text = gtk_text_buffer_get_text(neg_tb, &nsi, &nei, FALSE);

	char *seed_str;
	if (*data->seed_value == -1) {
		seed_str = generate_sd_seed();
	} else {
		seed_str = convert_long_long_int_to_string(*data->seed_value);
	}
	
	char *steps_str = convert_double_to_string(*data->steps_value, "%.0f");
	char *batch_count_str = convert_double_to_string(*data->batch_count_value, "%.0f");
	char *cfg_str = convert_double_to_string(*data->cfg_value, "%.1f");
	char *denoise_str = convert_double_to_string(*data->denoise_value, "%.2f");
	char *clip_skip_str = convert_double_to_string(*data->clip_skip_value, "%.0f");
	char *up_repeat_str = convert_double_to_string(*data->up_repeat_value, "%.0f");

	g_ptr_array_set_size(data->sd_cmd_array, 0);

	#ifdef G_OS_WIN32
		gchar *current_dir = g_get_current_dir();
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf(*data->cpu_bool == 1 ? "%s\\sd-cpu": "%s\\sd", current_dir));
		g_free(current_dir);
	#else
		g_ptr_array_add(data->sd_cmd_array, g_strdup(*data->cpu_bool == 1 ? "./sd-cpu" : "./sd"));
	#endif

	if (data->img2img_file_path != NULL && strcmp(data->img2img_file_path->str, "None") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--mode"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup("img_gen"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(*data->kontext_bool == 1 ? "--ref-image" : "--init-img"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(data->img2img_file_path->str));
	}

	if (data->checkpoint_string != NULL) {
		if (*data->sd_based_bool == 1) {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--model"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/checkpoints/%s", data->checkpoint_string->str));
		} else {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--diffusion-model"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/checkpoints/%s", data->checkpoint_string->str));
		}
	}
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--lora-model-dir"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup("./models/loras"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--embd-dir"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup("./models/embeddings"));

	if (data->vae_string != NULL && strcmp(data->vae_string->str, "None") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--vae"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/vae/%s", data->vae_string->str));
		if (*data->k_vae_bool == 1) {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--vae-on-cpu"));
		}
	}

	if (data->cnet_string != NULL && strcmp(data->cnet_string->str, "None") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--control-net"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/controlnet/%s", data->cnet_string->str));
		if (*data->k_cnet_bool == 1) {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--control-net-cpu"));
		}
	}

	if (data->upscaler_string != NULL && strcmp(data->upscaler_string->str, "None") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--upscale-model"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/upscale_models/%s", data->upscaler_string->str));
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--upscale-repeats"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(up_repeat_str));
	}
	
	if (data->clip_l_string != NULL && strcmp(data->clip_l_string->str, "None") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--clip_l"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/clips/%s", data->clip_l_string->str));
	}
	
	if (data->clip_g_string != NULL && strcmp(data->clip_g_string->str, "None") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--clip_g"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/clips/%s", data->clip_g_string->str));
	}
	
	if (data->text_enc_string != NULL && strcmp(data->text_enc_string->str, "None") != 0) {
		if (*data->llm_bool == 1) {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--llm"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/text_encoders/%s", data->text_enc_string->str));
		} else {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--t5xxl"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/text_encoders/%s", data->text_enc_string->str));
		}
	}
	
	if (*data->k_clip_bool == 1) {
		if (data->clip_l_string != NULL || data->clip_g_string != NULL || data->text_enc_string != NULL) {
			if (strcmp(data->clip_l_string->str, "None") != 0 || strcmp(data->clip_g_string->str, "None") != 0 || strcmp(data->text_enc_string->str, "None") != 0) {
				g_ptr_array_add(data->sd_cmd_array, g_strdup("--clip-on-cpu"));
			}
		}
	}

	g_ptr_array_add(data->sd_cmd_array, g_strdup("--strength"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup(denoise_str));

	if (*data->taesd_bool == 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--taesd"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(".models/vae/taesd_decoder.safetensors"));
	}

	g_ptr_array_add(data->sd_cmd_array, g_strdup("--cfg-scale"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup(cfg_str));
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--clip-skip"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup(clip_skip_str));

	if (*data->sampler_index < LIST_SAMPLES_COUNT - 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--sampling-method"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(LIST_SAMPLES[(*data->sampler_index)]));
	}
	
	if (*data->scheduler_index < LIST_SCHEDULES_COUNT - 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--scheduler"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(LIST_SCHEDULES[(*data->scheduler_index)]));
	}
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--seed"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup(seed_str != NULL ? seed_str : "-1"));
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--steps"));
	g_ptr_array_add(data->sd_cmd_array, steps_str != NULL ? g_strdup(steps_str) : g_strdup_printf("%f", DEFAULT_N_STEPS));
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--batch-count"));
	g_ptr_array_add(data->sd_cmd_array, batch_count_str != NULL ? g_strdup(batch_count_str) : g_strdup_printf("%f", DEFAULT_BATCH_COUNT));

	if (*data->w_index < LIST_RESOLUTIONS_STR_COUNT - 1 && *data->h_index < LIST_RESOLUTIONS_STR_COUNT - 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--width"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(LIST_RESOLUTIONS_STR[(*data->w_index)]));
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--height"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(LIST_RESOLUTIONS_STR[(*data->h_index)]));
	}

	if (*data->vt_bool == 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--vae-tiling"));
	}
	
	if (*data->ram_offload_bool == 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--offload-to-cpu"));
	}

	if (*data->fa_bool == 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--diffusion-fa"));
		if (*data->sd_based_bool != 1) g_ptr_array_add(data->sd_cmd_array, g_strdup("--chroma-disable-dit-mask"));
	}

	if (p_text != NULL) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--prompt"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(p_text));
	}

	if (n_text != NULL && strcmp(n_text, "") != 0 && strcmp(n_text, " ") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--negative-prompt"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(n_text));
	}
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--output"));
	
	char *timestamp = get_time_str();
	
	#ifdef G_OS_WIN32
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf(".\\outputs\\IMG_%s.png", timestamp));
	#else
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./outputs/IMG_%s.png", timestamp));
	#endif
	
	g_ptr_array_add(data->sd_cmd_array, NULL);

	if (*data->update_cache_bool == 1) {
		update_cache(data, data->checkpoint_string->str, data->vae_string->str, data->cnet_string->str, data->upscaler_string->str, data->clip_l_string->str, data->clip_g_string->str, data->text_enc_string->str, p_text, n_text, timestamp);
	}

	g_free(p_text);
	g_free(n_text);
	free(timestamp);
	if (seed_str != NULL) {
		free(seed_str);
	}
	free(steps_str);
	free(batch_count_str);
	free(cfg_str);
	free(denoise_str);
	free(clip_skip_str);
	free(up_repeat_str);
	
	if (*data->verbose_bool == 1 && data->sd_cmd_array != NULL) {
		char *final_cmd = g_strjoinv(" ", (char **)data->sd_cmd_array->pdata);
		g_print("Executing: %s\n", final_cmd);
		g_free(final_cmd);
	}
}
