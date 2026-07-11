#include <stdio.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "file_utils.h"
#include "structs.h"
#include "constants.h"
#include "str_utils.h"
#include "widgets_cb.h"

void create_cache(char *n, GError **error)
{
	DIR* cd = opendir(".cache");
	if (cd == NULL) {
		g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Directory '.cache' does not exist or cannot be accessed.");
		return;
	}
	closedir(cd);

	if (strcmp(n, ".cache/pp_cache") == 0) {
		FILE *pcf = fopen(".cache/pp_cache", "wb");
		if (pcf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "File '.cache/pp_cache' does not exist or cannot be accessed.");
			return;
		}
		fprintf(pcf, "%s", POSITIVE_PROMPT);
		fclose(pcf);
	}

	if (strcmp(n, ".cache/np_cache") == 0) {
		FILE *ncf = fopen(".cache/np_cache", "wb");
		if (ncf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "File '.cache/np_cache' does not exist or cannot be accessed.");
			return;
		}
		fprintf(ncf, "%s", NEGATIVE_PROMPT);
		fclose(ncf);
	}

	if (strcmp(n, ".cache/np_cache.ini") == 0) {
		FILE *cf = fopen(".cache/np_cache.ini", "wb");
		if (cf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "File '.cache/np_cache.ini' does not exist or cannot be accessed.");
			return;
		}
		fprintf(cf, "last_image_path=%s\n", DEFAULT_IMG_PATH);
		fprintf(cf, "checkpoint=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "vae=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "cnet=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "upscaler=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "clip_l=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "clip_g=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "text_enc=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "sampler_index=%d\n", DEFAULT_SAMPLER);
		fprintf(cf, "scheduler_index=%d\n", DEFAULT_SCHEDULER);
		fprintf(cf, "img_width_index=%d\n", DEFAULT_SIZE);
		fprintf(cf, "img_height_index=%d\n", DEFAULT_SIZE);
		fprintf(cf, "n_steps=%d.0\n", DEFAULT_N_STEPS);
		fprintf(cf, "batch_count=%d.0\n", DEFAULT_BATCH_COUNT);
		fprintf(cf, "kontext_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "inpaint_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "sd_based_bool=%d\n", ENABLED_OPT);
		fprintf(cf, "llm_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "hires_upscaler_index=%d\n", DISABLED_OPT);
		fprintf(cf, "hires_scale_value=%.2f\n", DEFAULT_HIRES_SCALE);
		fprintf(cf, "hires_steps_value=%d.0\n", DEFAULT_HIRES_STEPS);
		fprintf(cf, "hires_denoise_value=%.1f\n", DEFAULT_HIRES_DENOISE_STR);
		fprintf(cf, "flash_attn_value=%d\n", DISABLED_OPT);
		fprintf(cf, "vae_tiling_index=%d\n", DEFAULT_MODELS);
		fprintf(cf, "mmap_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "taesd_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "update_cache_bool=%d\n", ENABLED_OPT);
		fprintf(cf, "verbose_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "chroma_dit_mask_bool=%d\n", ENABLED_OPT);
		fprintf(cf, "qwen_zero_cond_t_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "model_runtime_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "model_param_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "te_runtime_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "te_param_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "vae_runtime_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "vae_param_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "cnet_runtime_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "cnet_param_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "upscaler_runtime_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "upscaler_param_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "seed=%lld\n", DEFAULT_SEED);
		fprintf(cf, "cfg_scale=%.1f\n", DEFAULT_CFG);
		fprintf(cf, "cnet_strength=%.2f\n", DEFAULT_CNET_STRENGTH);
		fprintf(cf, "denoise_strength=%.2f\n", DEFAULT_DENOISE);
		fprintf(cf, "clip_skip=%d\n", DEFAULT_CLIP_SKIP);
		fprintf(cf, "repeat_upscale=%d.0\n", DEFAULT_RP_UPSCALE);
		fclose(cf);
	}
	return;
}

char* ini_file_get_value(const char *filename, const char *search_key)
{
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		g_printerr("Failed to open file '%s', using default value(s).\n", filename);
		return NULL;
	}

	static char value[256];
	char line[256];

	while (fgets(line, sizeof(line), file)) {
		if (line[0] == '#' || line[0] == ';' || line[0] == '\n')
		continue;
		
		line[strcspn(line, "\n")] = '\0';
		
		char *equal_sign = strchr(line, '=');
		if (!equal_sign)
		continue;
		
		*equal_sign = '\0';
		char *key = line;
		char *val = equal_sign + 1;
		
		if (strcmp(key, search_key) == 0) {
		strncpy(value, val, sizeof(value) - 1);
		fclose(file);
		return value;
		}
	}

	g_printerr("[WARNING]: Could not find key '%s' in file '%s', using default value(s).\n", search_key, filename);
	fclose(file);
	return NULL;
}

void load_pp_cache(GtkTextBuffer *pos_tb)
{
	if (check_file_exists(".cache/pp_cache", 1) == 1) {
		FILE *pcf = fopen(".cache/pp_cache", "r");
		if (pcf == NULL) {
			g_printerr("Failed to open file '.cache/pp_cache', using default value(s).\n");
			gtk_text_buffer_set_text (pos_tb, POSITIVE_PROMPT, -1);
			return;
		}
		char line[512];
		int i = 0;
		
		fseek(pcf, 0, SEEK_END);
		long pcf_size = ftell(pcf);
		fseek(pcf, 0, SEEK_SET);
		char *pb = (char *)malloc(pcf_size + 1);
		if (pb == NULL) {
			g_printerr("Memory allocation error in function 'load_pp_cache', using default value(s)\n");
			fclose(pcf);
			gtk_text_buffer_set_text (pos_tb, POSITIVE_PROMPT, -1);
			return;
		}
		size_t pbr = fread(pb, 1, pcf_size, pcf);
		if (pbr != pcf_size) {
			g_printerr("Error reading file '.cache/pp_cache', using default value(s)\n");
			free(pb);
			fclose(pcf);
			gtk_text_buffer_set_text (pos_tb, POSITIVE_PROMPT, -1);
			return;
		}
		pb[pbr] = '\0';

		gtk_text_buffer_set_text (pos_tb, pb, -1);

		free(pb);
		fclose(pcf);
	} else {
		g_printerr("Error loading '.cache/pp_cache', using default value(s).\n");
		gtk_text_buffer_set_text (pos_tb, POSITIVE_PROMPT, -1);
	}
}

void load_np_cache(GtkTextBuffer *neg_tb)
{
	if (check_file_exists(".cache/np_cache", 1) == 1) {
		FILE *ncf = fopen(".cache/np_cache", "r");
		if (ncf == NULL) {
			g_printerr("Failed to open file '.cache/np_cache', using default value(s).\n");
			gtk_text_buffer_set_text (neg_tb, NEGATIVE_PROMPT, -1);
			return;
		}
		char line[512];
		int i = 0;
		
		fseek(ncf, 0, SEEK_END);
		long ncf_size = ftell(ncf);
		fseek(ncf, 0, SEEK_SET);
		char *nb = (char *)malloc(ncf_size + 1);
		if (nb == NULL) {
			g_printerr("Memory allocation error in function 'load_np_cache', using default value(s)\n");
			fclose(ncf);
			gtk_text_buffer_set_text (neg_tb, NEGATIVE_PROMPT, -1);
			return;
		}
		size_t nbr = fread(nb, 1, ncf_size, ncf);
		if (nbr != ncf_size) {
			g_printerr("Error reading file '.cache/np_cache', using default value(s)\n");
			free(nb);
			fclose(ncf);
			gtk_text_buffer_set_text (neg_tb, NEGATIVE_PROMPT, -1);
			return ;
		}
		nb[nbr] = '\0';

		gtk_text_buffer_set_text (neg_tb, nb, -1);

		free(nb);
		fclose(ncf);
	} else {
		g_printerr("Error loading '.cache/np_cache', using default value(s).\n");
		gtk_text_buffer_set_text (neg_tb, NEGATIVE_PROMPT, -1);
	}
}

void load_cache_fallback(gpointer user_data)
{
	AppStartData *data = user_data;
	
	g_string_assign(data->checkpoint_string, OPTIONAL_ITEMS);
	g_string_assign(data->vae_string, OPTIONAL_ITEMS);
	g_string_assign(data->cnet_string, OPTIONAL_ITEMS);
	g_string_assign(data->upscaler_string, OPTIONAL_ITEMS);
	g_string_assign(data->clip_l_string, OPTIONAL_ITEMS);
	g_string_assign(data->clip_g_string, OPTIONAL_ITEMS);
	g_string_assign(data->text_enc_string, OPTIONAL_ITEMS);
	
	data->sampler_index = DEFAULT_SAMPLER;
	data->scheduler_index = DEFAULT_SCHEDULER;
	data->w_index = DEFAULT_SIZE;
	data->h_index = DEFAULT_SIZE;
	data->steps_value = DEFAULT_N_STEPS;
	data->batch_count_value = DEFAULT_BATCH_COUNT;
	data->kontext_bool = DISABLED_OPT;
	data->inpaint_bool = DISABLED_OPT;
	data->sd_based_bool = ENABLED_OPT;
	data->llm_bool = DISABLED_OPT;
	data->hires_upscaler_index = DISABLED_OPT;
	data->flash_attn_value = DISABLED_OPT;
	data->mmap_bool = DISABLED_OPT;
	data->taesd_bool = DISABLED_OPT;
	data->update_cache_bool = ENABLED_OPT;
	data->verbose_bool = DISABLED_OPT;
	data->chroma_dit_mask_bool = ENABLED_OPT;
	data->qwen_zero_cond_t_bool = DISABLED_OPT;
	
	data->seed_value = DEFAULT_SEED;
	
	data->cfg_value = DEFAULT_CFG;
	data->cnet_value = DEFAULT_CNET_STRENGTH;
	data->denoise_value = DEFAULT_DENOISE;
	data->clip_skip_value = DEFAULT_CLIP_SKIP;
	data->up_repeat_value = DEFAULT_RP_UPSCALE;
	data->hires_scale_value = DEFAULT_HIRES_SCALE;
	data->hires_steps_value = DEFAULT_HIRES_STEPS;
	data->hires_denoise_value = DEFAULT_HIRES_DENOISE_STR;
}

void load_cache(gpointer user_data)
{

	char *cache_filename = ".cache/np_cache.ini";

	if (check_file_exists(cache_filename, 1) == 1) {
		AppStartData *data = user_data;
		
		get_png_files(data->preview_image_files);
		
		char *img_str = ini_file_get_value(cache_filename, "last_image_path");
		if (img_str && check_file_exists(img_str, 1) == 1) {
			set_current_image_index(img_str + 2, data->preview_label_string, data->preview_image_files, &data->preview_image_index, -1);
		} else {
			g_printerr("Error loading image: The file '%s' is missing, corrupted, or invalid.\n", img_str);
		}
		
		char *checkpoint_str = ini_file_get_value(cache_filename, "checkpoint");
		if (checkpoint_str) {
			g_string_assign(data->checkpoint_string, checkpoint_str);
		} else {
			g_string_assign(data->checkpoint_string, OPTIONAL_ITEMS);
		}
		
		char *vae_str = ini_file_get_value(cache_filename, "vae");
		if (vae_str) {
			g_string_assign(data->vae_string, vae_str);
		} else {
			g_string_assign(data->vae_string, OPTIONAL_ITEMS);
		}
		
		char *cnet_str = ini_file_get_value(cache_filename, "cnet");
		if (cnet_str) {
			g_string_assign(data->cnet_string, cnet_str);
		} else {
			g_string_assign(data->cnet_string, OPTIONAL_ITEMS);
		}
		
		char *upscaler_str = ini_file_get_value(cache_filename, "upscaler");
		if (upscaler_str) {
			g_string_assign(data->upscaler_string, upscaler_str);
		} else {
			g_string_assign(data->upscaler_string, OPTIONAL_ITEMS);
		}
		
		char *clip_l_str = ini_file_get_value(cache_filename, "clip_l");
		if (clip_l_str) {
			g_string_assign(data->clip_l_string, clip_l_str);
		} else {
			g_string_assign(data->clip_l_string, OPTIONAL_ITEMS);
		}
		
		char *clip_g_str = ini_file_get_value(cache_filename, "clip_g");
		if (clip_g_str) {
			g_string_assign(data->clip_g_string, clip_g_str);
		} else {
			g_string_assign(data->clip_g_string, OPTIONAL_ITEMS);
		}
		
		char *text_enc_str = ini_file_get_value(cache_filename, "text_enc");
		if (text_enc_str) {
			g_string_assign(data->text_enc_string, text_enc_str);
		} else {
			g_string_assign(data->text_enc_string, OPTIONAL_ITEMS);
		}

		char *sampler_index_str = ini_file_get_value(cache_filename, "sampler_index");
		if (sampler_index_str) {
			sscanf(sampler_index_str, "%d", &data->sampler_index);
		} else {
			data->sampler_index = DEFAULT_SAMPLER;
		}
		
		char *scheduler_index_str = ini_file_get_value(cache_filename, "scheduler_index");
		if (scheduler_index_str) {
			sscanf(scheduler_index_str, "%d", &data->scheduler_index);
		} else {
			data->scheduler_index = DEFAULT_SCHEDULER;
		}
		
		char *img_width_index_str = ini_file_get_value(cache_filename, "img_width_index");
		if (img_width_index_str) {
			sscanf(img_width_index_str, "%d", &data->w_index);
		} else {
			data->w_index = DEFAULT_SIZE;
		}
		
		char *img_height_index_str = ini_file_get_value(cache_filename, "img_height_index");
		if (img_height_index_str) {
			sscanf(img_height_index_str, "%d", &data->h_index);
		} else {
			data->h_index = DEFAULT_SIZE;
		}
		
		char *n_steps_str = ini_file_get_value(cache_filename, "n_steps");
		if (n_steps_str) {
			char *endptr;
			data->steps_value = g_ascii_strtod(n_steps_str, &endptr);
		} else {
			data->steps_value = DEFAULT_N_STEPS;
		}
		
		char *batch_count_str = ini_file_get_value(cache_filename, "batch_count");
		if (batch_count_str) {
			char *endptr;
			data->batch_count_value = g_ascii_strtod(batch_count_str, &endptr);
		} else {
			data->batch_count_value = DEFAULT_BATCH_COUNT;
		}

		char *kontext_bool_str = ini_file_get_value(cache_filename, "kontext_bool");
		if (kontext_bool_str) {
			sscanf(kontext_bool_str, "%d", &data->kontext_bool);
		} else {
			data->kontext_bool = DISABLED_OPT;
		}
		
		
		char *inpaint_bool_str = ini_file_get_value(cache_filename, "inpaint_bool");
		if (inpaint_bool_str) {
			//Keep it disabled at startup for now.
			data->inpaint_bool = DISABLED_OPT;
		} else {
			data->inpaint_bool = DISABLED_OPT;
		}

		char *sd_based_bool_str = ini_file_get_value(cache_filename, "sd_based_bool");
		if (sd_based_bool_str) {
			sscanf(sd_based_bool_str, "%d", &data->sd_based_bool);
		} else {
			data->sd_based_bool = ENABLED_OPT;
		}
		
		char *llm_bool_str = ini_file_get_value(cache_filename, "llm_bool");
		if (llm_bool_str) {
			sscanf(llm_bool_str, "%d", &data->llm_bool);
		} else {
			data->llm_bool = DISABLED_OPT;
		}
		
		char *hires_upscaler_index_str = ini_file_get_value(cache_filename, "hires_upscaler_index");
		if (hires_upscaler_index_str) {
			sscanf(hires_upscaler_index_str, "%d", &data->hires_upscaler_index);
		} else {
			data->hires_upscaler_index = DISABLED_OPT;
		}
		
		char *hires_scale_value_str = ini_file_get_value(cache_filename, "hires_scale_value");
		if (hires_scale_value_str) {
			char *endptr;
			data->hires_scale_value = g_ascii_strtod(hires_scale_value_str, &endptr);
		} else {
			data->hires_scale_value = DEFAULT_HIRES_SCALE;
		}
		
		char *hires_steps_value_str = ini_file_get_value(cache_filename, "hires_steps_value");
		if (hires_steps_value_str) {
			char *endptr;
			data->hires_steps_value = g_ascii_strtod(hires_steps_value_str, &endptr);
		} else {
			data->hires_steps_value = DEFAULT_HIRES_STEPS;
		}
		
		char *hires_denoise_value_str = ini_file_get_value(cache_filename, "hires_denoise_value");
		if (hires_denoise_value_str) {
			char *endptr;
			data->hires_denoise_value = g_ascii_strtod(hires_denoise_value_str, &endptr);
		} else {
			data->hires_denoise_value = DEFAULT_HIRES_DENOISE_STR;
		}
		
		char *flash_attn_value_str = ini_file_get_value(cache_filename, "flash_attn_value");
		if (flash_attn_value_str) {
			sscanf(flash_attn_value_str, "%d", &data->flash_attn_value);
		} else {
			data->flash_attn_value = DISABLED_OPT;
		}
		
		char *vae_tiling_index_str = ini_file_get_value(cache_filename, "vae_tiling_index");
		if (vae_tiling_index_str) {
			sscanf(vae_tiling_index_str, "%d", &data->vae_tiling_index);
		} else {
			data->vae_tiling_index = DEFAULT_MODELS;
		}
		
		char *mmap_bool_str = ini_file_get_value(cache_filename, "mmap_bool");
		if (mmap_bool_str) {
			sscanf(mmap_bool_str, "%d", &data->mmap_bool);
		} else {
			data->mmap_bool = DISABLED_OPT;
		}
		
		char *taesd_bool_str = ini_file_get_value(cache_filename, "taesd_bool");
		if (taesd_bool_str) {
			sscanf(taesd_bool_str, "%d", &data->taesd_bool);
		} else {
			data->taesd_bool = DISABLED_OPT;
		}
		
		char *update_cache_bool_str = ini_file_get_value(cache_filename, "update_cache_bool");
		if (update_cache_bool_str) {
			sscanf(update_cache_bool_str, "%d", &data->update_cache_bool);
		} else {
			data->update_cache_bool = ENABLED_OPT;
		}
		
		char *verbose_bool_str = ini_file_get_value(cache_filename, "verbose_bool");
		if (verbose_bool_str) {
			sscanf(verbose_bool_str, "%d", &data->verbose_bool);
		} else {
			data->verbose_bool = DISABLED_OPT;
		}

		char *chroma_dit_mask_bool_str = ini_file_get_value(cache_filename, "chroma_dit_mask_bool");
		if (chroma_dit_mask_bool_str) {
			sscanf(chroma_dit_mask_bool_str, "%d", &data->chroma_dit_mask_bool);
		} else {
			data->chroma_dit_mask_bool = ENABLED_OPT;
		}

		char *qwen_zero_cond_t_bool_str = ini_file_get_value(cache_filename, "qwen_zero_cond_t_bool");
		if (qwen_zero_cond_t_bool_str) {
			sscanf(qwen_zero_cond_t_bool_str, "%d", &data->qwen_zero_cond_t_bool);
		} else {
			data->qwen_zero_cond_t_bool = DISABLED_OPT;
		}
		
		char *model_runtime_backend_index_str = ini_file_get_value(cache_filename, "model_runtime_backend_index");
		if (model_runtime_backend_index_str) {
			sscanf(model_runtime_backend_index_str, "%d", &data->model_runtime_backend_index);
		} else {
			data->model_runtime_backend_index = DEFAULT_BACKEND;
		}
		
		char *model_param_backend_index_str = ini_file_get_value(cache_filename, "model_param_backend_index");
		if (model_param_backend_index_str) {
			sscanf(model_param_backend_index_str, "%d", &data->model_param_backend_index);
		} else {
			data->model_param_backend_index = DEFAULT_BACKEND;
		}
		
		char *te_runtime_backend_index_str = ini_file_get_value(cache_filename, "te_runtime_backend_index");
		if (te_runtime_backend_index_str) {
			sscanf(te_runtime_backend_index_str, "%d", &data->te_runtime_backend_index);
		} else {
			data->te_runtime_backend_index = DEFAULT_BACKEND;
		}
		
		char *te_param_backend_index_str = ini_file_get_value(cache_filename, "te_param_backend_index");
		if (te_param_backend_index_str) {
			sscanf(te_param_backend_index_str, "%d", &data->te_param_backend_index);
		} else {
			data->te_param_backend_index = DEFAULT_BACKEND;
		}
		
		char *vae_runtime_backend_index_str = ini_file_get_value(cache_filename, "vae_runtime_backend_index");
		if (vae_runtime_backend_index_str) {
			sscanf(vae_runtime_backend_index_str, "%d", &data->vae_runtime_backend_index);
		} else {
			data->vae_runtime_backend_index = DEFAULT_BACKEND;
		}
		
		char *vae_param_backend_index_str = ini_file_get_value(cache_filename, "vae_param_backend_index");
		if (vae_param_backend_index_str) {
			sscanf(vae_param_backend_index_str, "%d", &data->vae_param_backend_index);
		} else {
			data->vae_param_backend_index = DEFAULT_BACKEND;
		}
		
		char *cnet_runtime_backend_index_str = ini_file_get_value(cache_filename, "cnet_runtime_backend_index");
		if (cnet_runtime_backend_index_str) {
			sscanf(cnet_runtime_backend_index_str, "%d", &data->cnet_runtime_backend_index);
		} else {
			data->cnet_runtime_backend_index = DEFAULT_BACKEND;
		}
		
		char *cnet_param_backend_index_str = ini_file_get_value(cache_filename, "cnet_param_backend_index");
		if (cnet_param_backend_index_str) {
			sscanf(cnet_param_backend_index_str, "%d", &data->cnet_param_backend_index);
		} else {
			data->cnet_param_backend_index = DEFAULT_BACKEND;
		}
		
		char *upscaler_runtime_backend_index_str = ini_file_get_value(cache_filename, "upscaler_runtime_backend_index");
		if (upscaler_runtime_backend_index_str) {
			sscanf(upscaler_runtime_backend_index_str, "%d", &data->upscaler_runtime_backend_index);
		} else {
			data->upscaler_runtime_backend_index = DEFAULT_BACKEND;
		}
		
		char *upscaler_param_backend_index_str = ini_file_get_value(cache_filename, "upscaler_param_backend_index");
		if (upscaler_param_backend_index_str) {
			sscanf(upscaler_param_backend_index_str, "%d", &data->upscaler_param_backend_index);
		} else {
			data->upscaler_param_backend_index = DEFAULT_BACKEND;
		}

		char *seed_str = ini_file_get_value(cache_filename, "seed");
		if (seed_str) {
			sscanf(seed_str, "%lld", &data->seed_value);
		} else {
			data->seed_value = DEFAULT_SEED;
		}
		
		char *cfg_scale_str = ini_file_get_value(cache_filename, "cfg_scale");
		if (cfg_scale_str) {
			char *endptr;
			data->cfg_value = g_ascii_strtod(cfg_scale_str, &endptr);
		} else {
			data->cfg_value = DEFAULT_CFG;
		}
		
		char *cnet_strength_str = ini_file_get_value(cache_filename, "cnet_strength");
		if (cnet_strength_str) {
			char *endptr;
			data->cnet_value = g_ascii_strtod(cnet_strength_str, &endptr);
		} else {
			data->cnet_value = DEFAULT_CNET_STRENGTH;
		}
		
		char *denoise_strength_str = ini_file_get_value(cache_filename, "denoise_strength");
		if (denoise_strength_str) {
			char *endptr;
			data->denoise_value = g_ascii_strtod(denoise_strength_str, &endptr);
		} else {
			data->denoise_value = DEFAULT_DENOISE;
		}
		
		char *clip_skip_str = ini_file_get_value(cache_filename, "clip_skip");
		if (clip_skip_str) {
			char *endptr;
			data->clip_skip_value = g_ascii_strtod(clip_skip_str, &endptr);
		} else {
			data->clip_skip_value = DEFAULT_CLIP_SKIP;
		}
		
		char *repeat_upscale_str = ini_file_get_value(cache_filename, "repeat_upscale");
		if (repeat_upscale_str) {
			char *endptr;
			data->up_repeat_value = g_ascii_strtod(repeat_upscale_str, &endptr);
		} else {
			data->up_repeat_value = DEFAULT_RP_UPSCALE;
		}
	} else {
		g_printerr("Error loading '.cache/np_cache.ini', using default value(s).\n");
		load_cache_fallback(user_data);
	}
}

void update_cache(GenerationSnapshotData *data)
{
	FILE *pcf = fopen(".cache/pp_cache", "wb");
	FILE *ncf = fopen(".cache/np_cache", "wb");
	FILE *cf = fopen(".cache/np_cache.ini", "wb");
	if (pcf == NULL || ncf == NULL || cf == NULL) {
		g_printerr("Error updating cache. If the error persists, try deleting the '.cache' directory.\n");
		return;
	}

	fprintf(pcf, "%s", data->positive_prompt);
	fclose(pcf);

	fprintf(ncf, "%s", data->negative_prompt);
	fclose(ncf);

	if (data->batch_count_value > 1) {
		size_t result_img_len = strlen(data->output_path);
		char *bn = g_strndup(data->output_path, result_img_len - 4);
		fprintf(cf, "last_image_path=%s_%d.png\n", bn, data->batch_count_value - 1);
		g_free(bn);
	} else {
		fprintf(cf, "last_image_path=%s\n", data->output_path);
	}
	
	fprintf(cf, "checkpoint=%s\n", data->checkpoint_filename);
	fprintf(cf, "vae=%s\n", data->vae_filename);
	fprintf(cf, "cnet=%s\n", data->cnet_filename);
	fprintf(cf, "upscaler=%s\n", data->upscaler_filename);
	fprintf(cf, "clip_l=%s\n", data->clip_l_filename);
	fprintf(cf, "clip_g=%s\n", data->clip_g_filename);
	fprintf(cf, "text_enc=%s\n", data->text_enc_filename);
	fprintf(cf, "sampler_index=%d\n", data->sampler_index);
	fprintf(cf, "scheduler_index=%d\n", data->scheduler_index);
	fprintf(cf, "img_width_index=%d\n", data->width_index);
	fprintf(cf, "img_height_index=%d\n", data->height_index);
	fprintf(cf, "n_steps=%.1f\n", (float)data->step_count_value);
	fprintf(cf, "batch_count=%.1f\n", (float)data->batch_count_value);
	fprintf(cf, "kontext_bool=%d\n", data->kontext_enabled);
	fprintf(cf, "inpaint_bool=%d\n", data->inpaint_enabled);
	fprintf(cf, "sd_based_bool=%d\n", data->sd_based_enabled);
	fprintf(cf, "llm_bool=%d\n", data->llm_mode_enabled);
	fprintf(cf, "hires_upscaler_index=%d\n", data->hires_upscaler_index);
	fprintf(cf, "hires_scale_value=%.2f\n", data->hires_scale_value);
	fprintf(cf, "hires_steps_value=%.1f\n", (float)data->hires_steps_value);
	fprintf(cf, "hires_denoise_value=%.2f\n", data->hires_denoise_value);
	fprintf(cf, "flash_attn_value=%d\n", data->flash_attn_value);
	fprintf(cf, "vae_tiling_index=%d\n", data->vae_tiling_index);
	fprintf(cf, "mmap_bool=%d\n", data->mmap_enabled);
	fprintf(cf, "taesd_bool=%d\n", data->taesd_enabled);
	fprintf(cf, "update_cache_bool=%d\n", ENABLED_OPT);
	fprintf(cf, "verbose_bool=%d\n", data->verbose_enabled);
	fprintf(cf, "chroma_dit_mask_bool=%d\n", data->chroma_dit_mask_enabled);
	fprintf(cf, "qwen_zero_cond_t_bool=%d\n", data->qwen_zero_cond_t_enabled);
	fprintf(cf, "model_runtime_backend_index=%d\n", data->model_runtime_backend_index);
	fprintf(cf, "model_param_backend_index=%d\n", data->model_param_backend_index);
	fprintf(cf, "te_runtime_backend_index=%d\n", data->te_runtime_backend_index);
	fprintf(cf, "te_param_backend_index=%d\n", data->te_param_backend_index);
	fprintf(cf, "vae_runtime_backend_index=%d\n", data->vae_runtime_backend_index);
	fprintf(cf, "vae_param_backend_index=%d\n", data->vae_param_backend_index);
	fprintf(cf, "cnet_runtime_backend_index=%d\n", data->cnet_runtime_backend_index);
	fprintf(cf, "cnet_param_backend_index=%d\n", data->cnet_param_backend_index);
	fprintf(cf, "upscaler_runtime_backend_index=%d\n", data->upscaler_runtime_backend_index);
	fprintf(cf, "upscaler_param_backend_index=%d\n", data->upscaler_param_backend_index);
	fprintf(cf, "seed=%lld\n", data->seed_value);
	fprintf(cf, "cfg_scale=%.1f\n", data->cfg_scale_value);
	fprintf(cf, "cnet_strength=%.2f\n", data->cnet_strength_value);
	fprintf(cf, "denoise_strength=%.2f\n", data->denoise_strength_value);
	fprintf(cf, "clip_skip=%.1f\n", (float)data->clip_skip_value);
	fprintf(cf, "repeat_upscale=%.1f\n", (float)data->upscale_passes_value);
	fclose(cf);

	return;
}
