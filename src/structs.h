#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
	GtkApplication *app;
	GtkWidget *window;
	GtkWidget *button;
	GString *model_string;
	GString *vae_string;
	GString *cnet_string;
	GString *upscale_string;
	GString *clip_l_string;
	GString *clip_g_string;
	GString *t5xxl_string;
	int sample_index;
	int schedule_index;
	int n_steps_index;
	int w_index;
	int h_index;
	int bs_index;
	int cpu_bool;
	int vt_bool;
	int k_clip_bool;
	int k_cnet_bool;
	int k_vae_bool;
	int fa_bool;
	int taesd_bool;
	int verbose_bool;
	long long int seed_value;
	double cfg_value;
	double denoise_value;
	double clip_skip_value;
	double up_repeat_value;
	GString *img2img_file_path;
} AppStartData;

typedef struct {
	char *pos_p;
	char *neg_p;
	char *img_name;
} MyCacheData;

typedef struct {
	GString *dd_item_str;
	int req_int;
	GtkWidget *g_btn;
} DropDownModelsNameData;

typedef struct {
	int tb_type;
	GtkTextBuffer *textbuffer;
} DropDownTextBufferData;

typedef struct {
	int pid;
	gchar *result_img_path;
	gchar **cmd_chunks;
	GString *cmd_string;
	GtkWidget *button;
	GtkWidget *image_widget;
	GtkWidget *show_img_btn;
	GtkWidget *halt_btn;
} EndGenerationData;

typedef struct {
	GString *model_string;
	GString *vae_string;
	GString *cnet_string;
	GString *upscale_string;
	GString *clip_l_string;
	GString *clip_g_string;
	GString *t5xxl_string;
	int *sample_index;
	int *schedule_index;
	int *n_steps_index;
	int *w_index;
	int *h_index;
	int *bs_index;
	int *cpu_bool;
	int *vt_bool;
	int *k_clip_bool;
	int *k_cnet_bool;
	int *k_vae_bool;
	int *fa_bool;
	int *taesd_bool;
	int *verbose_bool;
	long long int *seed_value;
	double *cfg_value;
	double *denoise_value;
	double *clip_skip_value;
	double *up_repeat_value;
	GtkTextBuffer *pos_p;
	GtkTextBuffer *neg_p;
	GtkWidget *image_widget;
	GtkWidget *show_img_btn;
	GtkWidget *halt_btn;
	GtkWidget *win;
	GString *img2img_file_path;
} GenerationData;

typedef struct {
	GtkWidget *win;
	GtkWidget *image_wgt;
	GString *img2img_file_path;
	GCancellable *cancellable;
} LoadImg2ImgData;

typedef struct {
	GtkWidget *win;
	GtkTextBuffer *pos_tb;
	GtkTextBuffer *neg_tb;
	GtkWidget *steps_dd;
	GtkWidget *cfg_spin;
	GtkWidget *seed_entry;
	GtkWidget *width_dd;
	GtkWidget *height_dd;
	GtkWidget *model_dd;
	GtkWidget *sample_dd;
	GtkWidget *schedule_dd;
	GCancellable *cancellable;
} LoadPNGData;

typedef struct {
	char *var_str;
	const char *str_format;
} NumStrData;

typedef struct {
	GtkWidget *image_widget;
} PreviewImageData;

typedef struct {
	GApplication* app;
	GtkWidget* model_dd;
	GtkWidget* vae_dd;
	GtkWidget* cnet_dd;
	GtkWidget* upscale_dd;
	GtkWidget* clip_l_dd;
	GtkWidget* clip_g_dd;
	GtkWidget* t5xxl_dd;
	GtkWidget* lora_dd;
	GtkWidget* embedding_dd;
} ReloadDropDownData;

typedef struct {
	GtkTextBuffer *pos_tb;
	GtkTextBuffer *neg_tb;
	GtkWidget *model_dd;
	GtkWidget *vae_dd;
	GtkWidget *cnet_dd;
	GtkWidget *upscale_dd;
	GtkWidget *clip_l_dd;
	GtkWidget *clip_g_dd;
	GtkWidget *t5xxl_dd;
	GtkWidget *cfg_spin;
	GtkWidget *denoise_spin;
	GtkWidget *seed_entry;
	GtkWidget *upscale_spin;
	GtkWidget *lora_dd;
	GtkWidget *embedding_dd;
	GtkWidget *sample_dd;
	GtkWidget *schedule_dd;
	GtkWidget *steps_dd;
	GtkWidget *width_dd;
	GtkWidget *height_dd;
	GtkWidget *batch_dd;
	GtkWidget *cpu_check;
	GtkWidget *tiling_check;
	GtkWidget *clip_check;
	GtkWidget *cnet_check;
	GtkWidget *vae_check;
	GtkWidget *flash_check;
} ResetCbData;

typedef struct {
	int img_n;
	int img_t;
	int verbose_bool;
	GtkWidget *button;
	int sdpid;
	GDataInputStream *out_pipe_stream;
} SDProcessOutputData;

typedef struct {
	int verbose_bool;
	GtkWidget *win;
	int sdpid;
	GDataInputStream *err_pipe_stream;
} SDProcessErrorData;

typedef struct {
	long long int *seed;
	GtkWidget *win;
} SeedEntryData;

typedef struct {
	int sd_pid;
	gint stdout_fd;
	char* cmd;
	char* img_name;
	GtkButton* gen_btn;
	GtkWidget* image_widget;
	GtkWidget* show_img_btn;
	GtkWidget* halt_btn;
} StartGenData;

#endif // STRUCTS_H
