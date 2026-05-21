#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "png_utils.h"
#include "structs.h"
#include "widgets_cb.h"
#include "constants.h"


static void draw_brush(double x, double y, double line_width, gpointer user_data)
{
	MaskData *data = (MaskData *)user_data;
	if (!data) return;
	cairo_t *cr = cairo_create(data->surface);
	cairo_set_source_rgb(cr, 247, 0, 255);
	cairo_set_line_width(cr, line_width);
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

	if (data->last_x < 0) {
		cairo_arc(cr, x, y, 1.5, 0, 2 * G_PI);
		cairo_fill(cr);
	} else {
		cairo_move_to(cr, data->last_x, data->last_y);
		cairo_line_to(cr, x, y);
		cairo_stroke(cr);
	}
	data->last_x = x; data->last_y = y;
	cairo_destroy(cr);
	gtk_widget_queue_draw(data->drawing_area);
}


static void drag_end_cb(GtkGestureDrag *g, double ox, double oy, gpointer user_data)
{
	MaskData *data = (MaskData *)user_data;
	if (!data) return;

	data->last_x = -1.0; data->last_y = -1.0;
}


static void drag_update_cb(GtkGestureDrag *g, double ox, double oy, gpointer user_data)
{
	MaskData *data = (MaskData *)user_data;
	if (!data) return;

	double brush_width = gtk_spin_button_get_value(GTK_SPIN_BUTTON(data->brush_width_spin));
	double sx, sy;
	gtk_gesture_drag_get_start_point(g, &sx, &sy);
	draw_brush(sx + ox, sy + oy, brush_width, user_data);
}


static void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data)
{
	MaskData *data = (MaskData *)user_data;

	if (data->surface) {
		cairo_set_source_surface(cr, data->surface, 0, 0);
		cairo_paint(cr);
	}
}


static void clear_canvas(GtkButton *btn, gpointer user_data)
{
	MaskData *data = (MaskData *)user_data;

	if (data->surface) cairo_surface_destroy(data->surface);

	data->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, data->ref_w, data->ref_h);
	cairo_t *cr = cairo_create(data->surface);
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);

	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(data->drawing_area), data->ref_w);
	gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(data->drawing_area), data->ref_h);
	gtk_widget_queue_draw(data->drawing_area);
}


static void save_png_mask(GtkButton *btn, gpointer user_data)
{
	MaskData *data = (MaskData *)user_data;

	int width = cairo_image_surface_get_width(data->surface);
	int height = cairo_image_surface_get_height(data->surface);

	cairo_surface_t *output_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
	cairo_t *cr = cairo_create(output_surface);

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);

	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_mask_surface(cr, data->surface, 0, 0);

	cairo_status_t status = cairo_surface_write_to_png(output_surface, MASK_IMG_PATH);

	if (status != CAIRO_STATUS_SUCCESS) {
		g_printerr("Error saving PNG: %s\n", cairo_status_to_string(status));
	} else {
		g_print("Image saved successfully to %s\n", MASK_IMG_PATH);
		
		clear_img2img_overlay(GTK_OVERLAY(data->overlay_img2img));
		
		GtkWidget *mask_picture = gtk_picture_new_for_filename(MASK_IMG_PATH);
		gtk_widget_set_valign(mask_picture, GTK_ALIGN_CENTER);
		gtk_widget_set_halign(mask_picture, GTK_ALIGN_CENTER);
		gtk_widget_set_opacity(mask_picture, 0.5);
		gtk_picture_set_content_fit(GTK_PICTURE(mask_picture), GTK_CONTENT_FIT_SCALE_DOWN);
		gtk_overlay_add_overlay(GTK_OVERLAY(data->overlay_img2img), mask_picture);
		
		gtk_check_button_set_active(GTK_CHECK_BUTTON(data->inpaint_check), TRUE);
	}

	cairo_destroy(cr);
	cairo_surface_destroy(output_surface);
}


void show_mask_area (GtkWidget *wgt, gpointer user_data)
{
	LoadImg2ImgData *data = (LoadImg2ImgData *)user_data;
	
	if (strcmp(data->img2img_file_path->str, "None") == 0) {
		show_error_message(data->win, "Error loading source image",
		"Select an image first to use the inpainting tool.");
		return;
	}
	
	uint32_t w = 0;
	uint32_t h = 0;
	int success_png_size = get_png_dimensions(data->img2img_file_path->str, &w, &h);
	if (success_png_size != 0) {
		show_error_message(data->win, "Error retrieving reference image dimensions.",
			"*Only PNGs supported.");
		g_printerr("Error retrieving reference image dimensions. (Only PNGs supported.)\n");
		return;
	}

	GtkWidget *mask_win = gtk_window_new();
	gtk_window_set_title(GTK_WINDOW(mask_win), "Mask Tool");
	gtk_window_set_transient_for(GTK_WINDOW(mask_win), GTK_WINDOW(data->win));
	gtk_window_set_resizable (GTK_WINDOW(mask_win), TRUE);
	gtk_window_set_deletable (GTK_WINDOW(mask_win), TRUE);
	gtk_window_set_decorated (GTK_WINDOW(mask_win), TRUE);
	gtk_window_set_destroy_with_parent (GTK_WINDOW(mask_win), TRUE);

	GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_widget_add_css_class(main_box, "left_box");
	gtk_window_set_child(GTK_WINDOW(mask_win), main_box);

	GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_widget_add_css_class(toolbar, "left_box");
	gtk_widget_set_margin_start(toolbar, 5);
	gtk_widget_set_margin_top(toolbar, 5);
	
	GtkWidget *brush_width_label = gtk_label_new("Brush Width");
	gtk_widget_add_css_class(brush_width_label, "param_label");
	gtk_box_append(GTK_BOX(toolbar), brush_width_label);
	
	GtkWidget *brush_width_spin = gtk_spin_button_new_with_range (1.0, 100.0, 5.0);
	gtk_widget_add_css_class(brush_width_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(brush_width_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(brush_width_spin), 30);
	gtk_widget_set_tooltip_text(GTK_WIDGET(brush_width_spin),
	"Sets the width of the draw mask brush.");
	gtk_box_append(GTK_BOX(toolbar), brush_width_spin);
	
	GtkWidget *clear_btn = gtk_button_new_with_label("Clear Canvas");
	GtkWidget *save_btn = gtk_button_new_with_label("Save Mask");
	GtkWidget *done_btn = gtk_button_new_with_label("Done");
	
	gtk_widget_add_css_class(clear_btn, "custom_btn");
	gtk_widget_add_css_class(save_btn, "custom_btn");
	gtk_widget_add_css_class(done_btn, "custom_btn");
	
	gtk_widget_set_hexpand(clear_btn, TRUE);
	gtk_widget_set_hexpand(save_btn, TRUE);
	gtk_widget_set_hexpand(done_btn, TRUE);

	gtk_box_append(GTK_BOX(toolbar), clear_btn);
	gtk_box_append(GTK_BOX(toolbar), save_btn);
	gtk_box_append(GTK_BOX(toolbar), done_btn);
	gtk_box_append(GTK_BOX(main_box), toolbar);
	
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	cairo_t *cr = cairo_create(surface);
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);
	
	GtkWidget *overlay = gtk_overlay_new();
	gtk_widget_set_size_request(overlay, w, h);

	
	GtkWidget *picture = gtk_picture_new_for_filename(data->img2img_file_path->str);
	gtk_picture_set_content_fit(GTK_PICTURE(picture), GTK_CONTENT_FIT_SCALE_DOWN);
	gtk_overlay_set_child(GTK_OVERLAY(overlay), picture);

	GtkWidget *drawing_area = gtk_drawing_area_new();
	gtk_widget_set_opacity(drawing_area, 0.8);
	gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(drawing_area), w > 0 ? w : 512);
	gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(drawing_area), h > 0 ? h : 512);
	gtk_widget_set_halign(drawing_area, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(drawing_area, GTK_ALIGN_CENTER);
	
	MaskData *mask_data = g_new(MaskData, 1);
	mask_data->overlay_img2img = data->overlay_img2img;
	mask_data->inpaint_check = data->inpaint_check;
	mask_data->ref_w = w;
	mask_data->ref_h = h;
	mask_data->last_x = -1.0;
	mask_data->last_y = -1.0;
	mask_data->surface = surface;
	mask_data->drawing_area = drawing_area;
	mask_data->brush_width_spin = brush_width_spin;
	
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_function, mask_data, NULL);
	gtk_overlay_add_overlay(GTK_OVERLAY(overlay), drawing_area);
	gtk_widget_queue_draw(drawing_area);

	GtkWidget *scroll = gtk_scrolled_window_new();
	gtk_widget_set_size_request(scroll, 800, 800);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), overlay);
	gtk_widget_set_vexpand(scroll, TRUE);
	gtk_box_append(GTK_BOX(main_box), scroll);

	GtkGesture *drag = gtk_gesture_drag_new();
	gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(drag));
	g_signal_connect(drag, "drag-update", G_CALLBACK(drag_update_cb), mask_data);
	g_signal_connect(drag, "drag-end", G_CALLBACK(drag_end_cb), mask_data);
	g_signal_connect(save_btn, "clicked", G_CALLBACK(save_png_mask), mask_data);
	g_signal_connect(clear_btn, "clicked", G_CALLBACK(clear_canvas), mask_data);
	g_signal_connect(done_btn, "clicked", G_CALLBACK(quit_btn_callback), mask_win);
	g_signal_connect (mask_win, "close-request", G_CALLBACK (on_add_mask_btn_destroy), mask_data);

	gtk_window_present(GTK_WINDOW(mask_win));
}

