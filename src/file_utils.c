#include <glib/gstdio.h>
#include <gtk/gtk.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "file_utils.h"
#include "str_utils.h"

static gboolean write_default_text_file(const char *file_path, const char *content, gboolean append_newline, GError **error)
{
	FILE *f = g_fopen(file_path, "wb");
	if (f == NULL) {
		int errsv = errno;
		g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(errsv), "Failed to create '%s': %s.", file_path, g_strerror(errsv));
		return FALSE;
	}

	const char *text = content ? content : "";
	int write_res = append_newline ? fprintf(f, "%s\n", text) : fprintf(f, "%s", text);
	int write_err = (write_res < 0) ? errno : 0;
	int close_res = fclose(f);

	if (write_res < 0 || close_res != 0) {
		int errsv = (write_res < 0) ? write_err : errno;
		g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(errsv), "Failed to write '%s': %s.", file_path, g_strerror(errsv));
		return FALSE;
	}

	return TRUE;
}

static int write_ini_double(FILE *f, const char *key, const char *fmt, double value)
{
	char buf[G_ASCII_DTOSTR_BUF_SIZE];
	g_ascii_formatd(buf, sizeof(buf), fmt, value);
	return fprintf(f, "%s=%s\n", key, buf);
}

static gboolean create_default_files(const char *file_path, GError **error)
{
	if (file_path == NULL || file_path[0] == '\0') {
		g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Invalid file path.");
		return FALSE;
	}

	size_t triggers_len = strlen(LORA_TRIGGERS_PATH);
	if (strncmp(file_path, LORA_TRIGGERS_PATH, triggers_len) == 0 &&
	    (file_path[triggers_len] == '/' || file_path[triggers_len] == G_DIR_SEPARATOR) &&
	    file_path[triggers_len + 1] != '\0') {
		GDir *ltd = g_dir_open(LORA_TRIGGERS_PATH, 0, error);
		if (ltd == NULL) return FALSE;
		g_dir_close(ltd);

		return write_default_text_file(file_path, "", FALSE, error);
	}

	if (g_str_has_prefix(file_path, ".cache/")) {
		GDir *cd = g_dir_open(".cache", 0, error);
		if (cd == NULL) return FALSE;
		g_dir_close(cd);

		if (strcmp(file_path, FAVORITES_PATH) == 0) {
			return write_default_text_file(FAVORITES_PATH, DEFAULT_IMG_PATH, TRUE, error);
		}

		if (strcmp(file_path, ".cache/pp_cache") == 0) {
			return write_default_text_file(".cache/pp_cache", POSITIVE_PROMPT, FALSE, error);
		}

		if (strcmp(file_path, ".cache/np_cache") == 0) {
			return write_default_text_file(".cache/np_cache", NEGATIVE_PROMPT, FALSE, error);
		}

		if (strcmp(file_path, ".cache/detector_pp_cache") == 0) {
			return write_default_text_file(".cache/detector_pp_cache", DETECTOR_POSITIVE_PROMPT, FALSE, error);
		}

		if (strcmp(file_path, ".cache/detector_np_cache") == 0) {
			return write_default_text_file(".cache/detector_np_cache", DETECTOR_NEGATIVE_PROMPT, FALSE, error);
		}

		if (strcmp(file_path, ".cache/np_cache.ini") == 0) {
			FILE *cf = g_fopen(".cache/np_cache.ini", "wb");
			if (cf == NULL) {
				int errsv = errno;
				g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(errsv), "Failed to create '%s': %s.", file_path, g_strerror(errsv));
				return FALSE;
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
			fprintf(cf, "llm_vision=%s\n", OPTIONAL_ITEMS);
			fprintf(cf, "tokenizer=%s\n", OPTIONAL_ITEMS);
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
			write_ini_double(cf, "detector_confidence_value", "%.2f", DEFAULT_DETECTOR_CONFIDENCE);
			write_ini_double(cf, "detector_denoise_value", "%.2f", DEFAULT_DETECTOR_DENOISE);
			fprintf(cf, "detector_inpaint_padding_value=%d.0\n", DEFAULT_DETECTOR_INPAINT_PADDING);
			fprintf(cf, "detector_inpaint_size_value=%d.0\n", DEFAULT_DETECTOR_INPAINT_SIZE);
			fprintf(cf, "detector_input_size_value=%d.0\n", DEFAULT_DETECTOR_INPUT_SIZE);
			fprintf(cf, "detector_mask_blur_value=%d.0\n", DEFAULT_DETECTOR_MASK_BLUR);
			write_ini_double(cf, "hires_scale_value", "%.2f", DEFAULT_HIRES_SCALE);
			fprintf(cf, "hires_steps_value=%d.0\n", DEFAULT_HIRES_STEPS);
			write_ini_double(cf, "hires_denoise_value", "%.1f", DEFAULT_HIRES_DENOISE_STR);
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
			write_ini_double(cf, "cfg_scale", "%.1f", DEFAULT_CFG);
			write_ini_double(cf, "cnet_strength", "%.2f", DEFAULT_CNET_STRENGTH);
			write_ini_double(cf, "denoise_strength", "%.2f", DEFAULT_DENOISE);
			write_ini_double(cf, "flow_shift", "%.1f", DEFAULT_FLOW_SHIFT);
			fprintf(cf, "clip_skip=%d\n", DEFAULT_CLIP_SKIP);
			fprintf(cf, "repeat_upscale=%d.0\n", DEFAULT_RP_UPSCALE);

			gboolean write_failed = (ferror(cf) != 0);
			int write_err = write_failed ? errno : 0;
			int close_res = fclose(cf);

			if (write_failed || close_res != 0) {
				int errsv = write_failed ? write_err : errno;
				g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(errsv), "Failed to write '%s': %s.", file_path, g_strerror(errsv));
				return FALSE;
			}

			return TRUE;
		}
	}

	g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_NOENT, "No default template defined for '%s'.", file_path);
	return FALSE;
}

static int is_file_empty(const char *fn)
{
	if (fn == NULL) return -1;

	GStatBuf st;
	if (g_stat(fn, &st) != 0) {
		g_printerr("Error checking file '%s': %s.\n", fn, g_strerror(errno));
		return -1;
	}

	return (st.st_size == 0) ? 1 : 0;
}

int check_file_exists(const char *filename, int is_text_file)
{
	if (filename == NULL || filename[0] == '\0') return 0;

	if (g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
		if (is_text_file == 1 && strcmp(filename, ".cache/pp_cache") == 0 && is_file_empty(filename) == 1) {
			GError *err = NULL;
			if (!create_default_files(filename, &err)) {
				if (err != NULL) {
					g_printerr("Error: %s\n", err->message);
					g_error_free(err);
				}
				return 0;
			}
		}
		return 1;
	}

	if (is_text_file == 1) {
		GError *err = NULL;
		if (!create_default_files(filename, &err)) {
			if (err != NULL) {
				g_printerr("Error: %s\n", err->message);
				g_error_free(err);
			}
			return 0;
		}
		return 1;
	}

	return 0;
}

static GDir *check_create_dir(const char *path, GError **error)
{
	GError *local_err = NULL;
	GDir *dir = g_dir_open(path, 0, &local_err);

	if (dir == NULL) {
		if (g_error_matches(local_err, G_FILE_ERROR, G_FILE_ERROR_NOENT)) {
			g_clear_error(&local_err);
			if (g_mkdir_with_parents(path, 0755) != 0) {
				int errsv = errno;
				g_set_error(error, G_FILE_ERROR, g_file_error_from_errno(errsv), "Error creating directory '%s': %s.", path, g_strerror(errsv));
				return NULL;
			}

			dir = g_dir_open(path, 0, error);
			if (dir == NULL) return NULL;
		} else {
			g_propagate_error(error, local_err);
			return NULL;
		}
	}

	return dir;
}

int check_create_base_dirs(void)
{
	const char *base_dirs[] = {
		CACHE_PATH,
		LORA_TRIGGERS_PATH,
		MODELS_PATH,
		OUTPUTS_PATH
	};

	for (size_t i = 0; i < G_N_ELEMENTS(base_dirs); i++) {
		if (g_mkdir_with_parents(base_dirs[i], 0700) != 0) {
			g_printerr("Failed to create directory '%s': %s.\n", base_dirs[i], g_strerror(errno));
			return 1;
		}
	}

	return 0;
}

GtkStringList *get_files(const char *path, GError **error)
{
	if (path == NULL || path[0] == '\0') {
		g_set_error(error, G_FILE_ERROR, G_FILE_ERROR_INVAL, "Invalid directory path.");
		return NULL;
	}

	GDir *dir = check_create_dir(path, error);
	if (dir == NULL) return NULL;

	GPtrArray *entries = g_ptr_array_new_with_free_func(g_free);
	const char *filename;

	while ((filename = g_dir_read_name(dir)) != NULL) {
		char *full_path = g_build_filename(path, filename, NULL);
		if (g_file_test(full_path, G_FILE_TEST_IS_REGULAR)) {
			g_ptr_array_add(entries, g_strdup(filename));
		}
		g_free(full_path);
	}

	g_dir_close(dir);

	if (entries->len > 1) {
		g_ptr_array_sort(entries, (GCompareFunc)compare_strings);
	}

	const char **items = g_new(const char *, (gsize)entries->len + 2);
	items[0] = OPTIONAL_ITEMS;
	for (guint i = 0; i < entries->len; i++) {
		items[i + 1] = g_ptr_array_index(entries, i);
	}
	items[entries->len + 1] = NULL;

	GtkStringList *files = gtk_string_list_new(items);

	g_free(items);
	g_ptr_array_unref(entries);

	return files;
}

void get_png_files(GPtrArray *image_files)
{
	if (image_files == NULL) return;

	g_ptr_array_set_size(image_files, 0);

	GError *error = NULL;
	GDir *dir = g_dir_open(OUTPUTS_PATH, 0, &error);

	if (dir == NULL) {
		g_printerr("Could not open directory '%s': %s\n", OUTPUTS_PATH, error ? error->message : "Unknown error");
		g_clear_error(&error);
		return;
	}

	const gchar *filename;
	while ((filename = g_dir_read_name(dir)) != NULL) {
		size_t name_len = strlen(filename);
		if (name_len > 4 && g_ascii_strcasecmp(filename + name_len - 4, ".png") == 0) {
			gchar *full_file_path = g_build_filename("outputs", filename, NULL);
			if (g_file_test(full_file_path, G_FILE_TEST_IS_REGULAR)) {
				g_ptr_array_add(image_files, full_file_path);
			} else {
				g_free(full_file_path);
			}
		}
	}

	g_dir_close(dir);

	if (image_files->len > 1) {
		g_ptr_array_sort(image_files, (GCompareFunc)compare_strings);
	}

	g_print("Found %u PNG files.\n", image_files->len);
}

char *get_unique_filepath(char *path)
{
	if (path == NULL) return NULL;
	if (!g_file_test(path, G_FILE_TEST_EXISTS)) return path;

	const char *last_slash = strrchr(path, '/');
	const char *last_backslash = strrchr(path, '\\');
	const char *last_sep = last_slash;
	if (last_backslash != NULL && (last_sep == NULL || last_backslash > last_sep)) {
		last_sep = last_backslash;
	}

	const char *filename_start = last_sep ? last_sep + 1 : path;
	const char *dot = strrchr(filename_start, '.');
	if (dot == filename_start) dot = NULL;

	size_t base_len = dot ? (size_t)(dot - path) : strlen(path);
	const char *extension = dot ? dot : "";
	char *base_path = g_strndup(path, base_len);
	char *new_path = NULL;

	for (int suffix = 2; ; suffix++) {
		char *candidate = g_strdup_printf("%s_%d%s", base_path, suffix, extension);
		if (!g_file_test(candidate, G_FILE_TEST_EXISTS)) {
			new_path = candidate;
			break;
		}
		g_free(candidate);
		if (suffix == INT_MAX) break;
	}

	g_free(base_path);
	if (new_path == NULL) {
		g_printerr("Failed to find a unique filepath for '%s'.\n", path);
		return path;
	}

	g_free(path);
	return new_path;
}

void set_current_image_index(const char *img_str, GString *img_index_string, GPtrArray *image_files, gint *current_image_index, int total_time)
{
	if (img_index_string == NULL || image_files == NULL || current_image_index == NULL) return;

	guint img_count = image_files->len;

	if (img_count > 0) {
		guint new_index = 0;
		gboolean found = FALSE;

		if (img_str != NULL) {
			found = g_ptr_array_find_with_equal_func(
				image_files,
				img_str,
				string_equal,
				&new_index
			);
		}

		if (found && new_index < img_count) {
			*current_image_index = (gint)new_index;
		} else {
			g_printerr("Could not load last generated image, using default value(s).\n");
			*current_image_index = (gint)(img_count - 1);
		}

		if (total_time < 0) {
			const char *selected_path = g_ptr_array_index(image_files, *current_image_index);
			gchar *basename = selected_path ? g_path_get_basename(selected_path) : g_strdup("");
			g_string_printf(img_index_string, "(%d / %u) %s", *current_image_index + 1, img_count, basename);
			g_free(basename);
		} else {
			int minutes = total_time / 60;
			int seconds = total_time % 60;
			g_string_printf(img_index_string, "(%d / %u) Last generation took: %dm %ds.",
				*current_image_index + 1, img_count, minutes, seconds);
		}
	} else {
		g_printerr("No images in 'outputs' directory.\n");
		*current_image_index = 0;
		g_string_assign(img_index_string, "0 / 0");
	}
}
