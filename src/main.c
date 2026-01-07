#include <dirent.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "constants.h"
#include "dropdown_widgets.h"
#include "file_utils.h"
#include "generate_cb.h"
#include "handle_cache.h"
#include "png_utils.h"
#include "str_utils.h"
#include "structs.h"
#include "widgets_cb.h"

static void
app_activate (GApplication *app, gpointer user_data)
{
	AppStartData *app_data = user_data;
	load_cache(user_data);
	
	PreviewImageData *preview_d;
	preview_d = g_new0 (PreviewImageData, 1);
	
	preview_d->img_index_string = app_data->img_index_string;
	preview_d->image_files = app_data->image_files;
	preview_d->current_image_index = &app_data->current_image_index;

	//Defining GTK Widgets
	GtkWidget *win;

	GtkCssProvider *css_provider;
	
	GtkWidget *titlebar;

	GtkWidget *box;
	
	GtkWidget *box_left;
	
	GtkWidget *properties_scrollable;

	GtkWidget *box_properties, *boxl_topbar;
	GtkWidget *info_btn, *reload_btn, *reset_default_btn, *load_from_img_btn;
	
	GtkWidget *img2img_expander;
	GtkWidget *box_img2img, *box_img2img_buttons, *box_preview_img2img;
	GtkWidget *load_img2img_btn, *clear_img2img_btn;
	GtkWidget *preview_img2img;

	GtkWidget *box_prompts ,*box_pos_prompt, *box_neg_prompt;
	
	GtkWidget *pp_lab;
	GtkWidget *pos_scr;
	GtkWidget *pos_tv;
	GtkTextBuffer *pos_tb;

	GtkWidget *np_lab;
	GtkWidget *neg_scr;
	GtkWidget *neg_tv;
	GtkTextBuffer *neg_tb;

	GtkWidget *box_checkpoint, *box_checkpoint_buttons;
	GtkWidget *generate_btn;
	GtkWidget *checkpoint_lab, *checkpoint_dd, *sd_based_check;
	GtkWidget *vae_lab, *vae_dd;

	GtkWidget *addons_expander;
	GtkWidget *box_addons;
	GtkWidget *cnet_lab, *cnet_dd;
	GtkWidget *upscaler_lab, *upscaler_dd;
	GtkWidget *clip_l_lab, *clip_l_dd;
	GtkWidget *clip_g_lab, *clip_g_dd;
	GtkWidget *box_text_encoder, *box_text_encoder_buttons, *text_enc_lab, *text_enc_dd, *llm_check;
	
	GtkWidget *box_params;
	GtkWidget *box_params_row1, *box_params_row1_col1, *box_params_row1_col2;
	GtkWidget *box_params_row2, *box_params_row2_col1, *box_params_row2_col2;
	GtkWidget *box_params_row3, *box_params_row3_col1, *box_params_row3_col2;
	GtkWidget *box_params_row4, *box_params_row4_col1, *box_params_row4_col2;
	GtkWidget *box_params_row5;
	GtkWidget *box_params_row6, *box_params_row6_col1, *box_params_row6_col2;
	
	GtkWidget *box_model_adapters;
	GtkWidget *lora_lab, *lora_dd;
	GtkWidget *embedding_lab, *embedding_dd;

	GtkWidget *width_lab, *width_dd;
	GtkWidget *height_lab, *height_dd;
	GtkWidget *steps_lab, *steps_spin;
	GtkWidget *batch_count_lab, *batch_count_spin;
	GtkWidget *sampler_lab, *sampler_dd;
	GtkWidget *scheduler_lab, *scheduler_dd;
	GtkWidget *cfg_lab, *cfg_spin;
	GtkWidget *denoise_lab, *denoise_spin;
	GtkWidget *seed_lab, *seed_entry;
	GtkWidget *clip_skip_lab, *clip_skip_spin;
	GtkWidget *upscale_str_lab, *upscale_spin;
	
	GtkWidget *extra_opts_expander, *box_extra_opts, *box_extra_opts_col1, *box_extra_opts_col2;
	
	GtkWidget *cpu_check, *tiling_check, *ram_offload_check, *clip_check, *cnet_check, *vae_check, *flash_check, *taesd_check, *update_cache_check, *verbose_check;

	GtkWidget *box_generation;
	GtkWidget *sd_halt_btn;

	GtkWidget *box_right, *boxr_img, *boxr_bottom_bar, *boxr_bottom_left_spacer, *boxr_bottom_right_spacer;
	GtkWidget *preview_img;
	GtkWidget *prev_img_button, *img_index_label, *next_img_button, *load_from_current_btn, *set_img2img_from_preview_btn, *hide_img_btn;

	ReloadDropDownData *reload_d;
	ResetCbData *reset_d;
	GenerationData *gen_d;
	LoadPNGData *load_png_info_d;
	LoadImg2ImgData *load_img2img_file_d;
	LoadImg2ImgFromPreviewData *load_img2img_from_preview_d;
	SeedEntryData *seed_entry_d;
	//End defining GTK Widgets;

	win = gtk_application_window_new (GTK_APPLICATION (app));
	gtk_window_set_title (GTK_WINDOW (win), "Neural Pixel");
	gtk_window_set_default_size (GTK_WINDOW (win), 900, 800);
	
	//Load css file 
	css_provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_file (css_provider, g_file_new_for_path ("resources/styles.css"));
	gtk_style_context_add_provider_for_display (gtk_widget_get_display (win), GTK_STYLE_PROVIDER (css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	
	titlebar = gtk_header_bar_new ();
	gtk_widget_add_css_class(titlebar, "titlebar");
	gtk_window_set_titlebar (GTK_WINDOW (win), titlebar);
	
	//Main box
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, ZERO_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box), FALSE);
	gtk_window_set_child (GTK_WINDOW (win), box);
		
	//Set Box Left and Right
	box_left = gtk_box_new (GTK_ORIENTATION_VERTICAL, LARGE_SPACING);
	gtk_widget_add_css_class(box_left, "left_box");
	gtk_widget_set_size_request(GTK_WIDGET(box_left), 460, -1);
	gtk_widget_set_hexpand(box_left, FALSE);
	gtk_box_append (GTK_BOX (box), box_left);
	box_right = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_widget_add_css_class(box_right, "right_box");
	gtk_box_append (GTK_BOX (box), box_right);

	info_btn = gtk_button_new_from_icon_name ("dialog-information-symbolic");
	gtk_widget_add_css_class(info_btn, "custom_btn_title");
	gtk_widget_set_tooltip_text(GTK_WIDGET(info_btn),
	"About Neural Pixel.");
	
	reload_btn = gtk_button_new_from_icon_name ("view-refresh-symbolic");
	gtk_widget_add_css_class(reload_btn, "custom_btn_title");
	gtk_widget_set_tooltip_text(GTK_WIDGET(reload_btn),
	"Detect newly added files (models, LoRAs, embeddings, etc.) and refresh all available file lists.");
	
	reset_default_btn = gtk_button_new_from_icon_name ("document-revert-symbolic");
	gtk_widget_add_css_class(reset_default_btn, "custom_btn_title");
	gtk_widget_set_tooltip_text(GTK_WIDGET(reset_default_btn),
	"Reset all settings to their default values.");
	
	load_from_img_btn = gtk_button_new_from_icon_name ("insert-image-symbolic");
	gtk_widget_add_css_class(load_from_img_btn, "custom_btn_title");
	gtk_widget_set_tooltip_text(GTK_WIDGET(load_from_img_btn),
	"Load the prompt, model, sampler settings, and other parameters\nfrom the embedded metadata of the selected image file.");
	
	gtk_header_bar_pack_start ( GTK_HEADER_BAR (titlebar), info_btn);
	gtk_header_bar_pack_start ( GTK_HEADER_BAR (titlebar), reload_btn);
	gtk_header_bar_pack_start ( GTK_HEADER_BAR (titlebar), reset_default_btn);
	gtk_header_bar_pack_start ( GTK_HEADER_BAR (titlebar), load_from_img_btn);
	
	//Properties Scrollable
	properties_scrollable = gtk_scrolled_window_new();
	gtk_scrolled_window_set_placement(GTK_SCROLLED_WINDOW (properties_scrollable), GTK_CORNER_BOTTOM_RIGHT);
	gtk_box_append (GTK_BOX (box_left), properties_scrollable);
	
	// Set Properties Box
	box_properties = gtk_box_new (GTK_ORIENTATION_VERTICAL, HUGE_SPACING);
	gtk_widget_set_hexpand (box_properties, TRUE);
	gtk_widget_set_margin_bottom (box_properties, MEDIUM_SPACING);
	gtk_widget_set_margin_end (box_properties, LARGE_SPACING);
	gtk_widget_set_margin_start (box_properties, LARGE_SPACING);
	gtk_widget_set_margin_top (box_properties, MEDIUM_SPACING);
	gtk_widget_add_css_class(box_properties, "left_box");
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW (properties_scrollable), box_properties);
	
	//Set IMG2IMG Widgets
	img2img_expander = gtk_expander_new ("IMG2IMG");
	gtk_widget_add_css_class(img2img_expander, "param_label");
	gtk_box_append (GTK_BOX (box_properties), img2img_expander);
	
	box_img2img = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_widget_add_css_class(box_img2img, "inner_box");
	gtk_box_set_homogeneous (GTK_BOX (box_img2img), FALSE);
	gtk_expander_set_child(GTK_EXPANDER(img2img_expander), box_img2img);
	
	box_img2img_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_img2img_buttons), TRUE);
	gtk_box_append (GTK_BOX (box_img2img), box_img2img_buttons);
	
	load_img2img_btn = gtk_button_new_with_label ("Load img2img");
	gtk_widget_add_css_class(load_img2img_btn, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(load_img2img_btn),
	"Select the image that will be used as a template for image-to-image processing.");
	gtk_box_append (GTK_BOX (box_img2img_buttons), load_img2img_btn);
	
	clear_img2img_btn = gtk_button_new_with_label ("Disable Img2Img");
	gtk_widget_add_css_class(clear_img2img_btn, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(clear_img2img_btn),
	"Clear the source image to stop using it as a template.");
	gtk_box_append (GTK_BOX (box_img2img_buttons), clear_img2img_btn);
	
	box_preview_img2img = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_widget_set_size_request(box_preview_img2img, 412, 400);
	gtk_widget_set_hexpand (box_preview_img2img, FALSE);
	gtk_widget_set_vexpand (box_preview_img2img, FALSE);
	gtk_box_append (GTK_BOX (box_img2img), box_preview_img2img);
	
	preview_img2img = gtk_picture_new_for_filename(EMPTY_IMG_PATH);
	gtk_box_append (GTK_BOX (box_preview_img2img), preview_img2img);

	//Set Prompts Box
	box_prompts = gtk_box_new (GTK_ORIENTATION_VERTICAL, LARGE_SPACING);
	gtk_widget_set_size_request(box_prompts, 412, 500);
	gtk_widget_set_hexpand (box_prompts, TRUE);
	gtk_box_set_homogeneous (GTK_BOX (box_prompts), TRUE);
	gtk_widget_add_css_class(box_prompts, "inner_box");
	gtk_box_append (GTK_BOX (box_properties), box_prompts);
	
	box_pos_prompt = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_prompts), box_pos_prompt);
	
	box_neg_prompt = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_prompts), box_neg_prompt);

	//Set Prompts Box Labels
	pp_lab = gtk_label_new ("Positive Prompt");
	np_lab = gtk_label_new ("Negative Prompt");
	
	gtk_widget_set_halign(pp_lab, LABEL_ALIGNMENT);
	gtk_widget_set_halign(np_lab, LABEL_ALIGNMENT);
	
	gtk_widget_add_css_class(pp_lab, "pos_prompt_label");
	gtk_widget_add_css_class(np_lab, "neg_prompt_label");

	gtk_box_append (GTK_BOX (box_pos_prompt), pp_lab);
	gtk_box_append (GTK_BOX (box_neg_prompt), np_lab);

	//Set positive textview
	pos_scr = gtk_scrolled_window_new ();
	pos_tv = gtk_text_view_new ();
	gtk_widget_add_css_class(pos_tv, "custom_entry");
	pos_tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pos_tv));
	
	gtk_widget_set_hexpand (pos_tv, TRUE);
	gtk_widget_set_vexpand (pos_tv, TRUE);
	load_pp_cache(pos_tb);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (pos_tv), GTK_WRAP_WORD_CHAR);
	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (pos_scr), pos_tv);

	gtk_box_append (GTK_BOX (box_pos_prompt), pos_scr);

	//Set negative textview
	neg_scr = gtk_scrolled_window_new ();
	neg_tv = gtk_text_view_new ();
	gtk_widget_add_css_class(neg_tv, "custom_entry");
	neg_tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (neg_tv));
	
	gtk_widget_set_hexpand (neg_tv, TRUE);
	gtk_widget_set_vexpand (neg_tv, TRUE);
	load_np_cache(neg_tb);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (neg_tv), GTK_WRAP_WORD_CHAR);
	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (neg_scr), neg_tv);

	gtk_box_append (GTK_BOX (box_neg_prompt), neg_scr);

	//Set Checkpoint Widgets
	box_checkpoint = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_widget_add_css_class(box_checkpoint, "inner_box");
	gtk_box_append (GTK_BOX (box_properties), box_checkpoint);
	
	checkpoint_lab = gtk_label_new ("Checkpoint");
	gtk_widget_set_halign(checkpoint_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(checkpoint_lab, "param_label");
	gtk_box_append (GTK_BOX (box_checkpoint), checkpoint_lab);
	
	box_checkpoint_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_checkpoint), box_checkpoint_buttons);
	
	generate_btn = gtk_button_new_with_label ("Generate");
	gtk_widget_add_css_class(generate_btn, "gen_btn");
	
	checkpoint_dd = gen_path_dd(CHECKPOINTS_PATH, DD_STRING_LEN, NULL, 0, app_data->checkpoint_string, generate_btn, app, 1);
	gtk_widget_set_hexpand (checkpoint_dd, TRUE);
	gtk_box_append (GTK_BOX (box_checkpoint_buttons), checkpoint_dd);
	
	sd_based_check = gtk_check_button_new_with_label("SD Ckpt");
	gtk_widget_add_css_class(sd_based_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(sd_based_check), app_data->sd_based_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(sd_based_check), "Uncheck for non Stable Diffusion based checkpoints\n(e.g., Flux, Chroma, Qwen).");
	g_signal_connect(sd_based_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->sd_based_bool);
	gtk_box_append (GTK_BOX (box_checkpoint_buttons), sd_based_check);
	
	//Set add-ons Widgets
	
	addons_expander = gtk_expander_new ("Extra Addons");
	gtk_widget_add_css_class(addons_expander, "param_label");
	gtk_box_append (GTK_BOX (box_properties), addons_expander);
	
	box_addons = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_addons), FALSE);
	gtk_widget_add_css_class(box_addons, "inner_box");
	gtk_expander_set_child(GTK_EXPANDER(addons_expander), box_addons);
	
	//Set VAE Widgets

	vae_lab = gtk_label_new ("VAE");
	gtk_widget_set_halign(vae_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(vae_lab, "param_label");
	gtk_box_append (GTK_BOX (box_addons), vae_lab);

	vae_dd = gen_path_dd(VAES_PATH, DD_STRING_LEN, NULL, 0, app_data->vae_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_addons), vae_dd);

	//Set CNet Widgets
	
	cnet_lab = gtk_label_new ("Control Net");
	gtk_widget_set_halign(cnet_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(cnet_lab, "param_label");
	gtk_box_append (GTK_BOX (box_addons), cnet_lab);
	
	cnet_dd = gen_path_dd(CONTROLNET_PATH, DD_STRING_LEN, NULL, 0, app_data->cnet_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_addons), cnet_dd);
	
	//Set Upscaler Widgets

	upscaler_lab = gtk_label_new ("Upscaler");
	gtk_widget_set_halign(upscaler_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(upscaler_lab, "param_label");
	gtk_box_append (GTK_BOX (box_addons), upscaler_lab);

	upscaler_dd = gen_path_dd(UPSCALES_PATH, DD_STRING_LEN, NULL, 0, app_data->upscaler_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_addons), upscaler_dd);

	//Set Clip_l Widgets
	
	clip_l_lab = gtk_label_new ("Clip_l");
	gtk_widget_set_halign(clip_l_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(clip_l_lab, "param_label");
	gtk_box_append (GTK_BOX (box_addons), clip_l_lab);
	
	clip_l_dd = gen_path_dd(CLIPS_PATH, DD_STRING_LEN, NULL, 0, app_data->clip_l_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_addons), clip_l_dd);
	
	//Set Clip_g Widgets
	
	clip_g_lab = gtk_label_new ("Clip_g");
	gtk_widget_set_halign(clip_g_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(clip_g_lab, "param_label");
	gtk_box_append (GTK_BOX (box_addons), clip_g_lab);
	
	clip_g_dd = gen_path_dd(CLIPS_PATH, DD_STRING_LEN, NULL, 0, app_data->clip_g_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_addons), clip_g_dd);
	
	//Set Text Encoder Widgets
	
	box_text_encoder = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_addons), box_text_encoder);
	
	text_enc_lab = gtk_label_new ("Text Encoder(T5xxl/LLM)");
	gtk_widget_set_halign(text_enc_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(text_enc_lab, "param_label");
	gtk_box_append (GTK_BOX (box_text_encoder), text_enc_lab);
	
	box_text_encoder_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_text_encoder), box_text_encoder_buttons);
	
	text_enc_dd = gen_path_dd(TEXT_ENCODERS_PATH, DD_STRING_LEN, NULL, 0, app_data->text_enc_string, NULL, app, 0);
	gtk_widget_set_hexpand (text_enc_dd, TRUE);
	gtk_box_append (GTK_BOX (box_text_encoder_buttons), text_enc_dd);
	
	llm_check = gtk_check_button_new_with_label("LLM");
	gtk_widget_add_css_class(llm_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(llm_check), app_data->llm_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(llm_check), "Enables LLM-based text encoding instead of the default diffusion-model text encoder.");
	g_signal_connect(llm_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->llm_bool);
	gtk_box_append (GTK_BOX (box_text_encoder_buttons), llm_check);
	
	//Set Model adapters Widgets
	
	box_model_adapters = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_model_adapters), FALSE);
	gtk_widget_add_css_class(box_model_adapters, "inner_box");
	gtk_box_append (GTK_BOX (box_properties), box_model_adapters);

	//Set LoRA Widgets

	lora_lab = gtk_label_new ("Add LoRA");
	gtk_widget_set_halign(lora_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(lora_lab, "param_label");
	gtk_box_append (GTK_BOX (box_model_adapters), lora_lab);
	
	lora_dd = gen_path_dd(LORAS_PATH, DD_STRING_LEN, pos_tb, 1, NULL, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_model_adapters), lora_dd);

	//Set Embeddings Widgets

	embedding_lab = gtk_label_new ("Add Embedding");
	gtk_widget_set_halign(embedding_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(embedding_lab, "param_label");
	gtk_box_append (GTK_BOX (box_model_adapters), embedding_lab);

	embedding_dd = gen_path_dd(EMBEDDINGS_PATH, DD_STRING_LEN, neg_tb, 0, NULL, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_model_adapters), embedding_dd);

	//Set Parameters Widgets
	box_params = gtk_box_new (GTK_ORIENTATION_VERTICAL, LARGE_SPACING);
	gtk_widget_add_css_class(box_params, "inner_box");
	gtk_box_append (GTK_BOX (box_properties), box_params);
	
	//Set Parameters First Row Widgets
	
	box_params_row1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_params_row1), TRUE);
	gtk_widget_set_hexpand (box_params_row1, TRUE);
	gtk_box_append (GTK_BOX (box_params), box_params_row1);

	box_params_row1_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params_row1), box_params_row1_col1);

	box_params_row1_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params_row1), box_params_row1_col2);
	
	//Set Width Widgets
	
	width_lab = gtk_label_new ("Width");
	gtk_widget_add_css_class(width_lab, "param_label");
	gtk_widget_set_halign(width_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_params_row1_col1), width_lab);
	
	width_dd = gen_const_dd(LIST_RESOLUTIONS_STR, &app_data->w_index);
	gtk_box_append (GTK_BOX (box_params_row1_col1), width_dd);
	
	//Set Height Widgets
	
	height_lab = gtk_label_new ("Height");
	gtk_widget_add_css_class(height_lab, "param_label");
	gtk_widget_set_halign(height_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_params_row1_col2), height_lab);
	
	height_dd = gen_const_dd(LIST_RESOLUTIONS_STR, &app_data->h_index);
	gtk_box_append (GTK_BOX (box_params_row1_col2), height_dd);
	
	//Set Parameters Second Row Widgets
	
	box_params_row2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_params_row2), TRUE);
	gtk_widget_set_hexpand (box_params_row2, TRUE);
	gtk_box_append (GTK_BOX (box_params), box_params_row2);

	box_params_row2_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params_row2), box_params_row2_col1);

	box_params_row2_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params_row2), box_params_row2_col2);
	
	//Set Steps Widgets
	
	steps_lab = gtk_label_new ("Steps");
	gtk_widget_add_css_class(steps_lab, "param_label");
	gtk_widget_set_halign(steps_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_params_row2_col1), steps_lab);
	
	steps_spin = gtk_spin_button_new_with_range (1.0, 150.0, 1.0);
	gtk_widget_add_css_class(steps_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(steps_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(steps_spin), app_data->steps_value);
	g_signal_connect (steps_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->steps_value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(steps_spin),
	"Number of refinement steps. More steps = higher detail but slower.\n20–40 is usually best.");
	gtk_box_append (GTK_BOX (box_params_row2_col1), steps_spin);
	
	//Set Batch count Widgets
	
	batch_count_lab = gtk_label_new ("Batch count");
	gtk_widget_add_css_class(batch_count_lab, "param_label");
	gtk_widget_set_halign(batch_count_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_params_row2_col2), batch_count_lab);

	batch_count_spin = gtk_spin_button_new_with_range (1.0, 50.0, 1.0);
	gtk_widget_add_css_class(batch_count_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(batch_count_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(batch_count_spin), app_data->batch_count_value);
	g_signal_connect (batch_count_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->batch_count_value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(batch_count_spin),
	"How many images to generate one after another.\nDoesn’t use extra VRAM.");
	gtk_box_append (GTK_BOX (box_params_row2_col2), batch_count_spin);
	
	//Set Parameters Third Row Widgets
	
	box_params_row3 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_params_row3), TRUE);
	gtk_widget_set_hexpand (box_params_row3, TRUE);
	gtk_box_append (GTK_BOX (box_params), box_params_row3);

	box_params_row3_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params_row3), box_params_row3_col1);

	box_params_row3_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params_row3), box_params_row3_col2);
	
	//Set Sampler Widgets
	
	sampler_lab = gtk_label_new ("Sampler");
	gtk_widget_add_css_class(sampler_lab, "param_label");
	gtk_widget_set_halign(sampler_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_params_row3_col1), sampler_lab);
	
	sampler_dd = gen_const_dd(LIST_SAMPLES, &app_data->sampler_index);
	gtk_box_append (GTK_BOX (box_params_row3_col1), sampler_dd);
	
	//Set Scheduler Widgets
	
	scheduler_lab = gtk_label_new ("Scheduler");
	gtk_widget_add_css_class(scheduler_lab, "param_label");
	gtk_widget_set_halign(scheduler_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_params_row3_col2), scheduler_lab);
	
	scheduler_dd = gen_const_dd(LIST_SCHEDULES, &app_data->scheduler_index);
	gtk_box_append (GTK_BOX (box_params_row3_col2), scheduler_dd);
	
	//Set Parameters Fourth Row Widgets
	
	box_params_row4 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_params_row4), TRUE);
	gtk_widget_set_hexpand (box_params_row4, TRUE);
	gtk_box_append (GTK_BOX (box_params), box_params_row4);

	box_params_row4_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params_row4), box_params_row4_col1);

	box_params_row4_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params_row4), box_params_row4_col2);
	
	//Set CFG Widgets
	
	cfg_lab = gtk_label_new ("CFG Scale");
	gtk_widget_set_halign(cfg_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(cfg_lab, "param_label");
	gtk_box_append (GTK_BOX (box_params_row4_col1), cfg_lab);
	
	cfg_spin = gtk_spin_button_new_with_range (1, 30.0, 0.5);
	gtk_widget_add_css_class(cfg_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(cfg_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), app_data->cfg_value);
	g_signal_connect (cfg_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->cfg_value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(cfg_spin),
	"Controls how strongly the prompt influences the generated image.\nHigher values make the output more closely follow the prompt.\nTypical range: 6-12.");
	gtk_box_append (GTK_BOX (box_params_row4_col1), cfg_spin);
	
	//Set Denoise Widgets
	
	denoise_lab = gtk_label_new ("Denoise Str");
	gtk_widget_set_halign(denoise_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(denoise_lab, "param_label");
	gtk_box_append (GTK_BOX (box_params_row4_col2), denoise_lab);
	
	denoise_spin = gtk_spin_button_new_with_range (0, 1.0, 0.05);
	gtk_widget_add_css_class(denoise_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(denoise_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(denoise_spin), app_data->denoise_value);
	g_signal_connect (denoise_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->denoise_value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(denoise_spin),
	"Controls how much the original image is changed.\nValues near 1.0 heavily modify or replace the image.\nValues near 0.0 preserve the original image with minimal changes.");
	gtk_box_append (GTK_BOX (box_params_row4_col2), denoise_spin);
	
	//Set Parameters Fifth Row Widgets
	
	box_params_row5 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params), box_params_row5);
	
	//Set Seed Widgets
	
	seed_lab = gtk_label_new ("Seed");
	gtk_widget_set_halign(seed_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(seed_lab, "param_label");
	gtk_box_append (GTK_BOX (box_params_row5), seed_lab);
	
	seed_entry = gtk_entry_new();
	gtk_widget_add_css_class(seed_entry, "custom_seed_entry");
	gtk_entry_set_icon_from_icon_name(GTK_ENTRY(seed_entry), GTK_ENTRY_ICON_SECONDARY, "media-playlist-shuffle-symbolic");
	gtk_entry_set_icon_activatable(GTK_ENTRY(seed_entry), GTK_ENTRY_ICON_SECONDARY, TRUE);
	gtk_entry_set_icon_tooltip_text(GTK_ENTRY(seed_entry), GTK_ENTRY_ICON_SECONDARY, "Sets to -1 = random seed");
	g_signal_connect(seed_entry, "icon-release", G_CALLBACK(random_seed_btn_toggle), NULL);
	char seed_str[LONGLONG_STR_SIZE];
	snprintf(seed_str, sizeof(seed_str), "%lld", app_data->seed_value);
	gtk_editable_set_text(GTK_EDITABLE(seed_entry), seed_str);
	gtk_box_append (GTK_BOX (box_params_row5), seed_entry);
	
	//Set Parameters Sixth Row Widgets
	
	box_params_row6 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_params_row6), TRUE);
	gtk_widget_set_hexpand (box_params_row6, TRUE);
	gtk_box_append (GTK_BOX (box_params), box_params_row6);

	box_params_row6_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params_row6), box_params_row6_col1);

	box_params_row6_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params_row6), box_params_row6_col2);
	
	//Set Clip Skip Widgets
	
	clip_skip_lab = gtk_label_new ("Clip Skip");
	gtk_widget_set_halign(clip_skip_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(clip_skip_lab, "param_label");
	gtk_box_append (GTK_BOX (box_params_row6_col1), clip_skip_lab);
	
	clip_skip_spin = gtk_spin_button_new_with_range (0, 12.0, 1.0);
	gtk_widget_add_css_class(clip_skip_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(clip_skip_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(clip_skip_spin), app_data->clip_skip_value);
	g_signal_connect (clip_skip_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->clip_skip_value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(clip_skip_spin),
	"Skips the last N layers of the text encoder output, reducing the influence of later CLIP layers.\nIf set to 0, the optimal value is automatically selected based on the checkpoint model.");
	gtk_box_append (GTK_BOX (box_params_row6_col1), clip_skip_spin);

	//Set Repeat Upscale Widgets

	upscale_str_lab = gtk_label_new ("Upscale Runs");
	gtk_widget_set_halign(upscale_str_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(upscale_str_lab, "param_label");
	gtk_box_append (GTK_BOX (box_params_row6_col2), upscale_str_lab);
	
	upscale_spin = gtk_spin_button_new_with_range (1.0, 8.0, 1.0);
	gtk_widget_add_css_class(upscale_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(upscale_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(upscale_spin), app_data->up_repeat_value);
	g_signal_connect (upscale_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->up_repeat_value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(upscale_spin),
	"Number of times to run the upscaler sequentially.\nEach run applies the upscale process to the previous result,\nfurther increasing the image dimensions.");
	gtk_box_append (GTK_BOX (box_params_row6_col2), upscale_spin);
	
	//Set Extra Options Widgets
	
	extra_opts_expander = gtk_expander_new ("Extra Options");
	gtk_widget_add_css_class(extra_opts_expander, "param_label");
	gtk_box_append (GTK_BOX (box_properties), extra_opts_expander);
	
	box_extra_opts = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_extra_opts), TRUE);
	gtk_widget_add_css_class(box_extra_opts, "inner_box");
	gtk_expander_set_child(GTK_EXPANDER(extra_opts_expander), box_extra_opts);
	
	box_extra_opts_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_extra_opts_col1), TRUE);
	gtk_box_append (GTK_BOX (box_extra_opts), box_extra_opts_col1);
	
	box_extra_opts_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_extra_opts_col2), TRUE);
	gtk_box_append (GTK_BOX (box_extra_opts), box_extra_opts_col2);
	
	//Set Extra Options Widgets
		
	cpu_check = gtk_check_button_new_with_label("Run in CPU");
	gtk_widget_add_css_class(cpu_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(cpu_check), app_data->cpu_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(cpu_check), "Run all the processing using your CPU.\n(Very Slow)");
	g_signal_connect(cpu_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->cpu_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col1), cpu_check);

	tiling_check = gtk_check_button_new_with_label("VAE Tiling");
	gtk_widget_add_css_class(tiling_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(tiling_check), app_data->vt_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(tiling_check), "Process vae in tiles to reduce memory usage.");
	g_signal_connect(tiling_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->vt_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col1), tiling_check);
	
	ram_offload_check = gtk_check_button_new_with_label("RAM Offload");
	gtk_widget_add_css_class(ram_offload_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(ram_offload_check), app_data->ram_offload_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(ram_offload_check),
	"Place the weights in RAM to save VRAM, and automatically\nload them into VRAM when needed.");
	g_signal_connect(ram_offload_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->ram_offload_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col1), ram_offload_check);

	clip_check = gtk_check_button_new_with_label("Keep Clip in CPU");
	gtk_widget_add_css_class(clip_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(clip_check), app_data->k_clip_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(clip_check), "Keep clip in cpu (for low vram).");
	g_signal_connect(clip_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->k_clip_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col1), clip_check);

	cnet_check = gtk_check_button_new_with_label("Keep CNet in CPU");
	gtk_widget_add_css_class(cnet_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(cnet_check), app_data->k_cnet_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(cnet_check), "Keep controlnet in cpu (for low vram).");
	g_signal_connect(cnet_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->k_cnet_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col1), cnet_check);

	vae_check = gtk_check_button_new_with_label("Keep VAE in CPU");
	gtk_widget_add_css_class(vae_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(vae_check), app_data->k_vae_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(vae_check), "Keep vae in cpu (for low vram).");
	g_signal_connect(vae_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->k_vae_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col2), vae_check);

	flash_check = gtk_check_button_new_with_label("Flash Attention");
	gtk_widget_add_css_class(flash_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(flash_check), app_data->fa_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(flash_check), "Use Flash Attention in the diffusion model.");
	g_signal_connect(flash_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->fa_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col2), flash_check);
	
	taesd_check = gtk_check_button_new_with_label("Enable TAESD");
	gtk_widget_add_css_class(taesd_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(taesd_check), app_data->taesd_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(taesd_check), "Use Tiny AutoEncoder for fast decoding\n(low quality).");
	g_signal_connect(taesd_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->taesd_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col2), taesd_check);
	
	update_cache_check = gtk_check_button_new_with_label("Update Cache");
	gtk_widget_add_css_class(update_cache_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(update_cache_check), app_data->update_cache_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(update_cache_check), "Uncheck to prevent settings from saving in cache; resets on restart.");
	g_signal_connect(update_cache_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->update_cache_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col2), update_cache_check);
	
	verbose_check = gtk_check_button_new_with_label("Terminal Verbose");
	gtk_widget_add_css_class(verbose_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(verbose_check), app_data->verbose_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(verbose_check), "Print verbose on terminal.");
	g_signal_connect(verbose_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->verbose_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col2), verbose_check);

	//Set Generation Box
	box_generation = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_generation), FALSE);
	gtk_box_append (GTK_BOX (box_left), box_generation);

	sd_halt_btn = gtk_button_new_from_icon_name ("process-stop-symbolic");
	gtk_widget_add_css_class(sd_halt_btn, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(sd_halt_btn), "Stops the sd.cpp process.");
	g_signal_connect (sd_halt_btn, "clicked", G_CALLBACK (kill_stable_diffusion_process), &app_data->sdpid);
	gtk_widget_set_sensitive(GTK_WIDGET(sd_halt_btn), FALSE);
	gtk_box_append (GTK_BOX (box_generation), sd_halt_btn);

	gtk_widget_set_hexpand (generate_btn, TRUE);
	gtk_box_append (GTK_BOX (box_generation), generate_btn);

	//Set Box Right Image
	boxr_img = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (boxr_img), TRUE);
	gtk_widget_set_hexpand (boxr_img, TRUE);
	gtk_widget_set_vexpand (boxr_img, TRUE);
	gtk_box_append (GTK_BOX (box_right), boxr_img);

	if (app_data->image_files->len > 0) {
		preview_img = gtk_picture_new_for_filename
		(g_ptr_array_index(app_data->image_files, app_data->current_image_index));
	} else {
		g_printerr("No images in 'outputs' directory.\n");
		preview_img = gtk_picture_new_for_filename
		(DEFAULT_IMG_PATH);
	}
	gtk_box_append (GTK_BOX (boxr_img), preview_img);
	
	//Set Box Right Button bar bottom
	boxr_bottom_bar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, ZERO_SPACING);
	gtk_widget_add_css_class(boxr_bottom_bar, "left_box");
	gtk_box_set_homogeneous (GTK_BOX (boxr_bottom_bar), FALSE);
	gtk_box_append (GTK_BOX (box_right), boxr_bottom_bar);
	
	//Set Box Right Button Bar Left Spacer
	boxr_bottom_left_spacer = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_widget_set_hexpand(boxr_bottom_left_spacer, TRUE);
	gtk_box_append (GTK_BOX (boxr_bottom_bar), boxr_bottom_left_spacer);
	
	//Previous Img Button
	prev_img_button = gtk_button_new_from_icon_name ("go-previous-symbolic");
	gtk_widget_add_css_class(prev_img_button, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(prev_img_button), "Previous Image.");
	gtk_widget_set_size_request(GTK_WIDGET(prev_img_button), 120, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_bar), prev_img_button);
	
	//Next Img Button
	next_img_button = gtk_button_new_from_icon_name ("go-next-symbolic");
	gtk_widget_add_css_class(next_img_button, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(next_img_button), "Next Image.");
	gtk_widget_set_size_request(GTK_WIDGET(next_img_button), 120, -1);
	gtk_box_append(GTK_BOX(boxr_bottom_bar), next_img_button);
	
	//Img index label
	img_index_label = gtk_label_new (app_data->img_index_string->str);
	gtk_widget_set_size_request(GTK_WIDGET(img_index_label), 120, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_bar), img_index_label);
	
	//Load info from current preview image
	load_from_current_btn = gtk_button_new_from_icon_name ("insert-image-symbolic");
	gtk_widget_add_css_class(load_from_current_btn, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(load_from_current_btn), "Load prompt and generation parameters from the displayed image.");
	gtk_widget_set_size_request(GTK_WIDGET(load_from_current_btn), 80, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_bar), load_from_current_btn);
	
	//Set img2img file from current preview image
	set_img2img_from_preview_btn = gtk_button_new_from_icon_name ("x-office-drawing-symbolic");
	gtk_widget_add_css_class(set_img2img_from_preview_btn, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(set_img2img_from_preview_btn),
	"Sets the current preview image as the template for image-to-image processing.");
	gtk_widget_set_size_request(GTK_WIDGET(set_img2img_from_preview_btn), 80, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_bar), set_img2img_from_preview_btn);
	
	//Hide Img Button
	hide_img_btn = gtk_button_new_from_icon_name ("view-reveal-symbolic");
	gtk_widget_add_css_class(hide_img_btn, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(hide_img_btn), "Toggle displayed image visibility.");
	gtk_widget_set_size_request(GTK_WIDGET(hide_img_btn), 80, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_bar), hide_img_btn);
	
	//Set Box Right Button Bar Right Spacer
	boxr_bottom_right_spacer = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_widget_set_hexpand(boxr_bottom_right_spacer, TRUE);
	gtk_box_append (GTK_BOX (boxr_bottom_bar), boxr_bottom_right_spacer);
	
	seed_entry_d = g_new0 (SeedEntryData, 1);
	seed_entry_d->seed = &app_data->seed_value;
	seed_entry_d->win = win;
	g_signal_connect(gtk_editable_get_delegate(GTK_EDITABLE(seed_entry)),
	"insert-text", G_CALLBACK(seed_entry_int_filter), seed_entry_d);

	reload_d = g_new0 (ReloadDropDownData, 1);
	reload_d->app = app;
	reload_d->checkpoint_dd = checkpoint_dd;
	reload_d->vae_dd = vae_dd;
	reload_d->cnet_dd = cnet_dd;
	reload_d->upscaler_dd = upscaler_dd;
	reload_d->clip_l_dd = clip_l_dd;
	reload_d->clip_g_dd = clip_g_dd;
	reload_d->text_enc_dd = text_enc_dd;
	reload_d->lora_dd = lora_dd;
	reload_d->embedding_dd = embedding_dd;
	g_signal_connect (reload_btn, "clicked", G_CALLBACK (reload_dropdown), reload_d);
	g_signal_connect (reload_btn, "destroy", G_CALLBACK (on_reload_btn_destroy), reload_d);

	reset_d = g_new0 (ResetCbData, 1);
	reset_d->pos_tb = pos_tb;
	reset_d->neg_tb = neg_tb;
	reset_d->checkpoint_dd = checkpoint_dd;
	reset_d->vae_dd = vae_dd;
	reset_d->cnet_dd = cnet_dd;
	reset_d->upscaler_dd = upscaler_dd;
	reset_d->clip_l_dd = clip_l_dd;
	reset_d->clip_g_dd = clip_g_dd;
	reset_d->text_enc_dd = text_enc_dd;
	reset_d->cfg_spin = cfg_spin;
	reset_d->denoise_spin = denoise_spin;
	reset_d->seed_entry = seed_entry;
	reset_d->upscale_spin = upscale_spin;
	reset_d->lora_dd = lora_dd;
	reset_d->embedding_dd = embedding_dd;
	reset_d->sampler_dd = sampler_dd;
	reset_d->scheduler_dd = scheduler_dd;
	reset_d->width_dd = width_dd;
	reset_d->height_dd = height_dd;
	reset_d->steps_spin = steps_spin;
	reset_d->batch_count_spin = batch_count_spin;
	reset_d->sd_based_check = sd_based_check;
	reset_d->llm_check = llm_check;
	reset_d->cpu_check = cpu_check;
	reset_d->tiling_check = tiling_check;
	reset_d->ram_offload_check = ram_offload_check;
	reset_d->clip_check = clip_check;
	reset_d->cnet_check = cnet_check;
	reset_d->vae_check = vae_check;
	reset_d->flash_check = flash_check;
	reset_d->taesd_check = taesd_check;
	reset_d->update_cache_check = update_cache_check;
	reset_d->verbose_check = verbose_check;
	g_signal_connect (reset_default_btn, "clicked", G_CALLBACK (reset_default_btn_cb), reset_d);
	g_signal_connect (reset_default_btn, "destroy", G_CALLBACK (on_reset_default_btn_destroy), reset_d);

	preview_d->image_widget = preview_img;
	preview_d->img_index_label = img_index_label;
	g_signal_connect (hide_img_btn, "clicked", G_CALLBACK (hide_img_btn_cb), preview_d);
	g_signal_connect (hide_img_btn, "destroy", G_CALLBACK (on_hide_img_btn_destroy), preview_d);
	g_signal_connect (prev_img_button, "clicked", G_CALLBACK (navigate_img_prev), preview_d);
	g_signal_connect (next_img_button, "clicked", G_CALLBACK (navigate_img_next), preview_d);

	load_png_info_d = g_new0 (LoadPNGData, 1);
	load_png_info_d->image_files = app_data->image_files;
	load_png_info_d->current_image_index = &app_data->current_image_index;
	load_png_info_d->win = win;
	load_png_info_d->pos_tb = pos_tb;
	load_png_info_d->neg_tb = neg_tb;
	load_png_info_d->steps_spin = steps_spin;
	load_png_info_d->cfg_spin = cfg_spin;
	load_png_info_d->seed_entry = seed_entry;
	load_png_info_d->width_dd = width_dd;
	load_png_info_d->height_dd = height_dd;
	load_png_info_d->checkpoint_dd = checkpoint_dd;
	load_png_info_d->sampler_dd = sampler_dd;
	load_png_info_d->scheduler_dd = scheduler_dd;
	g_signal_connect (load_from_img_btn, "clicked", G_CALLBACK (load_from_img_btn_cb), load_png_info_d);
	g_signal_connect (load_from_current_btn, "clicked", G_CALLBACK (load_from_img_preview), load_png_info_d);
	g_signal_connect (load_from_img_btn, "destroy", G_CALLBACK (on_load_from_img_btn_destroy), load_png_info_d);
	
	load_img2img_file_d = g_new0 (LoadImg2ImgData, 1);
	load_img2img_file_d->win = win;
	load_img2img_file_d->image_wgt = preview_img2img;
	load_img2img_file_d->img2img_file_path = app_data->img2img_file_path;
	g_signal_connect (load_img2img_btn, "clicked", G_CALLBACK (load_img2img_btn_cb), load_img2img_file_d);
	g_signal_connect (clear_img2img_btn, "clicked", G_CALLBACK (clear_img2img_btn_cb), load_img2img_file_d);
	g_signal_connect (clear_img2img_btn, "destroy", G_CALLBACK (on_clear_img2img_btn_destroy), load_img2img_file_d);
	
	load_img2img_from_preview_d = g_new0 (LoadImg2ImgFromPreviewData, 1);
	load_img2img_from_preview_d->current_image_index = &app_data->current_image_index;
	load_img2img_from_preview_d->image_files = app_data->image_files;
	load_img2img_from_preview_d->img2img_file_path = app_data->img2img_file_path;
	load_img2img_from_preview_d->image_wgt = preview_img2img;
	g_signal_connect (set_img2img_from_preview_btn, "clicked", G_CALLBACK (set_current_preview_to_img2img), load_img2img_from_preview_d);
	g_signal_connect (set_img2img_from_preview_btn, "destroy", G_CALLBACK (on_set_img2img_from_preview_btn_destroy), load_img2img_from_preview_d);

	gen_d = g_new0 (GenerationData, 1);
	gen_d->checkpoint_string = app_data->checkpoint_string;
	gen_d->vae_string = app_data->vae_string;
	gen_d->cnet_string = app_data->cnet_string;
	gen_d->upscaler_string = app_data->upscaler_string;
	gen_d->clip_l_string = app_data->clip_l_string;
	gen_d->clip_g_string = app_data->clip_g_string;
	gen_d->text_enc_string = app_data->text_enc_string;
	gen_d->img_index_string = app_data->img_index_string;
	gen_d->image_files = app_data->image_files;
	gen_d->sdpid = &app_data->sdpid;
	gen_d->sampler_index = &app_data->sampler_index;
	gen_d->scheduler_index = &app_data->scheduler_index;
	gen_d->w_index = &app_data->w_index;
	gen_d->h_index = &app_data->h_index;
	gen_d->steps_value = &app_data->steps_value;
	gen_d->batch_count_value = &app_data->batch_count_value;
	gen_d->sd_based_bool = &app_data->sd_based_bool;
	gen_d->llm_bool = &app_data->llm_bool;
	gen_d->cpu_bool = &app_data->cpu_bool;
	gen_d->vt_bool = &app_data->vt_bool;
	gen_d->ram_offload_bool = &app_data->ram_offload_bool;
	gen_d->k_clip_bool = &app_data->k_clip_bool;
	gen_d->k_cnet_bool = &app_data->k_cnet_bool;
	gen_d->k_vae_bool = &app_data->k_vae_bool;
	gen_d->fa_bool = &app_data->fa_bool;
	gen_d->taesd_bool = &app_data->taesd_bool;
	gen_d->update_cache_bool = &app_data->update_cache_bool;
	gen_d->verbose_bool = &app_data->verbose_bool;
	gen_d->current_image_index = &app_data->current_image_index;
	gen_d->seed_value = &app_data->seed_value;
	gen_d->cfg_value = &app_data->cfg_value;
	gen_d->denoise_value = &app_data->denoise_value;
	gen_d->clip_skip_value = &app_data->clip_skip_value;
	gen_d->up_repeat_value = &app_data->up_repeat_value;
	gen_d->pos_p = pos_tb;
	gen_d->neg_p = neg_tb;
	gen_d->image_widget = preview_img;
	gen_d->img_index_label = img_index_label;
	gen_d->show_img_btn = hide_img_btn;
	gen_d->halt_btn = sd_halt_btn;
	gen_d->win = win;
	gen_d->img2img_file_path = app_data->img2img_file_path;
	g_signal_connect (generate_btn, "clicked", G_CALLBACK (generate_cb), gen_d);
	g_signal_connect (generate_btn, "destroy", G_CALLBACK (on_generate_btn_destroy), gen_d);

	g_signal_connect (info_btn, "clicked", G_CALLBACK (show_info_message), win);
	g_signal_connect (win, "close-request", G_CALLBACK (close_app_callback), user_data);
	
	gtk_window_present (GTK_WINDOW (win));
}

int
main (int argc, char **argv)
{
	int a = check_create_base_dirs();
	if (a != 0) {
		return 1;
	}

	GtkApplication *app;
	AppStartData *data = g_new0 (AppStartData, 1);
	data->checkpoint_string = NULL;
	data->vae_string = NULL;
	data->cnet_string = NULL;
	data->upscaler_string = NULL;
	data->clip_l_string = NULL;
	data->clip_g_string = NULL;
	data->text_enc_string = NULL;
	data->img2img_file_path = NULL;
	
	data->checkpoint_string = g_string_new("None");
	if (data->checkpoint_string == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->vae_string = g_string_new("None");
	if (data->vae_string == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->cnet_string = g_string_new("None");
	if (data->cnet_string == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->upscaler_string = g_string_new("None");
	if (data->upscaler_string == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->clip_l_string = g_string_new("None");
	if (data->clip_l_string == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->clip_g_string = g_string_new("None");
	if (data->clip_g_string == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->text_enc_string = g_string_new("None");
	if (data->text_enc_string == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->img2img_file_path = g_string_new("None");
	if (data->img2img_file_path == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->img_index_string = g_string_new("0 / 0");
	if (data->img_index_string == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->image_files = g_ptr_array_new_full(1, g_free);
	if (data->image_files == NULL) {
		g_error("Failed to allocate GPtrArray.");
		return 1;
	}
	
	data->current_image_index = 0;
	data->sdpid = 0;
	
	int s;

	app = gtk_application_new ("com.github.LuizAlcantara.NeuralPixel", 0);
	g_signal_connect (app, "activate", G_CALLBACK (app_activate), data);
	s = g_application_run (G_APPLICATION (app), argc, argv);
	if (data != NULL) {
		g_free(data);
	}
	g_object_unref (app);
	return s;
}

