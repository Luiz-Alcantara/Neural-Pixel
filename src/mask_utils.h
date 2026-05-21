#ifndef MASK_UTILS_H
#define MASK_UTILS_H

void clear_img2img_overlay(GtkOverlay *overlay);

static void draw_brush(double x, double y, double line_width, gpointer user_data);

static void drag_end_cb(GtkGestureDrag *g, double ox, double oy, gpointer user_data);

static void drag_update_cb(GtkGestureDrag *g, double ox, double oy, gpointer user_data);

static void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer user_data);

static void clear_canvas(GtkButton *btn, gpointer user_data);

static void save_png_mask(GtkButton *btn, gpointer user_data);

void show_mask_area (GtkWidget *wgt, gpointer user_data);

#endif // MASK_UTILS_H
