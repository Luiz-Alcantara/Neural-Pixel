#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
	GtkApplication *app;
	GtkWidget *window;
	GtkWidget *button;
	GString *checkpoint_string;
	GString *vae_string;
	GString *cnet_string;
	GString *upscaler_string;
	GString *clip_l_string;
	GString *clip_g_string;
	GString *text_enc_string;
	GString *img_index_string;
	GPtrArray *image_files;
	GPtrArray *sd_cmd_array;
	int sdpid;
	int sampler_index;
	int scheduler_index;
	int w_index;
	int h_index;
	int kontext_bool;
	int sd_based_bool;
	int llm_bool;
	int cpu_bool;
	int vt_bool;
	int ram_offload_bool;
	int k_clip_bool;
	int k_cnet_bool;
	int k_vae_bool;
	int fa_bool;
	int taesd_bool;
	int update_cache_bool;
	int verbose_bool;
	int total_time;
	gint current_image_index;
	long long int seed_value;
	double cfg_value;
	double cnet_value;
	double denoise_value;
	double clip_skip_value;
	double up_repeat_value;
	double steps_value;
	double batch_count_value;
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
	int *total_time;
	int *sdpid;
	GPtrArray *image_files;
	gint *current_image_index;
	gchar *result_img_path;
	GPtrArray *sd_cmd_array;
	GString *img_index_string;
	GtkWidget *button;
	GtkWidget *image_widget;
	GtkWidget *img_index_label;
	GtkWidget *show_img_btn;
	GtkWidget *halt_btn;
} EndGenerationData;

typedef struct {
	GString *checkpoint_string;
	GString *vae_string;
	GString *cnet_string;
	GString *upscaler_string;
	GString *clip_l_string;
	GString *clip_g_string;
	GString *text_enc_string;
	GString *img_index_string;
	GPtrArray *image_files;
	GPtrArray *sd_cmd_array;
	int *sdpid;
	int *sampler_index;
	int *scheduler_index;
	int *w_index;
	int *h_index;
	int *kontext_bool;
	int *sd_based_bool;
	int *llm_bool;
	int *cpu_bool;
	int *vt_bool;
	int *ram_offload_bool;
	int *k_clip_bool;
	int *k_cnet_bool;
	int *k_vae_bool;
	int *fa_bool;
	int *taesd_bool;
	int *update_cache_bool;
	int *verbose_bool;
	int *total_time;
	gint *current_image_index;
	long long int *seed_value;
	double *cfg_value;
	double *cnet_value;
	double *denoise_value;
	double *clip_skip_value;
	double *up_repeat_value;
	double *steps_value;
	double *batch_count_value;
	GtkTextBuffer *pos_p;
	GtkTextBuffer *neg_p;
	GtkWidget *image_widget;
	GtkWidget *img_index_label;
	GtkWidget *show_img_btn;
	GtkWidget *halt_btn;
	GtkWidget *win;
	GString *img2img_file_path;
} GenerationData;

typedef struct {
	GtkWidget *win;
	GtkWidget *img2img_expander;
	GtkWidget *image_wgt;
	GString *img2img_file_path;
	GCancellable *cancellable;
} LoadImg2ImgData;

typedef struct {
	gint *current_image_index;
	GPtrArray *image_files;
	GString *img2img_file_path;
	GtkWidget *image_wgt;
	GtkWidget *img2img_expander;
} LoadImg2ImgFromPreviewData;

typedef struct {
	GPtrArray *image_files;
	gint *current_image_index;
	GtkWidget *win;
	GtkTextBuffer *pos_tb;
	GtkTextBuffer *neg_tb;
	GtkWidget *steps_spin;
	GtkWidget *cfg_spin;
	GtkWidget *clip_skip_spin;
	GtkWidget *seed_entry;
	GtkWidget *width_dd;
	GtkWidget *height_dd;
	GtkWidget *checkpoint_dd;
	GtkWidget *vae_dd;
	GtkWidget *sampler_dd;
	GtkWidget *scheduler_dd;
	GCancellable *cancellable;
} LoadPNGData;

typedef struct {
	char *var_str;
	const char *str_format;
} NumStrData;

typedef struct {
	gint *current_image_index;
	GPtrArray *image_files;
	GString *img_index_string;
	GtkWidget *hide_img_btn;
	GtkWidget *image_widget;
	GtkWidget *img_index_label;
} PreviewImageData;

typedef struct {
	GApplication* app;
	GtkWidget* checkpoint_dd;
	GtkWidget* vae_dd;
	GtkWidget* cnet_dd;
	GtkWidget* upscaler_dd;
	GtkWidget* clip_l_dd;
	GtkWidget* clip_g_dd;
	GtkWidget* text_enc_dd;
	GtkWidget* lora_dd;
	GtkWidget* embedding_dd;
} ReloadDropDownData;

typedef struct {
	GtkTextBuffer *pos_tb;
	GtkTextBuffer *neg_tb;
	GtkWidget *checkpoint_dd;
	GtkWidget *vae_dd;
	GtkWidget *cnet_dd;
	GtkWidget *upscaler_dd;
	GtkWidget *clip_l_dd;
	GtkWidget *clip_g_dd;
	GtkWidget *text_enc_dd;
	GtkWidget *cfg_spin;
	GtkWidget *cnet_strength_spin;
	GtkWidget *denoise_spin;
	GtkWidget *seed_entry;
	GtkWidget *upscale_spin;
	GtkWidget *lora_dd;
	GtkWidget *embedding_dd;
	GtkWidget *sampler_dd;
	GtkWidget *scheduler_dd;
	GtkWidget *width_dd;
	GtkWidget *height_dd;
	GtkWidget *steps_spin;
	GtkWidget *batch_count_spin;
	GtkWidget *kontext_check;
	GtkWidget *sd_based_check;
	GtkWidget *llm_check;
	GtkWidget *cpu_check;
	GtkWidget *tiling_check;
	GtkWidget *ram_offload_check;
	GtkWidget *clip_check;
	GtkWidget *cnet_check;
	GtkWidget *vae_check;
	GtkWidget *flash_check;
	GtkWidget *taesd_check;
	GtkWidget *update_cache_check;
	GtkWidget *verbose_check;
} ResetCbData;

typedef struct {
	int is_img2img_encoding;
	int img2img_enc_completed;
	int is_generating_latent;
	int n_current_image;
	int n_total_images;
	int gen_latent_completed;
	int is_decoding_latents;
	int n_current_latent;
	int dec_latents_completed;
	int is_upscaling;
	int n_current_upscale;
	int verbose_bool;
	int *total_time;
	GtkWidget *button;
	int *sdpid;
	GInputStream *out_pipe_stream;
	char read_buffer[4096];
	GString *stdout_string;
} SDProcessOutputData;

typedef struct {
	int verbose_bool;
	GtkWidget *win;
	int *sdpid;
	GDataInputStream *err_pipe_stream;
} SDProcessErrorData;

typedef struct {
	long long int *seed;
	GtkWidget *win;
} SeedEntryData;

typedef struct {
	gchar *new_img_path;
	gint *current_image_index;
	GPtrArray *image_files;
	GString *img_index_string;
	GtkWidget *hide_img_btn;
	GtkWidget *image_widget;
	GtkWidget *img_index_label;
	GtkWidget *to_trash_btn;
} SendTrashData;

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
