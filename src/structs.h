#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
	GtkApplication *app;
	GString *checkpoint_string;
	GString *detector_string;
	GString *vae_string;
	GString *cnet_string;
	GString *upscaler_string;
	GString *clip_l_string;
	GString *clip_g_string;
	GString *text_enc_string;
	GString *preview_label_string;
	GString *img2img_file_path;
	GPtrArray *preview_image_files;
	GPtrArray *sd_cmd_array;
	GQueue *job_queue;
	gboolean is_generating;
	int sdpid;
	int sampler_index;
	int scheduler_index;
	int w_index;
	int h_index;
	int kontext_bool;
	int detector_bool;
	int inpaint_bool;
	int sd_based_bool;
	int llm_bool;
	int hires_upscaler_index;
	int vae_tiling_index;
	int flash_attn_value;
	int mmap_bool;
	int taesd_bool;
	int update_cache_bool;
	int verbose_bool;
	int chroma_dit_mask_bool;
	int qwen_zero_cond_t_bool;
	int model_runtime_backend_index;
	int model_param_backend_index;
	int te_runtime_backend_index;
	int te_param_backend_index;
	int vae_runtime_backend_index;
	int vae_param_backend_index;
	int cnet_runtime_backend_index;
	int cnet_param_backend_index;
	int upscaler_runtime_backend_index;
	int upscaler_param_backend_index;
	int detector_runtime_backend_index;
	int detector_param_backend_index;
	int total_time;
	gint preview_image_index;
	long long int seed_value;
	double cfg_value;
	double cnet_value;
	double denoise_value;
	double detector_confidence_value;
	double detector_denoise_value;
	double detector_inpaint_padding_value;
	double detector_input_size_value;
	double detector_mask_blur_value;
	double clip_skip_value;
	double up_repeat_value;
	double steps_value;
	double batch_count_value;
	double hires_scale_value;
	double hires_steps_value;
	double hires_denoise_value;
} AppStartData;

typedef struct {
	int *sdpid;
	GQueue *job_queue;
	GtkWidget *halt_btn;
} CancelAllData;

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
	gchar *result_img_path;
	gpointer *gen_snapshot_ptr;
} EndGenerationData;

typedef struct {
	AppStartData *app_data;
	GtkTextBuffer *neg_p;
	GtkTextBuffer *pos_p;
	GtkWidget *cancel_all_btn;
	GtkWidget *chroma_dit_mask_check;
	GtkWidget *detector_check;
	GtkWidget *inpaint_check;
	GtkWidget *kontext_check;
	GtkWidget *llm_check;
	GtkWidget *mmap_check;
	GtkWidget *qwen_zero_cond_t_check;
	GtkWidget *sd_based_check;
	GtkWidget *taesd_check;
	GtkWidget *update_cache_check;
	GtkWidget *verbose_check;
	GtkWidget *width_dd;
	GtkWidget *height_dd;
	GtkWidget *sampler_dd;
	GtkWidget *scheduler_dd;
	GtkWidget *hires_upscaler_dd;
	GtkWidget *vae_tiling_dd;
	GtkWidget *model_runtime_backend_dd;
	GtkWidget *model_parameter_backend_dd;
	GtkWidget *te_runtime_backend_dd;
	GtkWidget *te_parameter_backend_dd;
	GtkWidget *vae_runtime_backend_dd;
	GtkWidget *vae_parameter_backend_dd;
	GtkWidget *cnet_runtime_backend_dd;
	GtkWidget *cnet_parameter_backend_dd;
	GtkWidget *upscaler_runtime_backend_dd;
	GtkWidget *upscaler_parameter_backend_dd;
	GtkWidget *detector_runtime_backend_dd;
	GtkWidget *detector_parameter_backend_dd;
	GtkWidget *detector_confidence_spin;
	GtkWidget *detector_denoise_spin;
	GtkWidget *detector_inpaint_padding_spin;
	GtkWidget *detector_input_size_spin;
	GtkWidget *detector_mask_blur_spin;
	GtkWidget *generation_label;
	GtkWidget *steps_spin;
	GtkWidget *batch_count_spin;
	GtkWidget *cfg_spin;
	GtkWidget *denoise_spin;
	GtkWidget *clip_skip_spin;
	GtkWidget *upscale_passes_spin;
	GtkWidget *cnet_strength_spin;
	GtkWidget *hires_scale_spin;
	GtkWidget *hires_steps_spin;
	GtkWidget *hires_denoise_spin;
	GtkWidget *checkpoint_dd;
	GtkWidget *clip_g_dd;
	GtkWidget *clip_l_dd;
	GtkWidget *cnet_dd;
	GtkWidget *detector_dd;
	GtkWidget *text_enc_dd;
	GtkWidget *upscaler_dd;
	GtkWidget *vae_dd;
	GtkWidget *halt_btn;
	GtkWidget *preview_image_widget;
	GtkWidget *preview_label;
	GtkWidget *preview_image_toggle_visibility_btn;
	GtkWidget *queue_size_label;
	GtkWidget *win;
} GenerationData;

typedef struct {
	AppStartData *app_data;
	char *checkpoint_filename;
	char *clip_l_filename;
	char *clip_g_filename;
	char *cnet_filename;
	char *detector_filename;
	char *img2img_file_path;
	char *negative_prompt;
	char *output_path;
	char *positive_prompt;
	char *text_enc_filename;
	char *upscaler_filename;
	char *vae_filename;
	double cfg_scale_value;
	double cnet_strength_value;
	double denoise_strength_value;
	double detector_confidence_value;
	double detector_denoise_value;
	double hires_denoise_value;
	double hires_scale_value;
	gboolean chroma_dit_mask_enabled;
	gboolean detector_enabled;
	gboolean inpaint_enabled;
	gboolean kontext_enabled;
	gboolean llm_mode_enabled;
	gboolean mmap_enabled;
	gboolean qwen_zero_cond_t_enabled;
	gboolean sd_based_enabled;
	gboolean taesd_enabled;
	gboolean update_cache_enabled;
	gboolean verbose_enabled;
	gint *preview_image_index;
	GPtrArray *preview_image_files;
	GPtrArray *sd_cmd_array;
	GString *preview_label_string;
	GtkWidget *cancel_all_btn;
	GtkWidget *checkpoint_dd;
	GtkWidget *generation_label;
	GtkWidget *gen_btn;
	GtkWidget *halt_btn;
	GtkWidget *preview_image_toggle_visibility_btn;
	GtkWidget *preview_input_widget;
	GtkWidget *preview_image_widget;
	GtkWidget *preview_label;
	GtkWidget *queue_size_label;
	GtkWidget *win;
	int batch_count_value;
	int clip_skip_value;
	int cnet_param_backend_index;
	int cnet_runtime_backend_index;
	int detector_inpaint_padding_value;
	int detector_input_size_value;
	int detector_mask_blur_value;
	int detector_param_backend_index;
	int detector_runtime_backend_index;
	int flash_attn_value;
	int height_index;
	int hires_steps_value;
	int hires_upscaler_index;
	int model_param_backend_index;
	int model_runtime_backend_index;
	int sampler_index;
	int scheduler_index;
	int step_count_value;
	int te_param_backend_index;
	int te_runtime_backend_index;
	int total_time;
	int upscale_passes_value;
	int upscaler_param_backend_index;
	int upscaler_runtime_backend_index;
	int vae_param_backend_index;
	int vae_runtime_backend_index;
	int vae_tiling_index;
	int width_index;
	int *sdpid;
	long long int seed_value;
} GenerationSnapshotData;

typedef struct {
	GtkWidget *win;
	GtkWidget *img2img_expander;
	GtkWidget *overlay_img2img;
	GtkWidget *image_wgt;
	GtkWidget *detector_check;
	GtkWidget *inpaint_check;
	GString *img2img_file_path;
	GCancellable *cancellable;
} LoadImg2ImgData;

typedef struct {
	gint *current_image_index;
	GPtrArray *image_files;
	GString *img2img_file_path;
	GtkWidget *image_wgt;
	GtkWidget *img2img_expander;
	GtkWidget *overlay_img2img;
	GtkWidget *detector_check;
	GtkWidget *inpaint_check;
} LoadImg2ImgFromPreviewData;

typedef struct {
	GPtrArray *image_files;
	gint *current_image_index;
	GtkWidget *win;
	GtkTextBuffer *pos_tb;
	GtkTextBuffer *neg_tb;
	GtkWidget *steps_spin;
	GtkWidget *cfg_spin;
	GtkWidget *denoise_spin;
	GtkWidget *clip_skip_spin;
	GtkWidget *seed_entry;
	GtkWidget *width_dd;
	GtkWidget *height_dd;
	GtkWidget *checkpoint_dd;
	GtkWidget *vae_dd;
	GtkWidget *sampler_dd;
	GtkWidget *scheduler_dd;
	GtkWidget *hires_upscaler_dd;
	GtkWidget *hires_scale_spin;
	GtkWidget *hires_steps_spin;
	GtkWidget *hires_denoise_spin;
	GCancellable *cancellable;
} LoadPNGData;

typedef struct {
	GtkWidget *main_win;
	GString *img2img_file_path;
} MaskWinData;

typedef struct {
	GtkWidget *overlay_img2img;
	GtkWidget *detector_check;
	GtkWidget *inpaint_check;
	GtkWidget *drawing_area;
	GtkWidget *brush_width_spin;
	cairo_surface_t *surface;
	double last_x;
	double last_y;
	int ref_w;
	int ref_h;
} MaskData;

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
	int is_hovering_preview;
	PreviewImageData *preview_d;
} PreviewBoxHoverData;

typedef struct {
	int scroll_steps;
	guint debounce_id;
	PreviewImageData *preview_d;
} PreviewBoxScrollData;

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
	GtkWidget *detector_dd;
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
	GtkWidget *upscale_passes_spin;
	GtkWidget *lora_dd;
	GtkWidget *embedding_dd;
	GtkWidget *sampler_dd;
	GtkWidget *scheduler_dd;
	GtkWidget *width_dd;
	GtkWidget *height_dd;
	GtkWidget *hires_upscaler_dd;
	GtkWidget *batch_count_spin;
	GtkWidget *detector_confidence_spin;
	GtkWidget *detector_denoise_spin;
	GtkWidget *detector_inpaint_padding_spin;
	GtkWidget *detector_input_size_spin;
	GtkWidget *detector_mask_blur_spin;
	GtkWidget *hires_scale_spin;
	GtkWidget *hires_steps_spin;
	GtkWidget *hires_denoise_spin;
	GtkWidget *steps_spin;
	GtkWidget *kontext_check;
	GtkWidget *detector_check;
	GtkWidget *inpaint_check;
	GtkWidget *sd_based_check;
	GtkWidget *llm_check;
	GtkWidget *mmap_check;
	GtkWidget *fa_off_btn;
	GtkWidget *taesd_check;
	GtkWidget *update_cache_check;
	GtkWidget *chroma_dit_mask_check;
	GtkWidget *qwen_zero_cond_t_check;
	GtkWidget *verbose_check;
	GtkWidget *vae_tiling_dd;
	GtkWidget *model_runtime_backend_dd;
	GtkWidget *model_parameter_backend_dd;
	GtkWidget *te_runtime_backend_dd;
	GtkWidget *te_parameter_backend_dd;
	GtkWidget *vae_runtime_backend_dd;
	GtkWidget *vae_parameter_backend_dd;
	GtkWidget *cnet_runtime_backend_dd;
	GtkWidget *cnet_parameter_backend_dd;
	GtkWidget *upscaler_runtime_backend_dd;
	GtkWidget *upscaler_parameter_backend_dd;
	GtkWidget *detector_runtime_backend_dd;
	GtkWidget *detector_parameter_backend_dd;
} ResetCbData;

typedef struct {
	char read_buffer[4096];
	int is_img2img_encoding;
	int img2img_enc_completed;
	int is_generating_latent;
	int n_current_image;
	int n_total_images;
	int gen_latent_completed;
	int is_decoding_latents;
	int n_current_latent;
	int dec_latents_completed;
	int n_current_hires;
	int is_hires_fix;
	int is_upscaling;
	int n_current_upscale;
	int *total_time;
	int *sdpid;
	gboolean verbose_enabled;
	GtkWidget *generation_label;
	GtkWidget *button;
	GtkWidget *win;
	GInputStream *out_pipe_stream;
	GString *stdout_string;
} SDProcessOutputData;

typedef struct {
	gboolean verbose_enabled;
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
