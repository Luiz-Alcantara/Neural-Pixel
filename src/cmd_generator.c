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

GString *gen_sd_string(GenerationData *data)
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
	
	char *cfg_str = convert_double_to_string(*data->cfg_value, "%.1f");
	char *denoise_str = convert_double_to_string(*data->denoise_value, "%.2f");
	char *clip_skip_str = convert_double_to_string(*data->clip_skip_value, "%.0f");
	char *up_repeat_str = convert_double_to_string(*data->up_repeat_value, "%.0f");

	GString *l1 = g_string_new(NULL);
	
	#ifdef G_OS_WIN32
		gchar *current_dir = g_get_current_dir();
		
		g_string_append(l1, current_dir);
		g_free(current_dir);
		
		if (*data->cpu_bool == 1) {
			g_string_append(l1, "\\sd-cpu");
		} else {
			g_string_append(l1, "\\sd");
		}
	#else
		if (*data->cpu_bool == 1) {
			g_string_append(l1, "./sd-cpu");
		} else {
			g_string_append(l1, "./sd");
		}
	#endif
	
	if (data->img2img_file_path != NULL && strcmp(data->img2img_file_path->str, "None") != 0) {
		g_string_append_printf(l1, "|-M|img2img|-i|%s", data->img2img_file_path->str);
	}

	if (data->model_string != NULL) {
		if (*data->sd_based_bool == 1) {
			g_string_append_printf(l1, "|--model|./models/checkpoints/%s", data->model_string->str);
		} else {
			g_string_append_printf(l1, "|--diffusion-model|./models/checkpoints/%s", data->model_string->str);
		}
	}
	
	g_string_append(l1, "|--lora-model-dir|./models/loras|--embd-dir|./models/embeddings");

	if (data->vae_string != NULL && strcmp(data->vae_string->str, "None") != 0) {
		g_string_append_printf(l1, "|--vae|./models/vae/%s", data->vae_string->str);
		if (*data->k_vae_bool == 1) {
			g_string_append(l1, "|--vae-on-cpu");
		}
	}

	if (data->cnet_string != NULL && strcmp(data->cnet_string->str, "None") != 0) {
		g_string_append_printf(l1, "|--control-net|./models/controlnet/%s", data->cnet_string->str);
		if (*data->k_cnet_bool == 1) {
			g_string_append(l1, "|--control-net-cpu");
		}
	}

	if (data->upscale_string != NULL && strcmp(data->upscale_string->str, "None") != 0) {
		g_string_append_printf(l1, "|--upscale-model|./models/upscale_models/%s|--upscale-repeats|%s", data->upscale_string->str, up_repeat_str);
	}
	
	if (data->clip_l_string != NULL && strcmp(data->clip_l_string->str, "None") != 0) {
		g_string_append_printf(l1, "|--clip_l|./models/clips/%s", data->clip_l_string->str);
	}
	
	if (data->clip_g_string != NULL && strcmp(data->clip_g_string->str, "None") != 0) {
		g_string_append_printf(l1, "|--clip_g|./models/clips/%s", data->clip_g_string->str);
	}
	
	if (data->t5xxl_string != NULL && strcmp(data->t5xxl_string->str, "None") != 0) {
		g_string_append_printf(l1, "|--t5xxl|./models/text_encoders/%s", data->t5xxl_string->str);
	}
	
	if (*data->k_clip_bool == 1) {
		if (data->clip_l_string != NULL || data->clip_g_string != NULL || data->t5xxl_string != NULL) {
			if (strcmp(data->clip_l_string->str, "None") != 0 || strcmp(data->clip_g_string->str, "None") != 0 || strcmp(data->t5xxl_string->str, "None") != 0) {
				g_string_append(l1, "|--clip-on-cpu");
			}
		}
	}

	g_string_append_printf(l1, "|--strength|%s", denoise_str);

	if (*data->taesd_bool == 1) {
		g_string_append(l1, "|--taesd|.models/TAESD/taesd_decoder.safetensors");
	}

	g_string_append_printf(l1, "|--cfg-scale|%s", cfg_str);
	
	g_string_append_printf(l1, "|--clip-skip|%s", clip_skip_str);

	if (*data->sample_index < LIST_SAMPLES_COUNT - 1) {
		g_string_append_printf(l1, "|--sampling-method|%s", LIST_SAMPLES[(*data->sample_index)]);
	}
	
	if (*data->schedule_index < LIST_SCHEDULES_COUNT - 1) {
		g_string_append_printf(l1, "|--scheduler|%s", LIST_SCHEDULES[(*data->schedule_index)]);
	}
	
	if (seed_str != NULL) {
		g_string_append_printf(l1,"|-s|%s", seed_str);
	} else {
		g_string_append(l1,"|-s|-1");
	}
	
	if (*data->n_steps_index < LIST_STEPS_STR_COUNT - 1) {
		g_string_append_printf(l1, "|--steps|%s", LIST_STEPS_STR[(*data->n_steps_index)]);
	}
	
	if (*data->bs_index < LIST_STEPS_STR_COUNT - 1) {
		g_string_append_printf(l1, "|-b|%s", LIST_STEPS_STR[(*data->bs_index)]);
	}

	if (data->img2img_file_path == NULL || strcmp(data->img2img_file_path->str, "None") == 0) {
		if (*data->w_index < LIST_RESOLUTIONS_STR_COUNT - 1 && *data->h_index < LIST_RESOLUTIONS_STR_COUNT - 1) {
			g_string_append_printf(l1, "|-W|%s", LIST_RESOLUTIONS_STR[(*data->w_index)]);
			g_string_append_printf(l1, "|-H|%s", LIST_RESOLUTIONS_STR[(*data->h_index)]);
		}
	}

	if (*data->vt_bool == 1) {
		g_string_append(l1, "|--vae-tiling");
	}

	if (*data->fa_bool == 1) {
		g_string_append(l1, "|--diffusion-fa");
	}

	if (p_text != NULL) {
		g_string_append_printf(l1, "|-p|\"%s\"", p_text);
	}

	if (n_text != NULL) {
		g_string_append_printf(l1, "|-n|\"%s\"", n_text);
	}
	
	char *timestamp = get_time_str();
	
	#ifdef G_OS_WIN32
		g_string_append_printf(l1, "|-o|.\\outputs\\IMG_%s.png", timestamp);
	#else
		g_string_append_printf(l1, "|-o|./outputs/IMG_%s.png", timestamp);
	#endif

	update_cache(data, data->model_string->str, data->vae_string->str, data->cnet_string->str, data->upscale_string->str, data->clip_l_string->str, data->clip_g_string->str, data->t5xxl_string->str, p_text, n_text, timestamp);

	g_free(p_text);
	g_free(n_text);
	free(timestamp);
	if (seed_str != NULL) {
		free(seed_str);
	}
	free(cfg_str);
	free(denoise_str);
	free(clip_skip_str);
	free(up_repeat_str);
	
	if (*data->verbose_bool == 1) {
		GString *l1_copy = g_string_new_len(l1->str, l1->len);

		for (gsize i = 0; i < l1_copy->len; i++) {
			if (l1_copy->str[i] == '|') {
				l1_copy->str[i] = ' ';
			}
		}
		
		printf("%s\n", l1_copy->str);
		g_string_free(l1_copy, TRUE);
	}
	
	return l1;
}
