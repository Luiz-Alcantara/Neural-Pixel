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

int check_file_exists(char *filename, int is_text_file)
{
	/* Check if file exists */
	if (access(filename, F_OK) == 0) {
		/* If it's a text file, verify it's not empty and initialize if needed */
		if (is_text_file == 1) {
			if (is_file_empty(filename) == 1 && strcmp(filename, ".cache/np_cache") != 0) {
				GError *err = NULL;
				create_cache(filename, &err);
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
			create_cache(filename, &err);
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

void set_current_image_index(char *img_str, GString *img_index_string, GPtrArray *image_files, gint *current_image_index, int *total_time)
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
		if (total_time == NULL) {
			g_string_append_printf(img_index_string, "(%d / %d) File: '%s'", *current_image_index + 1, img_count, img_str + 8);
		} else {
			int minutes = *total_time / 60;
			int seconds = *total_time % 60;
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
