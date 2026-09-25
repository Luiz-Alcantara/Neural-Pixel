#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#ifdef _WIN32
	#include <windows.h>
#endif
#include "constants.h"
#include "handle_cache.h"
#include "file_utils.h"
#include "str_utils.h"

#define FULL_PATH_MAX 512

static void create_default_files(const char *file_path, GError **error)
{
	DIR* cd = opendir(".cache");
	if (cd == NULL) {
		g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Directory '.cache' does not exist or cannot be accessed.");
		return;
	}
	closedir(cd);

	if (strcmp(file_path, ".cache/pp_cache") == 0) {
		FILE *pcf = fopen(".cache/pp_cache", "wb");
		if (pcf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "File '.cache/pp_cache' does not exist or cannot be accessed.");
			return;
		}
		fprintf(pcf, "%s", POSITIVE_PROMPT);
		fclose(pcf);
		return;
	}

	if (strcmp(file_path, ".cache/np_cache") == 0) {
		FILE *ncf = fopen(".cache/np_cache", "wb");
		if (ncf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "File '.cache/np_cache' does not exist or cannot be accessed.");
			return;
		}
		fprintf(ncf, "%s", NEGATIVE_PROMPT);
		fclose(ncf);
		return;
	}

	if (strcmp(file_path, ".cache/detector_pp_cache") == 0) {
		FILE *dpcf = fopen(".cache/detector_pp_cache", "wb");
		if (dpcf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "File '.cache/detector_pp_cache' does not exist or cannot be accessed.");
			return;
		}
		fprintf(dpcf, "%s", DETECTOR_POSITIVE_PROMPT);
		fclose(dpcf);
		return;
	}

	if (strcmp(file_path, ".cache/detector_np_cache") == 0) {
		FILE *dncf = fopen(".cache/detector_np_cache", "wb");
		if (dncf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "File '.cache/detector_np_cache' does not exist or cannot be accessed.");
			return;
		}
		fprintf(dncf, "%s", DETECTOR_NEGATIVE_PROMPT);
		fclose(dncf);
		return;
	}

	if (strcmp(file_path, ".cache/np_cache.ini") == 0) {
		FILE *cf = fopen(".cache/np_cache.ini", "wb");
		if (cf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "File '.cache/np_cache.ini' does not exist or cannot be accessed.");
			return;
		}
		fprintf(cf, "last_image_path=%s\n", DEFAULT_IMG_PATH);
		fprintf(cf, "checkpoint=%s\n", OPTIONAL_ITEMS);
		fprintf(cf, "detector=%s\n", OPTIONAL_ITEMS);
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
		fprintf(cf, "detector_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "inpaint_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "sd_based_bool=%d\n", ENABLED_OPT);
		fprintf(cf, "llm_bool=%d\n", DISABLED_OPT);
		fprintf(cf, "hires_upscaler_index=%d\n", DISABLED_OPT);
		fprintf(cf, "detector_confidence_value=%.2f\n", DEFAULT_DETECTOR_CONFIDENCE);
		fprintf(cf, "detector_denoise_value=%.2f\n", DEFAULT_DETECTOR_DENOISE);
		fprintf(cf, "detector_inpaint_padding_value=%d.0\n", DEFAULT_DETECTOR_INPAINT_PADDING);
		fprintf(cf, "detector_inpaint_size_value=%d.0\n", DEFAULT_DETECTOR_INPAINT_SIZE);
		fprintf(cf, "detector_input_size_value=%d.0\n", DEFAULT_DETECTOR_INPUT_SIZE);
		fprintf(cf, "detector_mask_blur_value=%d.0\n", DEFAULT_DETECTOR_MASK_BLUR);
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
		fprintf(cf, "detector_runtime_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "detector_param_backend_index=%d\n", DEFAULT_BACKEND);
		fprintf(cf, "seed=%lld\n", DEFAULT_SEED);
		fprintf(cf, "cfg_scale=%.1f\n", DEFAULT_CFG);
		fprintf(cf, "cnet_strength=%.2f\n", DEFAULT_CNET_STRENGTH);
		fprintf(cf, "denoise_strength=%.2f\n", DEFAULT_DENOISE);
		fprintf(cf, "clip_skip=%d\n", DEFAULT_CLIP_SKIP);
		fprintf(cf, "repeat_upscale=%d.0\n", DEFAULT_RP_UPSCALE);
		fclose(cf);
		return;
	}

	if (strncmp(file_path, "./.lora_triggers/", 17) == 0) {
		DIR* ltd = opendir("./.lora_triggers");
		if (ltd == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Directory '.lora_triggers' does not exist or cannot be accessed.");
			return;
		}
		closedir(ltd);

		FILE *ltf = fopen(file_path, "wb");
		if (ltf == NULL) {
			g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "The lora trigger file cannot be created or open.");
			return;
		}
		fprintf(ltf, "");
		fclose(ltf);
		return;
	}
}

int is_file_empty(const char *fn)
{
	FILE *f = fopen(fn, "r");
	if (f == NULL) {
		fprintf(stderr, "Error opening file.\n");
		return -1;
	}
	fseek(f, 0, SEEK_END);
	long fs = ftell(f);
	fclose(f);
	if (fs == 0) {
		return 1;
	}
	return 0;
}


int is_directory(const char *path)
{
	struct stat path_stat;
	if (stat(path, &path_stat) != 0) return 0;
	return S_ISDIR(path_stat.st_mode);
}

int count_files(DIR* dir, const char * dir_path, const char* const* array)
{
	int nf = 0;
	if (dir != NULL && array == NULL) {
		struct dirent* entry;
		char full_path[FULL_PATH_MAX];
		while ((entry = readdir(dir)) != NULL) {
			snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
			if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && !is_directory(full_path)) {
				nf++;
			}
		}
		rewinddir(dir);
	}
	if (dir == NULL && array != NULL) {
		while (*array != NULL) {
			array++;
			nf++;
		}
	}
	return nf;
}

int count_output_files()
{
	const char *outputs_path = "./outputs";
	DIR* dir = opendir(outputs_path);
	if (dir == NULL) {
		return 0;
	}
	int nf = 0;

	struct dirent* entry;
	char full_path[FULL_PATH_MAX];
	while ((entry = readdir(dir)) != NULL) {
		snprintf(full_path, sizeof(full_path), "%s/%s", outputs_path, entry->d_name);
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && !is_directory(full_path)) {
			nf++;
		}
	}
	closedir(dir);
	return nf;
}

int check_file_exists(const char *filename, int is_text_file)
{
	/* Check if file exists */
	if (access(filename, F_OK) == 0) {
		/* If it's a text file, verify it's not empty and initialize if needed */
		if (is_text_file == 1) {
			/* pp_cache should NOT be empty */
			if ((is_file_empty(filename) == 1) && strcmp(filename, ".cache/pp_cache") == 0) {
				GError *err = NULL;
				create_default_files(filename, &err);
				if (err != NULL) {
					g_printerr("Error: %s\n", err->message);
					g_error_free(err);
					return 0;
				}
			}
		}
		return 1;  // File exists
	} else {
		/* If file doesn't exist but should be a text file, create it */
		if (is_text_file == 1) {
			GError *err = NULL;
			create_default_files(filename, &err);
			if (err != NULL) {
				g_printerr("Error: %s\n", err->message);
				g_error_free(err);
				return 0;
			}
			return 1;  // File was created
		}
		return 0;  // File doesn't exist
	}
}

int has_files(const char *directory)
{
	DIR *dir;
	struct dirent *ent;
	dir = opendir(directory);
	if (dir == NULL) {
		return 0;
	}
	while ((ent = readdir(dir)) != NULL) {
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
			closedir(dir);
			return 1;
		}
	}
	closedir(dir);
	return 0;
}

DIR* check_create_dir(const char* path)
{
	DIR* dir = opendir(path);
	if (dir == NULL) {
		#ifdef _WIN32
			if (mkdir(path) != 0) {
				fprintf(stderr, "Error creating directory.\n");
				return NULL;
			}

			DIR* ndir = opendir(path);
			return ndir;
		#else
			if (mkdir(path, 0777) != 0) {
				fprintf(stderr, "Error creating directory.\n");
				return NULL;
			}

			DIR* ndir = opendir(path);
			return ndir;
		#endif
	} else {
		return dir;
	}
}

int check_create_base_dirs() {
	DIR* cache_dir = opendir(CACHE_PATH);
	if (cache_dir == NULL) {
		#ifdef _WIN32
			if (mkdir(CACHE_PATH) != 0) {
				fprintf(stderr, "Error creating required \".cache\" directory.\n");
				return 1;
			}
		#else
			if (mkdir(CACHE_PATH, 0777) != 0) {
				fprintf(stderr, "Error creating required \".cache\" directory.\n");
				return 1;
			}
		#endif
	}
	closedir(cache_dir);

	DIR* lora_triggers_dir = opendir(LORA_TRIGGERS_PATH);
	if (lora_triggers_dir == NULL) {
		#ifdef _WIN32
			if (mkdir(LORA_TRIGGERS_PATH) != 0) {
				fprintf(stderr, "Error creating required \".lora_triggers\" directory.\n");
				return 1;
			}
		#else
			if (mkdir(LORA_TRIGGERS_PATH, 0777) != 0) {
				fprintf(stderr, "Error creating required \".lora_triggers\" directory.\n");
				return 1;
			}
		#endif
	}
	closedir(lora_triggers_dir);

	DIR* models_dir = opendir(MODELS_PATH);
	if (models_dir == NULL) {
		#ifdef _WIN32
			if (mkdir(MODELS_PATH) != 0) {
				fprintf(stderr, "Error creating required \"models\" directory.\n");
				return 1;
			}
		#else
			if (mkdir(MODELS_PATH, 0777) != 0) {
				fprintf(stderr, "Error creating required \"models\" directory.\n");
				return 1;
			}
		#endif
	}
	closedir(models_dir);

	DIR* outputs_dir = opendir(OUTPUTS_PATH);
	if (outputs_dir == NULL) {
		#ifdef _WIN32
			if (mkdir(OUTPUTS_PATH) != 0) {
				fprintf(stderr, "Error creating required \"outputs\" directory.\n");
				return 1;
			}
		#else
			if (mkdir(OUTPUTS_PATH, 0777) != 0) {
				fprintf(stderr, "Error creating required \"outputs\" directory.\n");
				return 1;
			}
		#endif
	}
	closedir(outputs_dir);
	return 0;
}

GtkStringList* get_files(const char* path, GError **error)
{
	DIR* dir = check_create_dir(path);
	if (dir == NULL) {
		g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Directory '%s' does not exist or cannot be accessed.", path);
		return NULL;
	}

	int nf = 0;
	nf = count_files(dir, path, NULL) + 1;
	GtkStringList *files = gtk_string_list_new(NULL);
	char** sort_files = malloc(sizeof(char*) * (nf + 1));

	if (sort_files == NULL) {
		closedir(dir);
		g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "Memory allocation failed.", path);
		return NULL;
	}

	int i = 0;
	struct dirent* entry;
	char full_path[FULL_PATH_MAX];

	sort_files[0] = strdup("None");
	while ((entry = readdir(dir)) != NULL) {
		snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..") && !is_directory(full_path)) {
			sort_files[i + 1] = strdup(entry->d_name); 
			i++;
		}
	}

	qsort(sort_files + 1, nf - 1, sizeof(const char *), compare_strings);

	for (int x = 0; x < nf; x++) {
		gtk_string_list_append(files, sort_files[x]);
		free(sort_files[x]);
	}

	free(sort_files);
	closedir(dir);

	return files;
}

void get_png_files(GPtrArray *image_files)
{
	g_ptr_array_set_size(image_files, 0);

	gchar *current_dir = g_get_current_dir();
	gchar *full_path = g_build_filename(current_dir, "outputs", NULL);
	g_free(current_dir);

	GDir *dir = g_dir_open(full_path, 0, NULL);
	g_free(full_path);

	if (!dir) {
		g_printerr("Could not open directory 'outputs'.");
		return;
	}

	const gchar *filename;
	while ((filename = g_dir_read_name(dir))) {
		if (g_str_has_suffix(filename, ".png") || g_str_has_suffix(filename, ".PNG")) {
			gchar *full_file_path = g_build_filename("outputs", filename, NULL);
			g_ptr_array_add(image_files, full_file_path);
		}
	}

	g_dir_close(dir);

	g_ptr_array_sort(image_files, (GCompareFunc)compare_strings);

	g_print("Found %u PNG files.\n", image_files->len);
}

char* get_unique_filepath(char *path)
{
	if (!path) return NULL;
	if (!check_file_exists(path, 0)) return path;

	const char *dot = g_strrstr(path, ".");
	size_t base_len = dot ? (size_t)(dot - path) : strlen(path);
	const char *extension = dot ? dot : "";
	char *base_path = g_strndup(path, base_len);
	char *new_path = NULL;
	int suffix = 2;

	while (1) {
		new_path = g_strdup_printf("%s_%d%s", base_path, suffix, extension);
		if (!check_file_exists(new_path, 0)) break;
		g_free(new_path);
		suffix++;
	}

	g_free(base_path);
	g_free(path);
	return new_path;
}

void set_current_image_index(char *img_str, GString *img_index_string, GPtrArray *image_files, gint *current_image_index, int total_time)
{
	gsize img_count = image_files->len;

	if (img_count > 0) {
		guint new_index;

		gboolean found = g_ptr_array_find_with_equal_func(
		image_files,
		img_str,
		string_equal,
		&new_index
		);

		if (found == TRUE && new_index < img_count) {
			*current_image_index = new_index;
		} else {
			g_printerr("Could not load last generated image, using default value(s).\n");
			*current_image_index = img_count - 1;
		}
		g_string_erase(img_index_string, 0, -1);
		if (total_time < 0) {
			g_string_append_printf(img_index_string, "(%d / %d) %s", *current_image_index + 1, img_count, img_str + 8);
		} else {
			int minutes = total_time / 60;
			int seconds = total_time % 60;
			g_string_append_printf(img_index_string, "(%d / %d) Last generation took: %dm %ds.",
			*current_image_index + 1, img_count, minutes, seconds);
		}
	} else {
		g_printerr("No images in 'outputs' directory.\n");
		*current_image_index = 0;
		g_string_erase(img_index_string, 0, -1);
		g_string_append(img_index_string, "0 / 0");
	}
}
