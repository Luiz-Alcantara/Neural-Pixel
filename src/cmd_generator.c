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

void gen_sd_string(GenerationSnapshotData *data)
{
	g_ptr_array_set_size(data->sd_cmd_array, 0);

	#ifdef G_OS_WIN32
		gchar *current_dir = g_get_current_dir();
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf(data->cpu_mode_enabled == 1 ? "%s\\sd-cpu": "%s\\sd", current_dir));
		g_free(current_dir);
	#else
		g_ptr_array_add(data->sd_cmd_array, g_strdup(data->cpu_mode_enabled == 1 ? "./sd-cpu" : "./sd"));
	#endif
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--mode"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup("img_gen"));

	if (data->checkpoint_filename != NULL) {
		if (data->sd_based_enabled == 1) {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--model"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/checkpoints/%s", data->checkpoint_filename));
		} else {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--diffusion-model"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/checkpoints/%s", data->checkpoint_filename));
		}
	}
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--lora-model-dir"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup("./models/loras"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--embd-dir"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup("./models/embeddings"));

	if (data->vae_filename != NULL && strcmp(data->vae_filename, "None") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--vae"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/vae/%s", data->vae_filename));
		if (data->keep_vae_cpu_enabled == 1) {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--vae-on-cpu"));
		}
	}
	
	if (data->img2img_file_path != NULL && strcmp(data->img2img_file_path, "None") != 0) {
		if (data->cnet_filename != NULL && strcmp(data->cnet_filename, "None") != 0) {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--control-net"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/controlnet/%s", data->cnet_filename));
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--control-image"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup(data->img2img_file_path));
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--control-strength"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("%.2f", data->cnet_strength_value));
			if (data->keep_cnet_cpu_enabled == 1) {
				g_ptr_array_add(data->sd_cmd_array, g_strdup("--control-net-cpu"));
			}
		} else {
			g_ptr_array_add(data->sd_cmd_array, g_strdup(data->kontext_enabled == 1 ? "--ref-image" : "--init-img"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup(data->img2img_file_path));
		}
	}

	if (data->upscaler_filename != NULL && strcmp(data->upscaler_filename, "None") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--upscale-model"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/upscale_models/%s", data->upscaler_filename));
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--upscale-repeats"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("%d", data->upscale_passes_value));
	}
	
	if (data->clip_l_filename != NULL && strcmp(data->clip_l_filename, "None") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--clip_l"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/clips/%s", data->clip_l_filename));
	}
	
	if (data->clip_g_filename != NULL && strcmp(data->clip_g_filename, "None") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--clip_g"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/clips/%s", data->clip_g_filename));
	}
	
	if (data->text_enc_filename != NULL && strcmp(data->text_enc_filename, "None") != 0) {
		if (data->llm_mode_enabled == 1) {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--llm"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/text_encoders/%s", data->text_enc_filename));
		} else {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--t5xxl"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("./models/text_encoders/%s", data->text_enc_filename));
		}
	}
	
	if (data->keep_clip_cpu_enabled == 1) {
		if (data->clip_l_filename != NULL || data->clip_g_filename != NULL || data->text_enc_filename != NULL) {
			if (strcmp(data->clip_l_filename, "None") != 0 || strcmp(data->clip_g_filename, "None") != 0 || strcmp(data->text_enc_filename, "None") != 0) {
				g_ptr_array_add(data->sd_cmd_array, g_strdup("--clip-on-cpu"));
			}
		}
	}

	g_ptr_array_add(data->sd_cmd_array, g_strdup("--strength"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("%.2f", data->denoise_strength_value));
	
	if (data->taesd_enabled == 1) {
		if (check_file_exists(".models/vae/taesd_decoder.safetensors", 0) == 1) {
			g_ptr_array_add(data->sd_cmd_array, g_strdup("--taesd"));
			g_ptr_array_add(data->sd_cmd_array, g_strdup(".models/vae/taesd_decoder.safetensors"));
		} else {
			g_printerr("Error loading file '.models/vae/taesd_decoder.safetensors'. The file must be located in this exact path and have the correct name.\n");
		}
	}

	g_ptr_array_add(data->sd_cmd_array, g_strdup("--cfg-scale"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("%.1f", data->cfg_scale_value));
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--clip-skip"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup_printf("%d", data->clip_skip_value));

	if (data->sampler_index < LIST_SAMPLES_COUNT - 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--sampling-method"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(LIST_SAMPLES[(data->sampler_index)]));
	}
	
	if (data->scheduler_index < LIST_SCHEDULES_COUNT - 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--scheduler"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(LIST_SCHEDULES[(data->scheduler_index)]));
	}
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--seed"));
	g_ptr_array_add(data->sd_cmd_array, data->seed_value ? g_strdup_printf("%lld", data->seed_value) : g_strdup_printf("%lld", DEFAULT_SEED));
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--steps"));
	g_ptr_array_add(data->sd_cmd_array, data->step_count_value ? g_strdup_printf("%d", data->step_count_value) : g_strdup_printf("%d", DEFAULT_N_STEPS));
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--batch-count"));
	g_ptr_array_add(data->sd_cmd_array, data->batch_count_value ? g_strdup_printf("%d", data->batch_count_value) : g_strdup_printf("%d", DEFAULT_BATCH_COUNT));

	if (data->width_index < LIST_RESOLUTIONS_STR_COUNT - 1 && data->height_index < LIST_RESOLUTIONS_STR_COUNT - 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--width"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(LIST_RESOLUTIONS_STR[(data->width_index)]));
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--height"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(LIST_RESOLUTIONS_STR[(data->height_index)]));
	}

	if (data->vae_tiling_enabled == 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--vae-tiling"));
	}
	
	if (data->ram_offload_enabled == 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--offload-to-cpu"));
	}

	if (data->flash_att_enabled == 1) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--diffusion-fa"));
		if (data->sd_based_enabled != 1) g_ptr_array_add(data->sd_cmd_array, g_strdup("--chroma-disable-dit-mask"));
	}

	if (data->positive_prompt != NULL) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--prompt"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(data->positive_prompt));
	}

	if (data->negative_prompt && strcmp(data->negative_prompt, "") != 0 && strcmp(data->negative_prompt, " ") != 0) {
		g_ptr_array_add(data->sd_cmd_array, g_strdup("--negative-prompt"));
		g_ptr_array_add(data->sd_cmd_array, g_strdup(data->negative_prompt));
	}
	
	g_ptr_array_add(data->sd_cmd_array, g_strdup("--output"));
	g_ptr_array_add(data->sd_cmd_array, g_strdup(data->output_path));
	
	g_ptr_array_add(data->sd_cmd_array, NULL);

	if (data->update_cache_enabled == 1) {
		update_cache(data);
	}
	
	if (data->verbose_enabled == 1 && data->sd_cmd_array != NULL) {
		char *final_cmd = g_strjoinv(" ", (char **)data->sd_cmd_array->pdata);
		g_print("Executing: %s\n", final_cmd);
		g_free(final_cmd);
	}
}
