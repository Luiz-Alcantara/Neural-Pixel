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
	
	if (strcmp(n, ".cache/img_cache") == 0) {
		FILE *imgcf = fopen(".cache/img_cache", "wb");
		if (imgcf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "File '.cache/img_cache' does not exist or cannot be accessed.");
			return;
		}
		fprintf(imgcf, "%s", DEFAULT_IMG_PATH);
		fclose(imgcf);
	}

	if (strcmp(n, ".cache/np_cache.ini") == 0) {
		FILE *cf = fopen(".cache/np_cache.ini", "wb");
		if (cf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "File '.cache/np_cache.ini' does not exist or cannot be accessed.");
			return;
		}
		fprintf(cf, "checkpoint=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "vae=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "cnet=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "upscaler=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "clip_l=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "clip_g=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "t5xxl=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "sampler_index=%d\n", DEFAULT_SAMPLER);
		fprintf(cf, "scheduler_index=%d\n", DEFAULT_SCHEDULER);
		fprintf(cf, "img_width_index=%d\n", DEFAULT_SIZE);
		fprintf(cf, "img_height_index=%d\n", DEFAULT_SIZE);
		fprintf(cf, "n_steps=%.1f\n", DEFAULT_N_STEPS);
		fprintf(cf, "batch_count=%.1f\n", DEFAULT_BATCH_COUNT);
		fprintf(cf, "sd_based_bool=%d\n", ENABLED_OPT);
		fprintf(cf, "cpu_mode_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "vae_tiling_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "ram_offload_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "keep_clip_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "keep_cnet_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "keep_vae_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "flash_attention_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "taesd_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "update_cache_bool=%d\n", ENABLED_OPT);
		fprintf(cf, "verbose_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "seed=%lld\n", DEFAULT_SEED);
		fprintf(cf, "cfg_scale=%.1f\n", DEFAULT_CFG);
		fprintf(cf, "denoise_strength=%.2f\n", DEFAULT_DENOISE);
		fprintf(cf, "clip_skip=%.1f\n", DEFAULT_CLIP_SKIP);
		fprintf(cf, "repeat_upscale=%.1f\n", DEFAULT_RP_UPSCALE);
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

	g_printerr("Error: Could not find key '%s' in file '%s', using default value(s).\n", search_key, filename);
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

void load_img_cache(GtkWidget *img_wgt)
{
	if (check_file_exists(".cache/img_cache", 1) == 1) {
		FILE *imgf = fopen(".cache/img_cache", "r");
		if (imgf == NULL) {
			g_printerr("Failed to open file '.cache/img_cache', using default value(s).\n");
			gtk_picture_set_filename(GTK_PICTURE(img_wgt), DEFAULT_IMG_PATH);
			return;
		}
		char line[128];
		int i = 0;
		
		while (fgets(line, sizeof(line), imgf) != NULL) {
			line[strcspn(line, "\n")] = '\0';
			if (i > 1) break;
			switch(i) {
				case 0: gtk_picture_set_filename(GTK_PICTURE(img_wgt), line); break;
				
				default: break;
			}
			i++;
		}
		fclose(imgf);
	} else {
		g_printerr("Error loading '.cache/img_cache', using default value(s).\n");
		gtk_picture_set_filename(GTK_PICTURE(img_wgt), DEFAULT_IMG_PATH);
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
	g_string_assign(data->t5xxl_string, OPTIONAL_ITEMS);
	
	data->sampler_index = DEFAULT_SAMPLER;
	data->scheduler_index = DEFAULT_SCHEDULER;
	data->w_index = DEFAULT_SIZE;
	data->h_index = DEFAULT_SIZE;
	data->steps_value = DEFAULT_N_STEPS;
	data->batch_count_value = DEFAULT_BATCH_COUNT;
	data->sd_based_bool = ENABLED_OPT;
	data->cpu_bool = DISABLED_OPT;
	data->vt_bool = DISABLED_OPT;
	data->ram_offload_bool = DISABLED_OPT;
	data->k_clip_bool = DISABLED_OPT;
	data->k_cnet_bool = DISABLED_OPT;
	data->k_vae_bool = DISABLED_OPT;
	data->fa_bool = DISABLED_OPT;
	data->taesd_bool = DISABLED_OPT;
	data->update_cache_bool = ENABLED_OPT;
	data->verbose_bool = DISABLED_OPT;
	
	data->seed_value = DEFAULT_SEED;
	
	data->cfg_value = DEFAULT_CFG;
	data->denoise_value = DEFAULT_DENOISE;
	data->clip_skip_value = DEFAULT_CLIP_SKIP;
	data->up_repeat_value = DEFAULT_RP_UPSCALE;
}

void load_cache(gpointer user_data)
{

	char *cache_filename = ".cache/np_cache.ini";

	if (check_file_exists(cache_filename, 1) == 1) {
		AppStartData *data = user_data;
		
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
		
		char *t5xxl_str = ini_file_get_value(cache_filename, "t5xxl");
		if (t5xxl_str) {
			g_string_assign(data->t5xxl_string, t5xxl_str);
		} else {
			g_string_assign(data->t5xxl_string, OPTIONAL_ITEMS);
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
			char *endptr1;
			data->steps_value = strtod(n_steps_str, &endptr1);
		} else {
			data->steps_value = DEFAULT_N_STEPS;
		}
		
		char *batch_count_str = ini_file_get_value(cache_filename, "batch_count");
		if (batch_count_str) {
			char *endptr2;
			data->batch_count_value = strtod(batch_count_str, &endptr2);
		} else {
			data->batch_count_value = DEFAULT_BATCH_COUNT;
		}

		char *sd_based_bool_str = ini_file_get_value(cache_filename, "sd_based_bool");
		if (sd_based_bool_str) {
			sscanf(sd_based_bool_str, "%d", &data->sd_based_bool);
		} else {
			data->sd_based_bool = ENABLED_OPT;
		}
		
		char *cpu_mode_bool_str = ini_file_get_value(cache_filename, "cpu_mode_bool");
		if (cpu_mode_bool_str) {
			sscanf(cpu_mode_bool_str, "%d", &data->cpu_bool);
		} else {
			data->cpu_bool = DISABLED_OPT;
		}
		
		char *vae_tiling_bool_str = ini_file_get_value(cache_filename, "vae_tiling_bool");
		if (vae_tiling_bool_str) {
			sscanf(vae_tiling_bool_str, "%d", &data->vt_bool);
		} else {
			data->vt_bool = DISABLED_OPT;
		}
		
		char *ram_offload_bool_str = ini_file_get_value(cache_filename, "ram_offload_bool");
		if (ram_offload_bool_str) {
			sscanf(ram_offload_bool_str, "%d", &data->ram_offload_bool);
		} else {
			data->ram_offload_bool = DISABLED_OPT;
		}
		
		char *keep_clip_bool_str = ini_file_get_value(cache_filename, "keep_clip_bool");
		if (keep_clip_bool_str) {
			sscanf(keep_clip_bool_str, "%d", &data->k_clip_bool);
		} else {
			data->k_clip_bool = DISABLED_OPT;
		}
		
		char *keep_cnet_bool_str = ini_file_get_value(cache_filename, "keep_cnet_bool");
		if (keep_cnet_bool_str) {
			sscanf(keep_cnet_bool_str, "%d", &data->k_cnet_bool);
		} else {
			data->k_cnet_bool = DISABLED_OPT;
		}
		
		char *keep_vae_bool_str = ini_file_get_value(cache_filename, "keep_vae_bool");
		if (*keep_vae_bool_str) {
			sscanf(keep_vae_bool_str, "%d", &data->k_vae_bool);
		} else {
			data->k_vae_bool = DISABLED_OPT;
		}
		
		char *flash_attention_bool_str = ini_file_get_value(cache_filename, "flash_attention_bool");
		if (flash_attention_bool_str) {
			sscanf(flash_attention_bool_str, "%d", &data->fa_bool);
		} else {
			data->fa_bool = DISABLED_OPT;
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

		char *seed_str = ini_file_get_value(cache_filename, "seed");
		if (seed_str) {
			sscanf(seed_str, "%lld", &data->seed_value);
		} else {
			data->seed_value = DEFAULT_SEED;
		}
		
		char *cfg_scale_str = ini_file_get_value(cache_filename, "cfg_scale");
		if (cfg_scale_str) {
			char *endptr3;
			data->cfg_value = strtod(cfg_scale_str, &endptr3);
		} else {
			data->cfg_value = DEFAULT_CFG;
		}
		
		char *denoise_strength_str = ini_file_get_value(cache_filename, "denoise_strength");
		if (denoise_strength_str) {
			char *endptr4;
			data->denoise_value = strtod(denoise_strength_str, &endptr4);
		} else {
			data->denoise_value = DEFAULT_DENOISE;
		}
		
		char *clip_skip_str = ini_file_get_value(cache_filename, "clip_skip");
		if (clip_skip_str) {
			char *endptr5;
			data->clip_skip_value = strtod(clip_skip_str, &endptr5);
		} else {
			data->clip_skip_value = DEFAULT_CLIP_SKIP;
		}
		
		char *repeat_upscale_str = ini_file_get_value(cache_filename, "repeat_upscale");
		if (repeat_upscale_str) {
			char *endptr6;
			data->up_repeat_value = strtod(repeat_upscale_str, &endptr6);
		} else {
			data->up_repeat_value = DEFAULT_RP_UPSCALE;
		}
	} else {
		g_printerr("Error loading '.cache/np_cache.ini', using default value(s).\n");
		load_cache_fallback(user_data);
	}
}

void update_cache(GenerationData *data, gchar *sel_checkpoint, gchar *sel_vae, gchar *sel_cnet, gchar *sel_upscaler, gchar *sel_clip_l, gchar *sel_clip_g, gchar *sel_t5xxl, char *pp, char *np, char *img_num)
{
	FILE *pcf = fopen(".cache/pp_cache", "wb");
	FILE *ncf = fopen(".cache/np_cache", "wb");
	FILE *imgcf = fopen(".cache/img_cache", "wb");
	FILE *cf = fopen(".cache/np_cache.ini", "wb");
	if (pcf == NULL || ncf == NULL || imgcf == NULL || cf == NULL) {
		g_printerr("Error updating cache. If the error persists, try deleting the '.cache' directory.\n");
		return;
	}

	fprintf(pcf, "%s", pp);
	fclose(pcf);

	fprintf(ncf, "%s", np);
	fclose(ncf);

	fprintf(imgcf, "./outputs/IMG_%s.png\n", img_num);
	fclose(imgcf);

	fprintf(cf, "checkpoint=%s\n", sel_checkpoint);
	fprintf(cf, "vae=%s\n", sel_vae);
	fprintf(cf, "cnet=%s\n", sel_cnet);
	fprintf(cf, "upscaler=%s\n", sel_upscaler);
	fprintf(cf, "clip_l=%s\n", sel_clip_l);
	fprintf(cf, "clip_g=%s\n", sel_clip_g);
	fprintf(cf, "t5xxl=%s\n", sel_t5xxl);
	fprintf(cf, "sampler_index=%d\n", *data->sampler_index);
	fprintf(cf, "scheduler_index=%d\n", *data->scheduler_index);
	fprintf(cf, "img_width_index=%d\n", *data->w_index);
	fprintf(cf, "img_height_index=%d\n", *data->h_index);
	fprintf(cf, "n_steps=%.1f\n", *data->steps_value);
	fprintf(cf, "batch_count=%.1f\n", *data->batch_count_value);
	fprintf(cf, "sd_based_bool=%d\n", *data->sd_based_bool);
	fprintf(cf, "cpu_mode_bool=%d\n", *data->cpu_bool);
	fprintf(cf, "vae_tiling_bool=%d\n", *data->vt_bool);
	fprintf(cf, "ram_offload_bool=%d\n", *data->ram_offload_bool);
	fprintf(cf, "keep_clip_bool=%d\n", *data->k_clip_bool);
	fprintf(cf, "keep_cnet_bool=%d\n", *data->k_cnet_bool);
	fprintf(cf, "keep_vae_bool=%d\n", *data->k_vae_bool);
	fprintf(cf, "flash_attention_bool=%d\n", *data->fa_bool);
	fprintf(cf, "taesd_bool=%d\n", *data->taesd_bool);
	fprintf(cf, "update_cache_bool=%d\n", ENABLED_OPT);
	fprintf(cf, "verbose_bool=%d\n", *data->verbose_bool);
	fprintf(cf, "seed=%lld\n", *data->seed_value);
	fprintf(cf, "cfg_scale=%.1f\n", *data->cfg_value);
	fprintf(cf, "denoise_strength=%.2f\n", *data->denoise_value);
	fprintf(cf, "clip_skip=%.1f\n", *data->clip_skip_value);
	fprintf(cf, "repeat_upscale=%.1f\n", *data->up_repeat_value);
	fclose(cf);

	return;
}
