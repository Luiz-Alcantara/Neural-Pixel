#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
	GtkApplication *app;
	GtkWidget *window;
	GtkWidget *button;
	int model_index;
	int vae_index;
	int cnet_index;
	int upscale_index;
	int clip_l_index;
	int clip_g_index;
	int t5xxl_index;
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
	double cfg_value;
	double denoise_value;
	double seed_value;
	double up_repeat_value;
	GString *img2img_file_path;
} AppStartData;

typedef struct {
	char *pos_p;
	char *neg_p;
	char *img_name;
} MyCacheData;

typedef struct {
	int *var;
	int req_int;
	GtkWidget *g_btn;
} DropDownConstData;

typedef struct {
	int tb_type;
	GtkTextBuffer *textbuffer;
} DropDownPathData;

typedef struct {
	int pid;
	char *cmd;
	GtkWidget *button;
	GtkWidget *image_widget;
	GtkWidget *show_img_btn;
	GtkWidget *halt_btn;
	char *img_name;
	GIOChannel *channel;
} EndGenerationData;

typedef struct {
	int *model_index;
	int *vae_index;
	int *cnet_index;
	int *upscale_index;
	int *clip_l_index;
	int *clip_g_index;
	int *t5xxl_index;
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
	double *cfg_value;
	double *denoise_value;
	double *seed_value;
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
	GtkWidget *win;
	GtkWidget *image_wgt;
	GString *img2img_file_path;
} LoadImg2ImgData;

typedef struct {
	GtkWidget *win;
	GtkTextBuffer *pos_tb;
	GtkTextBuffer *neg_tb;
	GtkWidget *steps_dd;
	GtkWidget *cfg_spin;
	GtkWidget *seed_spin;
	GtkWidget *width_dd;
	GtkWidget *height_dd;
	GtkWidget *model_dd;
	GtkWidget *sample_dd;
	GtkWidget *schedule_dd;
} LoadPNGData;

typedef struct {
	char *var_str;
	const char *str_format;
} NumStrData;

typedef struct {
	GtkWidget *image_widget;
} PreviewImageData;

typedef struct {
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
	GtkWidget *seed_spin;
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
