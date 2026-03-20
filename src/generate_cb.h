#ifndef GENERATE_CB_H
#define GENERATE_CB_H

static void handle_stderr(GObject* stream_obj, GAsyncResult* res, gpointer user_data);

static void show_progress(GObject* stream_obj, GAsyncResult* res, gpointer user_data);

static void on_subprocess_end(GObject* source_object, GAsyncResult* res, gpointer user_data);

static void start_reading_error(gpointer user_data);

static void start_reading_output(gpointer user_data);

static void start_generation(gpointer user_data);

void prepare_gen_data(GtkWidget *gen_btn, gpointer user_data);

#endif // GENERATE_CB_H
