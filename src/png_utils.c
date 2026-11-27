#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <png.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "file_utils.h"
#include "png_utils.h"
#include "str_utils.h"
#include "structs.h"
#include "toast_notification.h"
#include "widgets_cb.h"

#define MAX_METADATA_PROMPT_LENGTH 2048
#define MAX_PROPERTY_LENGTH 512

typedef enum {
	METADATA_TASK_LOAD_WIDGETS = 0,
	METADATA_TASK_COPY_ALL = 1,
	METADATA_TASK_COPY_POSITIVE = 2,
	METADATA_TASK_COPY_NEGATIVE = 3
} MetadataTask;

#if GTK_CHECK_VERSION(4, 10, 0)
typedef struct {
	LoadPNGData *data;
	GCancellable *cancellable;
	GtkWidget *win;
} OpenPNGDialogContext;

typedef struct {
	LoadImg2ImgData *data;
	GCancellable *cancellable;
	GtkWidget *win;
} OpenImg2ImgDialogContext;
#endif

int get_png_dimensions(const char *filename, uint32_t *w, uint32_t *h)
{
	if (!filename || filename[0] == '\0' || !w || !h) return -1;

	const uint8_t png_signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
	uint8_t header[24];

	FILE *f = g_fopen(filename, "rb");
	if (!f) {
		g_printerr("Could not open png file: '%s'\n", filename);
		return -1;
	}

	size_t read_bytes = fread(header, 1, sizeof(header), f);
	fclose(f);

	if (read_bytes != sizeof(header)) {
		g_printerr("Failed to read png header from file: '%s'\n", filename);
		return -1;
	}

	if (memcmp(header, png_signature, sizeof(png_signature)) != 0) {
		g_printerr("The file is not a png file.\n");
		return -1;
	}

	uint32_t ihdr_len = ((uint32_t)header[8] << 24) |
		((uint32_t)header[9] << 16) |
		((uint32_t)header[10] << 8) |
		(uint32_t)header[11];

	if (ihdr_len != 13 || memcmp(header + 12, "IHDR", 4) != 0) {
		g_printerr("Invalid PNG IHDR chunk in file: '%s'\n", filename);
		return -1;
	}

	uint32_t width = ((uint32_t)header[16] << 24) |
		((uint32_t)header[17] << 16) |
		((uint32_t)header[18] << 8) |
		(uint32_t)header[19];
	uint32_t height = ((uint32_t)header[20] << 24) |
		((uint32_t)header[21] << 16) |
		((uint32_t)header[22] << 8) |
		(uint32_t)header[23];

	if (width == 0 || height == 0 || width > (uint32_t)INT_MAX || height > (uint32_t)INT_MAX) {
		g_printerr("Invalid PNG dimensions (%ux%u) in file: '%s'\n", width, height, filename);
		return -1;
	}

	*w = width;
	*h = height;

	printf("Ref image dimensions: %ux%u\n", *w, *h);

	return 0;
}

GdkTexture *get_texture_from_pixbuf(GdkPixbuf *pixbuf)
{
	if (!GDK_IS_PIXBUF(pixbuf)) return NULL;

	int width = gdk_pixbuf_get_width(pixbuf);
	int height = gdk_pixbuf_get_height(pixbuf);
	int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
	int bits_per_sample = gdk_pixbuf_get_bits_per_sample(pixbuf);
	gboolean has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);

	if (width <= 0 || height <= 0 || rowstride <= 0 || bits_per_sample != 8 ||
	gdk_pixbuf_get_colorspace(pixbuf) != GDK_COLORSPACE_RGB ||
	n_channels != (has_alpha ? 4 : 3)) {
		return NULL;
	}

	gsize row_bytes = 0;
	if (!g_size_checked_mul(&row_bytes, (gsize)width, (gsize)n_channels) || (gsize)rowstride < row_bytes) {
		return NULL;
	}

	gsize rows_offset = 0;
	gsize required_size = 0;
	if (!g_size_checked_mul(&rows_offset, (gsize)(height - 1), (gsize)rowstride) ||
	!g_size_checked_add(&required_size, rows_offset, row_bytes)) {
		return NULL;
	}

	GBytes *bytes = gdk_pixbuf_read_pixel_bytes(pixbuf);
	if (!bytes) return NULL;

	if (g_bytes_get_size(bytes) < required_size) {
		g_bytes_unref(bytes);
		return NULL;
	}

	GdkTexture *texture = gdk_memory_texture_new(
		width,
		height,
		has_alpha ? GDK_MEMORY_R8G8B8A8 : GDK_MEMORY_R8G8B8,
		bytes,
		(gsize)rowstride
	);

	g_bytes_unref(bytes);
	return texture;
}

static void copy_utf8_bounded(const char *start, size_t len, char *dest, size_t dest_size)
{
	if (!dest || dest_size == 0) return;
	if (!start) {
		dest[0] = '\0';
		return;
	}

	while (len > 0 && start[len - 1] == '\r') len--;

	if (len >= dest_size) {
		len = dest_size - 1;
		while (len > 0 && (((unsigned char)start[len]) & 0xC0) == 0x80) len--;
	}

	memcpy(dest, start, len);
	dest[len] = '\0';
}

static gboolean extract_delimited_value(const char *haystack, const char *key, const char *delims, char *out, size_t out_size)
{
	if (!haystack || !key || !delims || !out || out_size == 0) return FALSE;

	const char *ptr = strstr(haystack, key);
	if (!ptr) return FALSE;

	const char *start = ptr + strlen(key);
	size_t len = strcspn(start, delims);
	if (len == 0) return FALSE;

	copy_utf8_bounded(start, len, out, out_size);
	return out[0] != '\0';
}

static gboolean extract_quoted_value(const char *haystack, const char *key, char *out, size_t out_size)
{
	if (!haystack || !key || !out || out_size == 0) return FALSE;

	const char *ptr = strstr(haystack, key);
	if (!ptr) return FALSE;

	const char *start = ptr + strlen(key);
	size_t len = strcspn(start, "\"\r\n");
	if (len == 0 || start[len] != '"') return FALSE;

	copy_utf8_bounded(start, len, out, out_size);
	return out[0] != '\0';
}

static gboolean parse_int_value(const char *str, int *out_val)
{
	if (!str || !out_val) return FALSE;

	char *endptr = NULL;
	errno = 0;
	gint64 val = g_ascii_strtoll(str, &endptr, 10);
	if (endptr == str || errno == ERANGE || val < INT_MIN || val > INT_MAX) return FALSE;

	*out_val = (int)val;
	return TRUE;
}

static gboolean parse_int64_value(const char *str, gint64 *out_val)
{
	if (!str || !out_val) return FALSE;

	char *endptr = NULL;
	errno = 0;
	gint64 val = g_ascii_strtoll(str, &endptr, 10);
	if (endptr == str || errno == ERANGE) return FALSE;

	*out_val = val;
	return TRUE;
}

static gboolean parse_double_value(const char *str, double *out_val)
{
	if (!str || !out_val) return FALSE;

	char *endptr = NULL;
	errno = 0;
	double val = g_ascii_strtod(str, &endptr);
	if (endptr == str || errno == ERANGE || !isfinite(val)) return FALSE;

	*out_val = val;
	return TRUE;
}

static void append_formatted_double(GString *dst, const char *label, const char *fmt, double value)
{
	char num_buf[G_ASCII_DTOSTR_BUF_SIZE];
	g_ascii_formatd(num_buf, sizeof(num_buf), fmt, value);
	g_string_append_printf(dst, "%s: %s\n", label, num_buf);
}

static void set_dropdown_from_const_list(GtkWidget *dd, const char *const *list, const char *value)
{
	if (!dd || !list || !value) return;
	int idx = check_list_contains_item(list, value);
	if (idx >= 0) gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), (guint)idx);
}

static void set_dropdown_from_gtk_list(GtkWidget *dd, const char *value)
{
	if (!dd || !value) return;
	GListModel *model = gtk_drop_down_get_model(GTK_DROP_DOWN(dd));
	if (GTK_IS_STRING_LIST(model)) {
		int idx = check_gtk_list_contains_item(GTK_STRING_LIST(model), value);
		if (idx >= 0) gtk_drop_down_set_selected(GTK_DROP_DOWN(dd), (guint)idx);
	}
}

static void set_clipboard_text_safe(GtkWidget *win, const char *text)
{
	if (!win || !text) return;
	GdkClipboard *clipboard = gtk_widget_get_clipboard(win);
	if (clipboard != NULL) {
		gdk_clipboard_set_text(clipboard, text);
	}
}

static GString *read_png_text_chunks(const char *path, const char **err_msg)
{
	FILE *fp = g_fopen(path, "rb");
	if (!fp) {
		fprintf(stderr, "File opening failed.\n");
		if (err_msg) *err_msg = "File opening failed";
		return NULL;
	}

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fprintf(stderr, "png_create_read_struct failed.\n");
		fclose(fp);
		if (err_msg) *err_msg = "png_create_read_struct failed";
		return NULL;
	}

	png_infop info = png_create_info_struct(png);
	if (!info) {
		fprintf(stderr, "png_create_info_struct failed.\n");
		png_destroy_read_struct(&png, NULL, NULL);
		fclose(fp);
		if (err_msg) *err_msg = "png_create_info_struct failed";
		return NULL;
	}

	if (setjmp(png_jmpbuf(png))) {
		fprintf(stderr, "Error during PNG initialization.\n");
		png_destroy_read_struct(&png, &info, NULL);
		fclose(fp);
		if (err_msg) *err_msg = "Error during PNG initialization";
		return NULL;
	}

	png_init_io(png, fp);
	png_read_info(png, info);

	png_textp text = NULL;
	int num_text = 0;
	png_get_text(png, info, &text, &num_text);

	GString *raw = g_string_new(NULL);
	if (text != NULL && num_text > 0) {
		for (int i = 0; i < num_text; i++) {
			const char *key = text[i].key ? text[i].key : "";
			const char *val = text[i].text ? text[i].text : "";
			g_string_append_printf(raw, "%s %s\n", key, val);
		}
	}

	png_destroy_read_struct(&png, &info, NULL);
	fclose(fp);

	if (!g_utf8_validate(raw->str, (gssize)raw->len, NULL)) {
		gchar *valid = g_utf8_make_valid(raw->str, (gssize)raw->len);
		g_string_assign(raw, valid);
		g_free(valid);
	}

	return raw;
}

static void set_png_metadata(const gchar *path, MetadataTask copy_info, LoadPNGData *data)
{
	if (!path || !data) return;

	gboolean set_widgets = (copy_info == METADATA_TASK_LOAD_WIDGETS);
	gboolean copy_all = (copy_info == METADATA_TASK_COPY_ALL);
	gboolean copy_pp = (copy_info == METADATA_TASK_COPY_POSITIVE || copy_all);
	gboolean copy_np = (copy_info == METADATA_TASK_COPY_NEGATIVE || copy_all);

	const char *err_msg = NULL;
	GString *l1 = read_png_text_chunks(path, &err_msg);
	if (!l1) {
		if (data->win) {
			show_simple_message(data->win, "Error loading PNG info", (char *)(err_msg ? err_msg : "Failed to read PNG metadata"), 1);
		}
		return;
	}

	GString *l2 = g_string_new(NULL);

	int n_err = 0;
	int n_loaded = 0;
	gboolean copied_pp = FALSE;
	gboolean copied_np = FALSE;
	const char *ptr = NULL;

	gboolean is_new_format = (
		strstr(l1->str, "\"schema\":\"sdcpp.generation/") != NULL ||
		strstr(l1->str, "Schedule type: ") != NULL ||
		strstr(l1->str, "SDCPP Schedule: ") != NULL ||
		strstr(l1->str, "SDCPP Sampler: ") != NULL
	);

	if (set_widgets || copy_pp) {
		const char *params_colon = strstr(l1->str, "parameters:");
		const char *params_space = strstr(l1->str, "parameters ");
		if (params_colon && params_space) ptr = (params_colon < params_space) ? params_colon : params_space;
		else ptr = params_colon ? params_colon : params_space;

		if (ptr) {
			const char *start = ptr + strlen("parameters:");
			while (*start == ' ') start++;
			const char *neg_end = strstr(start, "\nNegative prompt:");
			const char *steps_end = strstr(start, "\nSteps:");
			const char *end = NULL;
			if (neg_end && steps_end) end = (neg_end < steps_end) ? neg_end : steps_end;
			else end = neg_end ? neg_end : steps_end;

			if (end && start < end) {
				char *p1 = g_strndup(start, (gsize)(end - start));

				if (set_widgets && data->pos_tb) {
					clear_adapter_cards(data->box_model_adapters_cards);
					char *p2 = extract_adapters_from_prompt(GTK_BOX(data->box_model_adapters_cards), 1, 0, p1);
					char *pprompt = extract_adapters_from_prompt(GTK_BOX(data->box_model_adapters_cards), 0, 0, p2);
					gtk_text_buffer_set_text(data->pos_tb, pprompt, -1);

					n_loaded++;
					g_free(p2);
					g_free(pprompt);
				} else if (copy_all) {
					g_string_append_printf(l2, "Prompt: %s\n", p1);
				} else if (copy_pp) {
					set_clipboard_text_safe(data->win, p1);
					copied_pp = TRUE;
				}
				g_free(p1);
			} else {
				fprintf(stderr, "Failed to parse positive prompt data.\n");
				n_err++;
			}
		} else {
			fprintf(stderr, "Failed to parse positive prompt data.\n");
			n_err++;
		}
	}

	if (set_widgets || copy_np) {
		ptr = strstr(l1->str, "Negative prompt:");
		if (ptr) {
			const char *start = ptr + strlen("Negative prompt:");
			while (*start == ' ') start++;
			const char *end = strstr(start, "\nSteps:");
			if (end && start <= end) {
				char *p1 = g_strndup(start, (gsize)(end - start));

				if (set_widgets && data->neg_tb) {
					char *nprompt = extract_adapters_from_prompt(GTK_BOX(data->box_model_adapters_cards), 0, 1, p1);
					gtk_text_buffer_set_text(data->neg_tb, nprompt, -1);

					n_loaded++;
					g_free(nprompt);
				} else if (copy_all) {
					if (p1[0] != '\0') {
						g_string_append_printf(l2, "Negative Prompt: %s\n", p1);
					}
				} else if (copy_np && p1[0] != '\0') {
					set_clipboard_text_safe(data->win, p1);
					copied_np = TRUE;
				}
				g_free(p1);
			} else {
				fprintf(stderr, "Failed to parse negative prompt data.\n");
				n_err++;
			}
		} else {
			if (n_err > 0) {
				fprintf(stderr, "Failed to parse negative prompt data.\n");
				n_err++;
			} else if (set_widgets && data->neg_tb) {
				gtk_text_buffer_set_text(data->neg_tb, "", -1);
			}
		}
	}

	if (set_widgets || copy_all) {
		ptr = strstr(l1->str, "Steps: ");
		if (ptr) {
			int steps_int = 0;
			if (parse_int_value(ptr + strlen("Steps: "), &steps_int)) {
				if (set_widgets && data->steps_spin) {
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->steps_spin), steps_int);
					n_loaded++;
				} else if (copy_all) {
					g_string_append_printf(l2, "Steps: %d\n", steps_int);
				}
			} else {
				fprintf(stderr, "Failed to parse steps data.\n");
				n_err++;
			}
		} else {
			fprintf(stderr, "Failed to parse steps data.\n");
			n_err++;
		}

		ptr = strstr(l1->str, "CFG scale: ");
		if (ptr) {
			double cfg_double = 0.0;
			if (parse_double_value(ptr + strlen("CFG scale: "), &cfg_double)) {
				if (set_widgets && data->cfg_spin) {
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->cfg_spin), cfg_double);
					n_loaded++;
				} else if (copy_all) {
					append_formatted_double(l2, "CFG scale", "%.1f", cfg_double);
				}
			} else {
				fprintf(stderr, "Failed to parse CFG data.\n");
				n_err++;
			}
		} else {
			fprintf(stderr, "Failed to parse CFG data.\n");
			n_err++;
		}

		ptr = strstr(l1->str, "Seed: ");
		if (ptr) {
			gint64 seed_val = -1;
			if (parse_int64_value(ptr + strlen("Seed: "), &seed_val) && seed_val >= -1) {
				if (set_widgets && data->seed_entry) {
					char seed_str[32];
					g_snprintf(seed_str, sizeof(seed_str), "%" G_GINT64_FORMAT, seed_val);
					gtk_editable_set_text(GTK_EDITABLE(data->seed_entry), seed_str);
					n_loaded++;
				} else if (copy_all) {
					g_string_append_printf(l2, "Seed: %" G_GINT64_FORMAT "\n", seed_val);
				}
			} else {
				fprintf(stderr, "Failed to parse seed data.\n");
				n_err++;
			}
		} else {
			fprintf(stderr, "Failed to parse seed data.\n");
			n_err++;
		}

		ptr = strstr(l1->str, "Size: ");
		if (ptr) {
			const char *start = ptr + strlen("Size: ");
			char *end_w = NULL;
			char *end_h = NULL;
			errno = 0;
			gint64 w_val = g_ascii_strtoll(start, &end_w, 10);
			gboolean size_ok = FALSE;
			int width_int = 0;
			int height_int = 0;

			if (end_w != start && errno != ERANGE && w_val > 0 && w_val <= INT_MAX && (*end_w == 'x' || *end_w == 'X')) {
				errno = 0;
				gint64 h_val = g_ascii_strtoll(end_w + 1, &end_h, 10);
				if (end_h != end_w + 1 && errno != ERANGE && h_val > 0 && h_val <= INT_MAX) {
					width_int = (int)w_val;
					height_int = (int)h_val;
					size_ok = TRUE;
				}
			}

			if (size_ok) {
				if (set_widgets) {
					char width_str[32];
					char height_str[32];
					g_snprintf(width_str, sizeof(width_str), "%d", width_int);
					g_snprintf(height_str, sizeof(height_str), "%d", height_int);

					set_dropdown_from_const_list(data->width_dd, LIST_RESOLUTIONS_STR, width_str);
					set_dropdown_from_const_list(data->height_dd, LIST_RESOLUTIONS_STR, height_str);
					n_loaded++;
				} else if (copy_all) {
					g_string_append_printf(l2, "Width: %d, Height: %d\n", width_int, height_int);
				}
			} else {
				fprintf(stderr, "Failed to parse image size data.\n");
				n_err++;
			}
		} else {
			fprintf(stderr, "Failed to parse image size data.\n");
			n_err++;
		}

		char checkpoint_str[MAX_PROPERTY_LENGTH];
		if (extract_delimited_value(l1->str, "Model: ", ",\r\n", checkpoint_str, sizeof(checkpoint_str))) {
			if (set_widgets) {
				set_dropdown_from_gtk_list(data->checkpoint_dd, checkpoint_str);
				n_loaded++;
			} else if (copy_all) {
				g_string_append_printf(l2, "Checkpoint: %s\n", checkpoint_str);
			}
		} else {
			fprintf(stderr, "Failed to parse checkpoint data.\n");
			n_err++;
		}

		if (is_new_format) {
			char sampler_str[MAX_PROPERTY_LENGTH];
			gboolean sampler_found = extract_quoted_value(l1->str, "\"sample_method\":\"", sampler_str, sizeof(sampler_str));
			if (!sampler_found) {
				sampler_found = extract_delimited_value(l1->str, "Sampler: ", ",\r\n", sampler_str, sizeof(sampler_str));
			}

			if (sampler_found) {
				if (set_widgets) {
					set_dropdown_from_const_list(data->sampler_dd, LIST_SAMPLES, sampler_str);
					n_loaded++;
				} else if (copy_all) {
					g_string_append_printf(l2, "Sampler: %s\n", sampler_str);
				}
			} else {
				fprintf(stderr, "Failed to parse sampler data.\n");
				n_err++;
			}

			char scheduler_str[MAX_PROPERTY_LENGTH];
			gboolean scheduler_found = extract_quoted_value(l1->str, "\"scheduler\":\"", scheduler_str, sizeof(scheduler_str));
			if (!scheduler_found) {
				scheduler_found = extract_delimited_value(l1->str, "Schedule type: ", ",\r\n", scheduler_str, sizeof(scheduler_str));
			}
			if (!scheduler_found) {
				scheduler_found = extract_delimited_value(l1->str, "SDCPP Schedule: ", ",\r\n", scheduler_str, sizeof(scheduler_str));
			}

			if (scheduler_found) {
				if (set_widgets) {
					set_dropdown_from_const_list(data->scheduler_dd, LIST_SCHEDULES, scheduler_str);
					n_loaded++;
				} else if (copy_all) {
					g_string_append_printf(l2, "Scheduler: %s\n", scheduler_str);
				}
			} else {
				fprintf(stderr, "Failed to parse scheduler data.\n");
				n_err++;
			}
		} else {
			ptr = strstr(l1->str, "Sampler: ");
			gboolean old_sampler_ok = FALSE;
			char sampler_str[MAX_PROPERTY_LENGTH];
			char scheduler_str[MAX_PROPERTY_LENGTH];

			if (ptr) {
				const char *start = ptr + strlen("Sampler: ");
				while (*start == ' ') start++;
				size_t line_len = strcspn(start, ",\r\n");
				const char *space = memchr(start, ' ', line_len);
				if (space != NULL && space > start) {
					size_t sampler_len = (size_t)(space - start);
					const char *sched_start = space + 1;
					while (sched_start < start + line_len && *sched_start == ' ') sched_start++;
					size_t sched_len = (size_t)((start + line_len) - sched_start);
					if (sched_len > 0) {
						copy_utf8_bounded(start, sampler_len, sampler_str, sizeof(sampler_str));
						copy_utf8_bounded(sched_start, sched_len, scheduler_str, sizeof(scheduler_str));
						old_sampler_ok = TRUE;
					}
				}
			}

			if (old_sampler_ok) {
				if (set_widgets) {
					set_dropdown_from_const_list(data->sampler_dd, LIST_SAMPLES, sampler_str);
					set_dropdown_from_const_list(data->scheduler_dd, LIST_SCHEDULES, scheduler_str);
					n_loaded++;
				} else if (copy_all) {
					g_string_append_printf(l2, "Sampler: %s, Scheduler: %s\n", sampler_str, scheduler_str);
				}
			} else {
				fprintf(stderr, "Failed to parse sampler data.\n");
				n_err++;
			}
		}

		ptr = strstr(l1->str, ", VAE: ");
		if (ptr) {
			char vae_str[MAX_PROPERTY_LENGTH];
			if (extract_delimited_value(ptr, ", VAE: ", ",\r\n", vae_str, sizeof(vae_str))) {
				if (set_widgets) {
					set_dropdown_from_gtk_list(data->vae_dd, vae_str);
					n_loaded++;
				} else if (copy_all) {
					g_string_append_printf(l2, "VAE: %s\n", vae_str);
				}
			} else {
				fprintf(stderr, "Failed to parse VAE data.\n");
				if (n_err == 0 && set_widgets && data->vae_dd) {
					gtk_drop_down_set_selected(GTK_DROP_DOWN(data->vae_dd), 0);
				}
				n_err++;
			}
		} else {
			if (n_err > 0) {
				fprintf(stderr, "Failed to parse VAE data.\n");
				n_err++;
			} else if (set_widgets && data->vae_dd) {
				gtk_drop_down_set_selected(GTK_DROP_DOWN(data->vae_dd), 0);
			}
		}

		ptr = strstr(l1->str, ", Clip skip: ");
		if (ptr) {
			int clip_skip_value = 0;
			if (parse_int_value(ptr + strlen(", Clip skip: "), &clip_skip_value)) {
				if (set_widgets && data->clip_skip_spin) {
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->clip_skip_spin), clip_skip_value);
					n_loaded++;
				} else if (copy_all) {
					g_string_append_printf(l2, "Clip skip: %d\n", clip_skip_value);
				}
			} else {
				fprintf(stderr, "Failed to parse Clip skip data.\n");
				n_err++;
			}
		} else {
			fprintf(stderr, "Failed to parse Clip skip data.\n");
			n_err++;
		}

		const char *upscaler_key = ", Hires upscaler: ";
		ptr = strstr(l1->str, upscaler_key);
		if (!ptr) {
			upscaler_key = ", Hires upscale: ";
			ptr = strstr(l1->str, upscaler_key);
		}
		if (ptr) {
			char hires_upscaler_str[MAX_PROPERTY_LENGTH];
			if (extract_delimited_value(ptr, upscaler_key, ",\r\n", hires_upscaler_str, sizeof(hires_upscaler_str))) {
				if (set_widgets) {
					set_dropdown_from_gtk_list(data->hires_upscaler_dd, hires_upscaler_str);
					n_loaded++;
				} else if (copy_all) {
					g_string_append_printf(l2, "Hires upscale: %s\n", hires_upscaler_str);
				}
			} else {
				fprintf(stderr, "Failed to parse Hires upscale data.\n");
				if (n_err == 0 && set_widgets && data->hires_upscaler_dd) {
					gtk_drop_down_set_selected(GTK_DROP_DOWN(data->hires_upscaler_dd), 0);
				}
				n_err++;
			}
		} else {
			if (n_err > 0) {
				fprintf(stderr, "Failed to parse Hires upscale data.\n");
				n_err++;
			} else if (set_widgets && data->hires_upscaler_dd) {
				gtk_drop_down_set_selected(GTK_DROP_DOWN(data->hires_upscaler_dd), 0);
			}
		}

		const char *scale_str = NULL;
		if ((ptr = strstr(l1->str, ", Hires scale: ")) != NULL) {
			scale_str = ptr + strlen(", Hires scale: ");
		} else {
			const char *slg_open = strstr(l1->str, "\"slg\":{");
			const char *slg_close = slg_open ? strchr(slg_open, '}') : NULL;

			ptr = strstr(l1->str, ",\"scale\":");
			while (ptr && slg_open && slg_close && ptr > slg_open && ptr < slg_close) {
				ptr = strstr(ptr + 1, ",\"scale\":");
			}
			if (ptr) scale_str = ptr + strlen(",\"scale\":");
		}

		if (scale_str) {
			double hires_scale_double = 0.0;
			if (parse_double_value(scale_str, &hires_scale_double)) {
				if (hires_scale_double >= 0.0) {
					if (set_widgets && data->hires_scale_spin) {
						gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->hires_scale_spin), hires_scale_double);
						n_loaded++;
					} else if (copy_all) {
						append_formatted_double(l2, "Hires scale", "%.2f", hires_scale_double);
					}
				}
			} else {
				fprintf(stderr, "Failed to parse Hires scale data.\n");
				n_err++;
			}
		} else if (n_err > 0) {
			fprintf(stderr, "Failed to parse Hires scale data.\n");
			n_err++;
		}

		ptr = strstr(l1->str, ", Hires steps: ");
		if (ptr) {
			int hires_steps_int = 0;
			if (parse_int_value(ptr + strlen(", Hires steps: "), &hires_steps_int)) {
				if (set_widgets && data->hires_steps_spin) {
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->hires_steps_spin), hires_steps_int);
					n_loaded++;
				} else if (copy_all) {
					g_string_append_printf(l2, "Hires steps: %d\n", hires_steps_int);
				}
			} else {
				fprintf(stderr, "Failed to parse Hires steps data.\n");
				n_err++;
			}
		} else if (n_err > 0) {
			fprintf(stderr, "Failed to parse Hires steps data.\n");
			n_err++;
		}

		ptr = strstr(l1->str, ", Denoising strength: ");
		if (ptr) {
			double hires_denoising_str_double = 0.0;
			if (parse_double_value(ptr + strlen(", Denoising strength: "), &hires_denoising_str_double)) {
				if (set_widgets && data->hires_denoise_spin) {
					gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->hires_denoise_spin), hires_denoising_str_double);
					n_loaded++;
				} else if (copy_all) {
					append_formatted_double(l2, "Hires Denoising strength", "%.2f", hires_denoising_str_double);
				}
			} else {
				fprintf(stderr, "Failed to parse Hires denoising strength data.\n");
				n_err++;
			}
		} else if (n_err > 0) {
			fprintf(stderr, "Failed to parse Hires denoising strength data.\n");
			n_err++;
		}

		ptr = strstr(l1->str, ",\"strength\":");
		if (ptr) {
			double denoise_str_value = 0.0;
			if (parse_double_value(ptr + strlen(",\"strength\":"), &denoise_str_value)) {
				if (denoise_str_value >= 0.0 && denoise_str_value <= 1.0) {
					if (set_widgets && data->denoise_spin) {
						gtk_spin_button_set_value(GTK_SPIN_BUTTON(data->denoise_spin), denoise_str_value);
						n_loaded++;
					} else if (copy_all) {
						append_formatted_double(l2, "Denoising strength", "%.2f", denoise_str_value);
					}
				}
			} else {
				fprintf(stderr, "Failed to parse Denoise strength data.\n");
				n_err++;
			}
		} else if (!is_new_format) {
			fprintf(stderr, "Failed to parse Denoise strength data.\n");
			n_err++;
		}
	}

	if (copy_all) {
		if (l2->len > 0) {
			g_string_append_printf(l2, "Application: %s (stable-diffusion.cpp)", APP_NAME_VERSION);
			set_clipboard_text_safe(data->win, l2->str);
			if (data->main_overlay) show_toast_notification(GTK_OVERLAY(data->main_overlay), "Generation Info Copied!");
		} else if (data->main_overlay) {
			show_toast_notification(GTK_OVERLAY(data->main_overlay), "No generation info found.");
		}
	} else if (copy_pp) {
		if (data->main_overlay) {
			show_toast_notification(GTK_OVERLAY(data->main_overlay), copied_pp ? "Positive Prompt Copied!" : "No positive prompt found.");
		}
	} else if (copy_np) {
		if (data->main_overlay) {
			show_toast_notification(GTK_OVERLAY(data->main_overlay), copied_np ? "Negative Prompt Copied!" : "No negative prompt found.");
		}
	} else if (set_widgets) {
		if (data->main_overlay) {
			show_toast_notification(GTK_OVERLAY(data->main_overlay), n_loaded > 0 ? "Generation Info Loaded!" : "No generation info found.");
		}
	}

	g_string_free(l1, TRUE);
	g_string_free(l2, TRUE);
}

static void apply_img2img_file(LoadImg2ImgData *data, const char *path)
{
	if (!data || !path) return;

	if (check_file_exists(path, 0) == 1) {
		GError *tex_err = NULL;
		GdkTexture *texture = gdk_texture_new_from_filename(path, &tex_err);
		if (texture != NULL) {
			if (data->img2img_file_path) g_string_assign(data->img2img_file_path, path);
			if (data->image_wgt) gtk_picture_set_paintable(GTK_PICTURE(data->image_wgt), GDK_PAINTABLE(texture));
			g_object_unref(texture);
			if (data->img2img_visibility_toggle_btn) {
				gtk_widget_add_css_class(data->img2img_visibility_toggle_btn, "custom_btn_img2img");
			}
			if (data->inpaint_check) {
				gtk_check_button_set_active(GTK_CHECK_BUTTON(data->inpaint_check), FALSE);
			}
			if (data->overlay_img2img) {
				clear_img2img_overlay(GTK_OVERLAY(data->overlay_img2img));
			}
			if (data->main_overlay) {
				show_toast_notification(GTK_OVERLAY(data->main_overlay), "Input image set.");
			}
			return;
		}
		g_printerr("Error loading image '%s': %s\n", path, tex_err ? tex_err->message : "Unknown error");
		g_clear_error(&tex_err);
	} else {
		g_printerr("Error loading image: The file '%s' is missing, corrupted, or invalid.\n", path);
	}

	if (data->win) {
		show_simple_message(data->win,
			"Error loading image",
			"Error loading image: The selected file is missing, corrupted, or invalid.\n", 1
		);
	}
}

#if GTK_CHECK_VERSION(4, 10, 0)
static void read_png_metadata(GObject *client, GAsyncResult *res, gpointer user_data)
{
	OpenPNGDialogContext *ctx = user_data;
	g_autoptr(GError) error = NULL;
	g_autoptr(GFile) png_file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(client), res, &error);

	if (!ctx) return;

	gboolean cancelled = g_cancellable_is_cancelled(ctx->cancellable);
	gboolean win_alive = (ctx->win != NULL);
	if (win_alive) {
		g_object_remove_weak_pointer(G_OBJECT(ctx->win), (gpointer *)&ctx->win);
	}

	if (!cancelled && win_alive && ctx->data != NULL) {
		LoadPNGData *data = ctx->data;

		if (error != NULL &&
		!g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_DISMISSED) &&
		!g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_CANCELLED) &&
		!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_warning("Opening file for metadata failed: %s", error->message);
		}

		if (png_file != NULL) {
			gchar *img_path = g_file_get_path(png_file);
			if (img_path != NULL) {
				set_png_metadata(img_path, METADATA_TASK_LOAD_WIDGETS, data);
				g_free(img_path);
			}
		}

		if (data->cancellable == ctx->cancellable) {
			g_clear_object(&data->cancellable);
		}
	}

	g_object_unref(ctx->cancellable);
	g_free(ctx);
}

static void set_file_path(GObject *client, GAsyncResult *res, gpointer user_data)
{
	OpenImg2ImgDialogContext *ctx = user_data;
	g_autoptr(GError) error = NULL;
	g_autoptr(GFile) png_file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(client), res, &error);

	if (!ctx) return;

	gboolean cancelled = g_cancellable_is_cancelled(ctx->cancellable);
	gboolean win_alive = (ctx->win != NULL);
	if (win_alive) {
		g_object_remove_weak_pointer(G_OBJECT(ctx->win), (gpointer *)&ctx->win);
	}

	if (!cancelled && win_alive && ctx->data != NULL) {
		LoadImg2ImgData *data = ctx->data;

		if (error != NULL &&
		!g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_DISMISSED) &&
		!g_error_matches(error, GTK_DIALOG_ERROR, GTK_DIALOG_ERROR_CANCELLED) &&
		!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_warning("Opening file failed: %s", error->message);
		}

		if (png_file != NULL) {
			char *path = g_file_get_path(png_file);
			if (path != NULL) {
				apply_img2img_file(data, path);
				g_free(path);
			}
		}

		if (data->cancellable == ctx->cancellable) {
			g_clear_object(&data->cancellable);
		}
	}

	g_object_unref(ctx->cancellable);
	g_free(ctx);
}

#else
static void read_png_metadata_deprecated(GtkDialog *dialog, int response, gpointer user_data)
{
	if (response == GTK_RESPONSE_ACCEPT && user_data != NULL) {
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		g_autoptr(GFile) png_file = gtk_file_chooser_get_file(chooser);

		if (png_file != NULL) {
			gchar *img_path = g_file_get_path(png_file);
			if (img_path != NULL) {
				set_png_metadata(img_path, METADATA_TASK_LOAD_WIDGETS, user_data);
				g_free(img_path);
			}
		}
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
}

static void set_file_path_deprecated(GtkDialog *dialog, int response, gpointer user_data)
{
	if (response == GTK_RESPONSE_ACCEPT && user_data != NULL) {
		LoadImg2ImgData *data = user_data;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		g_autoptr(GFile) png_file = gtk_file_chooser_get_file(chooser);

		if (png_file != NULL) {
			char *path = g_file_get_path(png_file);
			if (path != NULL) {
				apply_img2img_file(data, path);
				g_free(path);
			}
		}
	}
	gtk_window_destroy(GTK_WINDOW(dialog));
}
#endif

static MetadataTask get_metadata_task_from_button(GtkWidget *btn, const char *func_name)
{
	const char *btn_label = gtk_button_get_label(GTK_BUTTON(btn));
	if (btn_label == NULL) return METADATA_TASK_LOAD_WIDGETS;
	if (g_strcmp0(btn_label, "Copy Negative Prompt") == 0) return METADATA_TASK_COPY_NEGATIVE;
	if (g_strcmp0(btn_label, "Copy Positive Prompt") == 0) return METADATA_TASK_COPY_POSITIVE;
	if (g_strcmp0(btn_label, "Copy All") == 0) return METADATA_TASK_COPY_ALL;

	g_printerr("Error in function '%s': The label '%s' is not recognized.\n", func_name, btn_label);
	return METADATA_TASK_LOAD_WIDGETS;
}

void load_metadata_from_favorites(GtkWidget *btn, gpointer user_data)
{
	if (btn == NULL || user_data == NULL) {
		g_printerr("Critical error in function 'load_metadata_from_favorites'.\n");
		return;
	}

	GtkWidget *fav_image = GTK_WIDGET(user_data);
	FavoritesWindowData *favorites_d = g_object_get_data(G_OBJECT(btn), "favorites_d");

	if (favorites_d == NULL || favorites_d->load_png_info_d == NULL) {
		g_printerr("Critical error in function 'load_metadata_from_favorites'.\n");
		return;
	}

	g_autofree gchar *img_path = g_strdup(g_object_get_data(G_OBJECT(fav_image), "image-path"));
	if (img_path == NULL) {
		g_printerr("Failed to get image path.\n");
	} else if (check_file_exists(img_path, 0) == 1) {
		MetadataTask task = get_metadata_task_from_button(btn, "load_metadata_from_favorites");
		set_png_metadata(img_path, task, favorites_d->load_png_info_d);
	} else {
		g_printerr("File '%s' does not exist or cannot be accessed.\n", img_path);
	}

	GtkWidget *menu_btn_popover = gtk_widget_get_ancestor(btn, GTK_TYPE_POPOVER);
	if (menu_btn_popover) gtk_popover_popdown(GTK_POPOVER(menu_btn_popover));
}

void load_from_img_preview(GtkWidget *btn, gpointer user_data)
{
	if (btn == NULL || user_data == NULL) return;

	LoadPNGData *data = user_data;
	if (data->image_files == NULL || data->current_image_index == NULL) return;

	MetadataTask task = get_metadata_task_from_button(btn, "load_from_img_preview");

	if (data->image_files->len > 0 && *data->current_image_index >= 0 &&
	(guint)*data->current_image_index < data->image_files->len) {
		g_autofree gchar *img_path = g_strdup(g_ptr_array_index(data->image_files, (guint)*data->current_image_index));

		if (img_path != NULL && check_file_exists(img_path, 0) == 1) {
			set_png_metadata(img_path, task, data);
		} else {
			g_printerr("Failed to get image path.\n");
		}
	} else {
		g_printerr("No images in the 'output' directory, using default image.\n");
		if (data->main_overlay) {
			show_toast_notification(GTK_OVERLAY(data->main_overlay), "No images in the 'output' directory.");
		}
		set_png_metadata(DEFAULT_IMG_PATH, task, data);
	}

	GtkWidget *menu_btn_popover = gtk_widget_get_ancestor(btn, GTK_TYPE_POPOVER);
	if (menu_btn_popover) gtk_popover_popdown(GTK_POPOVER(menu_btn_popover));
}

void load_from_img_btn_cb(GtkWidget *btn, gpointer user_data)
{
	(void)btn;
	LoadPNGData *data = user_data;
	if (!data || !data->win) return;

	GtkWindow *win = GTK_WINDOW(data->win);

	#if GTK_CHECK_VERSION(4, 10, 0)
		GtkFileDialog *load_img_dialog = gtk_file_dialog_new();
		gtk_file_dialog_set_title(load_img_dialog, "Choose the image:");
		gtk_file_dialog_set_modal(load_img_dialog, TRUE);

		GtkFileFilter *load_img_filter = gtk_file_filter_new();
		gtk_file_filter_add_suffix(load_img_filter, "png");
		gtk_file_filter_set_name(load_img_filter, "PNG");

		GListStore *filter_list_store = g_list_store_new(GTK_TYPE_FILE_FILTER);
		g_list_store_append(filter_list_store, load_img_filter);
		gtk_file_dialog_set_filters(load_img_dialog, G_LIST_MODEL(filter_list_store));

		if (data->cancellable) {
			g_cancellable_cancel(data->cancellable);
			g_clear_object(&data->cancellable);
		}

		data->cancellable = g_cancellable_new();

		OpenPNGDialogContext *ctx = g_new0(OpenPNGDialogContext, 1);
		ctx->data = data;
		ctx->cancellable = g_object_ref(data->cancellable);
		ctx->win = data->win;
		g_object_add_weak_pointer(G_OBJECT(ctx->win), (gpointer *)&ctx->win);

		gtk_file_dialog_open(load_img_dialog, win, data->cancellable, read_png_metadata, ctx);

		g_object_unref(filter_list_store);
		g_object_unref(load_img_filter);
		g_object_unref(load_img_dialog);
	#else
		GtkWidget *load_img_dialog = gtk_file_chooser_dialog_new("Choose the image:",
			win,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			"Cancel",
			GTK_RESPONSE_CANCEL,
			"Open",
			GTK_RESPONSE_ACCEPT,
			NULL);

		gtk_window_set_modal(GTK_WINDOW(load_img_dialog), TRUE);
		gtk_window_set_destroy_with_parent(GTK_WINDOW(load_img_dialog), TRUE);

		GtkFileFilter *load_img_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(load_img_filter, "PNG");
		gtk_file_filter_add_pattern(load_img_filter, "*.png");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(load_img_dialog), load_img_filter);
		g_object_unref(load_img_filter);

		g_signal_connect(load_img_dialog, "response", G_CALLBACK(read_png_metadata_deprecated), user_data);
		gtk_window_present(GTK_WINDOW(load_img_dialog));
	#endif
}

void load_img2img_btn_cb(GtkWidget *btn, gpointer user_data)
{
	(void)btn;
	LoadImg2ImgData *data = user_data;
	if (!data || !data->win) return;

	GtkWindow *win = GTK_WINDOW(data->win);

	#if GTK_CHECK_VERSION(4, 10, 0)
		GtkFileDialog *load_img_dialog = gtk_file_dialog_new();
		gtk_file_dialog_set_title(load_img_dialog, "Choose the image file:");
		gtk_file_dialog_set_modal(load_img_dialog, TRUE);

		GtkFileFilter *load_img_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(load_img_filter, "Images (*.png, *.jpg)");
		gtk_file_filter_add_suffix(load_img_filter, "png");
		gtk_file_filter_add_suffix(load_img_filter, "jpg");
		gtk_file_filter_add_suffix(load_img_filter, "jpeg");

		GListStore *filter_list_store = g_list_store_new(GTK_TYPE_FILE_FILTER);
		g_list_store_append(filter_list_store, load_img_filter);
		gtk_file_dialog_set_filters(load_img_dialog, G_LIST_MODEL(filter_list_store));

		if (data->cancellable) {
			g_cancellable_cancel(data->cancellable);
			g_clear_object(&data->cancellable);
		}

		data->cancellable = g_cancellable_new();

		OpenImg2ImgDialogContext *ctx = g_new0(OpenImg2ImgDialogContext, 1);
		ctx->data = data;
		ctx->cancellable = g_object_ref(data->cancellable);
		ctx->win = data->win;
		g_object_add_weak_pointer(G_OBJECT(ctx->win), (gpointer *)&ctx->win);

		gtk_file_dialog_open(load_img_dialog, win, data->cancellable, set_file_path, ctx);

		g_object_unref(filter_list_store);
		g_object_unref(load_img_filter);
		g_object_unref(load_img_dialog);
	#else
		GtkWidget *load_img_dialog = gtk_file_chooser_dialog_new("Choose the image file:",
			win,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			"Cancel",
			GTK_RESPONSE_CANCEL,
			"Open",
			GTK_RESPONSE_ACCEPT,
			NULL);

		gtk_window_set_modal(GTK_WINDOW(load_img_dialog), TRUE);
		gtk_window_set_destroy_with_parent(GTK_WINDOW(load_img_dialog), TRUE);

		GtkFileFilter *load_img_filter = gtk_file_filter_new();
		gtk_file_filter_set_name(load_img_filter, "Images (*.png, *.jpg)");
		gtk_file_filter_add_pattern(load_img_filter, "*.png");
		gtk_file_filter_add_pattern(load_img_filter, "*.jpg");
		gtk_file_filter_add_pattern(load_img_filter, "*.jpeg");
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(load_img_dialog), load_img_filter);
		g_object_unref(load_img_filter);

		g_signal_connect(load_img_dialog, "response", G_CALLBACK(set_file_path_deprecated), user_data);
		gtk_window_present(GTK_WINDOW(load_img_dialog));
	#endif
}

void set_current_preview_to_img2img(GtkWidget *btn, gpointer user_data)
{
	(void)btn;
	LoadImg2ImgFromPreviewData *data = user_data;
	if (!data || !data->image_files || !data->current_image_index) return;

	if (data->image_files->len > 0 && *data->current_image_index >= 0 &&
	(guint)*data->current_image_index < data->image_files->len) {
		const char *path = g_ptr_array_index(data->image_files, (guint)*data->current_image_index);

		if (path != NULL && check_file_exists(path, 0) == 1) {
			GError *tex_err = NULL;
			GdkTexture *texture = gdk_texture_new_from_filename(path, &tex_err);
			if (texture != NULL) {
				if (data->inpaint_check) {
					gtk_check_button_set_active(GTK_CHECK_BUTTON(data->inpaint_check), FALSE);
				}
				if (data->overlay_img2img) {
					clear_img2img_overlay(GTK_OVERLAY(data->overlay_img2img));
				}
				if (data->img2img_file_path) {
					g_string_assign(data->img2img_file_path, path);
				}
				if (data->image_wgt) {
					gtk_picture_set_paintable(GTK_PICTURE(data->image_wgt), GDK_PAINTABLE(texture));
				}
				g_object_unref(texture);
				if (data->img2img_visibility_toggle_btn) {
					gtk_widget_add_css_class(data->img2img_visibility_toggle_btn, "custom_btn_img2img");
				}
				if (data->main_overlay) {
					show_toast_notification(GTK_OVERLAY(data->main_overlay), "Input image set.");
				}
			} else {
				g_printerr("Error loading current preview image into img2img: %s\n", tex_err ? tex_err->message : "Unknown error");
				g_clear_error(&tex_err);
			}
		} else {
			g_printerr("Error loading current preview image into img2img.\n");
		}
	}
}
