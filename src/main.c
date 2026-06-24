#include <dirent.h>
#include <gtk/gtk.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "constants.h"
#include "dropdown_widgets.h"
#include "file_utils.h"
#include "generate_cb.h"
#include "handle_cache.h"
#include "mask_utils.h"
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
	
	preview_d->img_index_string = app_data->preview_label_string;
	preview_d->image_files = app_data->preview_image_files;
	preview_d->current_image_index = &app_data->preview_image_index;

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
	GtkWidget *box_img2img, *box_img2img_top_buttons, *box_preview_img2img, *box_img2img_bottom_buttons;
	GtkWidget *load_img2img_btn, *kontext_check, *clear_img2img_btn;
	GtkWidget *overlay_img2img, *preview_img2img;
	GtkWidget *mask_img2img_btn, *clear_mask_btn, *inpaint_check;

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
	GtkWidget *box_params_row7;
	
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
	GtkWidget *cnet_strength_lab, *cnet_strength_spin;
	
	GtkWidget *box_hires;
	GtkWidget *box_hires_col1, *box_hires_col2;
	GtkWidget *hires_upscaler_lab, *hires_upscaler_dd;
	GtkWidget *hires_scale_lab, *hires_scale_spin;
	GtkWidget *hires_steps_lab, *hires_steps_spin;
	GtkWidget *hires_denoise_lab, *hires_denoise_spin;
	
	GtkWidget *extra_opts_expander, *box_extra_opts, *box_extra_opts_row1, *box_extra_opts_col1, *box_extra_opts_col2;
	
	GtkWidget *fa_separator;
	GtkWidget *fa_toggle_lab;
	GtkWidget *box_fa_toggle;
	GtkWidget *fa_off_btn, *fa_diffusion_btn, *fa_full_btn;
	GtkWidget *mmap_check, *taesd_check, *update_cache_check, *verbose_check;
	
	GtkWidget *vae_tiling_separator;
	GtkWidget *vae_tiling_lab;
	GtkWidget *vae_tiling_dd;
	
	GtkWidget *model_backend_separator;
	GtkWidget *model_backend_lab;
	GtkWidget *box_model_backend, *box_model_backend_col1, *box_model_backend_col2;
	GtkWidget *model_runtime_backend_lab, *model_parameter_backend_lab;
	GtkWidget *model_runtime_backend_dd, *model_parameter_backend_dd;
	
	GtkWidget *te_backend_separator;
	GtkWidget *te_backend_lab;
	GtkWidget *box_te_backend, *box_te_backend_col1, *box_te_backend_col2;
	GtkWidget *te_runtime_backend_lab, *te_parameter_backend_lab;
	GtkWidget *te_runtime_backend_dd, *te_parameter_backend_dd;
	
	GtkWidget *vae_backend_separator;
	GtkWidget *vae_backend_lab;
	GtkWidget *box_vae_backend, *box_vae_backend_col1, *box_vae_backend_col2;
	GtkWidget *vae_runtime_backend_lab, *vae_parameter_backend_lab;
	GtkWidget *vae_runtime_backend_dd, *vae_parameter_backend_dd;
	
	GtkWidget *cnet_backend_separator;
	GtkWidget *cnet_backend_lab;
	GtkWidget *box_cnet_backend, *box_cnet_backend_col1, *box_cnet_backend_col2;
	GtkWidget *cnet_runtime_backend_lab, *cnet_parameter_backend_lab;
	GtkWidget *cnet_runtime_backend_dd, *cnet_parameter_backend_dd;
	
	GtkWidget *upscaler_backend_separator;
	GtkWidget *upscaler_backend_lab;
	GtkWidget *box_upscaler_backend, *box_upscaler_backend_col1, *box_upscaler_backend_col2;
	GtkWidget *upscaler_runtime_backend_lab, *upscaler_parameter_backend_lab;
	GtkWidget *upscaler_runtime_backend_dd, *upscaler_parameter_backend_dd;

	GtkWidget *box_generation;
	GtkWidget *sd_halt_btn;

	GtkWidget *box_right, *boxr_img, *boxr_bottom_bar, *boxr_bottom_left_box, *boxr_bottom_right_box;
	GtkWidget *preview_img;
	GtkWidget *prev_10_img_button, *prev_img_button, *img_index_label, *next_img_button, *next_10_img_button, *load_from_current_btn, *set_img2img_from_preview_btn, *hide_img_btn, *to_trash_btn;

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
	
	// Scrolled Window Viewport
	GtkWidget *prop_scroll_vp = gtk_scrolled_window_get_child(GTK_SCROLLED_WINDOW(properties_scrollable));
	if (GTK_IS_VIEWPORT(prop_scroll_vp)) {
		gtk_viewport_set_scroll_to_focus(GTK_VIEWPORT(prop_scroll_vp), FALSE);
	}
	
	//Set IMG2IMG Widgets
	img2img_expander = gtk_expander_new ("Input Image");
	gtk_widget_add_css_class(img2img_expander, "param_label");
	gtk_widget_set_tooltip_text(GTK_WIDGET(img2img_expander),
	"Expand the area to configure img2img, Flux Kontext, or ControlNet processing.\nThe arrow on the side will be colored whenever an image is loaded.");
	gtk_box_append (GTK_BOX (box_properties), img2img_expander);
	
	box_img2img = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_widget_add_css_class(box_img2img, "inner_box");
	gtk_box_set_homogeneous (GTK_BOX (box_img2img), FALSE);
	gtk_expander_set_child(GTK_EXPANDER(img2img_expander), box_img2img);
	
	box_img2img_top_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_img2img_top_buttons), FALSE);
	gtk_box_append (GTK_BOX (box_img2img), box_img2img_top_buttons);
	
	load_img2img_btn = gtk_button_new_with_label ("Choose Image");
	gtk_widget_set_hexpand (load_img2img_btn, TRUE);
	gtk_widget_add_css_class(load_img2img_btn, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(load_img2img_btn),
	"Select the source image that will be used for img2img, Flux Kontext, or ControlNet processing.");
	gtk_box_append (GTK_BOX (box_img2img_top_buttons), load_img2img_btn);
	
	clear_img2img_btn = gtk_button_new_from_icon_name ("edit-delete-symbolic");
	gtk_widget_add_css_class(clear_img2img_btn, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(clear_img2img_btn),
	"Clear the source image to stop using it as a template.");
	gtk_box_append (GTK_BOX (box_img2img_top_buttons), clear_img2img_btn);
	
	kontext_check = gtk_check_button_new_with_label("Kontext");
	gtk_widget_add_css_class(kontext_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(kontext_check), app_data->kontext_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(kontext_check), "Enables Flux-Kontext for superior structural adherence to your reference image.\nEnsure you are using a compatible Kontext checkpoint.");
	g_signal_connect(kontext_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->kontext_bool);
	gtk_box_append (GTK_BOX (box_img2img_top_buttons), kontext_check);
	
	box_preview_img2img = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_widget_add_css_class(box_preview_img2img, "img_preview_box");
	gtk_widget_set_size_request(box_preview_img2img, 412, 400);
	gtk_widget_set_hexpand (box_preview_img2img, FALSE);
	gtk_widget_set_vexpand (box_preview_img2img, FALSE);
	gtk_box_append (GTK_BOX (box_img2img), box_preview_img2img);
	
	overlay_img2img = gtk_overlay_new();
	gtk_widget_set_size_request(overlay_img2img, 412, 400);
	
	preview_img2img = gtk_picture_new_for_filename(EMPTY_IMG_PATH);
	gtk_widget_set_valign(preview_img2img, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(preview_img2img, GTK_ALIGN_CENTER);
	
	gtk_overlay_set_child(GTK_OVERLAY(overlay_img2img), preview_img2img);
	gtk_box_append (GTK_BOX (box_preview_img2img), overlay_img2img);
	
	box_img2img_bottom_buttons = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_img2img_bottom_buttons), FALSE);
	gtk_box_append (GTK_BOX (box_img2img), box_img2img_bottom_buttons);
	
	mask_img2img_btn = gtk_button_new_with_label ("Draw Mask");
	gtk_widget_set_hexpand (mask_img2img_btn, TRUE);
	gtk_widget_add_css_class(mask_img2img_btn, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(mask_img2img_btn),
	"Create an inpaint mask to modify specific areas.");
	gtk_box_append (GTK_BOX (box_img2img_bottom_buttons), mask_img2img_btn);
	
	clear_mask_btn = gtk_button_new_from_icon_name ("edit-delete-symbolic");
	gtk_widget_add_css_class(clear_mask_btn, "custom_btn");
	gtk_widget_set_tooltip_text(GTK_WIDGET(clear_mask_btn),
	"Delete mask image.");
	gtk_box_append (GTK_BOX (box_img2img_bottom_buttons), clear_mask_btn);
	
	inpaint_check = gtk_check_button_new_with_label("Enable Inpaint");
	gtk_widget_add_css_class(inpaint_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(inpaint_check), FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(inpaint_check), "Enables Inpainting for precise modification of masked regions within your image.\nFor optimal results try using a inpainting-specific model.");
	g_signal_connect(inpaint_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->inpaint_bool);
	gtk_box_append (GTK_BOX (box_img2img_bottom_buttons), inpaint_check);

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
	gtk_widget_set_focusable(generate_btn, FALSE);
	
	checkpoint_dd = gen_path_dd(CHECKPOINTS_PATH, NULL, 0, app_data->checkpoint_string, generate_btn, app, 1);
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

	vae_dd = gen_path_dd(VAES_PATH, NULL, 0, app_data->vae_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_addons), vae_dd);

	//Set CNet Widgets
	
	cnet_lab = gtk_label_new ("Control Net");
	gtk_widget_set_halign(cnet_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(cnet_lab, "param_label");
	gtk_box_append (GTK_BOX (box_addons), cnet_lab);
	
	cnet_dd = gen_path_dd(CONTROLNET_PATH, NULL, 0, app_data->cnet_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_addons), cnet_dd);
	
	//Set Upscaler Widgets

	upscaler_lab = gtk_label_new ("Upscaler");
	gtk_widget_set_halign(upscaler_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(upscaler_lab, "param_label");
	gtk_box_append (GTK_BOX (box_addons), upscaler_lab);

	upscaler_dd = gen_path_dd(UPSCALES_PATH, NULL, 0, app_data->upscaler_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_addons), upscaler_dd);

	//Set Clip_l Widgets
	
	clip_l_lab = gtk_label_new ("Clip_l");
	gtk_widget_set_halign(clip_l_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(clip_l_lab, "param_label");
	gtk_box_append (GTK_BOX (box_addons), clip_l_lab);
	
	clip_l_dd = gen_path_dd(CLIPS_PATH, NULL, 0, app_data->clip_l_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_addons), clip_l_dd);
	
	//Set Clip_g Widgets
	
	clip_g_lab = gtk_label_new ("Clip_g");
	gtk_widget_set_halign(clip_g_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(clip_g_lab, "param_label");
	gtk_box_append (GTK_BOX (box_addons), clip_g_lab);
	
	clip_g_dd = gen_path_dd(CLIPS_PATH, NULL, 0, app_data->clip_g_string, NULL, app, 0);
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
	
	text_enc_dd = gen_path_dd(TEXT_ENCODERS_PATH, NULL, 0, app_data->text_enc_string, NULL, app, 0);
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
	
	lora_dd = gen_path_dd(LORAS_PATH, pos_tb, 1, NULL, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_model_adapters), lora_dd);

	//Set Embeddings Widgets

	embedding_lab = gtk_label_new ("Add Embedding");
	gtk_widget_set_halign(embedding_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(embedding_lab, "param_label");
	gtk_box_append (GTK_BOX (box_model_adapters), embedding_lab);

	embedding_dd = gen_path_dd(EMBEDDINGS_PATH, neg_tb, 0, NULL, NULL, app, 0);
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
	stop_spinbutton_scroll(steps_spin);
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
	stop_spinbutton_scroll(batch_count_spin);
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
	stop_spinbutton_scroll(cfg_spin);
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
	"Controls how much the original image is changed.\nValues near 1.0 heavily modify or replace the image.\nValues near 0 preserve the original image with minimal changes.");
	stop_spinbutton_scroll(denoise_spin);
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
	stop_spinbutton_scroll(clip_skip_spin);
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
	stop_spinbutton_scroll(upscale_spin);
	gtk_box_append (GTK_BOX (box_params_row6_col2), upscale_spin);
	
	//Set Parameters Seventh Row Widgets
	
	box_params_row7 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_params), box_params_row7);
	
	//Set Control Net Strength Widgets

	cnet_strength_lab = gtk_label_new ("ControlNet Str");
	gtk_widget_set_halign(cnet_strength_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(cnet_strength_lab, "param_label");
	gtk_box_append (GTK_BOX (box_params_row7), cnet_strength_lab);
	
	cnet_strength_spin = gtk_spin_button_new_with_range (0, 1.0, 0.05);
	gtk_widget_add_css_class(cnet_strength_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(cnet_strength_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(cnet_strength_spin), app_data->cnet_value);
	g_signal_connect (cnet_strength_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->cnet_value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(cnet_strength_spin),
	"Adjusts how strongly the ControlNet influences the image.\nValues near '1' force the output to match the control map strictly.\nValues near '0' allow the prompt more creative freedom.");
	stop_spinbutton_scroll(cnet_strength_spin);
	gtk_box_append (GTK_BOX (box_params_row7), cnet_strength_spin);
	
	//Set Hires Widgets
	
	box_hires = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_hires), TRUE);
	gtk_widget_add_css_class(box_hires, "inner_box");
	gtk_box_append (GTK_BOX (box_properties), box_hires);
	
	box_hires_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_hires_col1), TRUE);
	gtk_box_append (GTK_BOX (box_hires), box_hires_col1);
	
	box_hires_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_hires_col2), TRUE);
	gtk_box_append (GTK_BOX (box_hires), box_hires_col2);

	//Set Hires DD Widgets

	hires_upscaler_lab = gtk_label_new ("Hires Upscaler");
	gtk_widget_add_css_class(hires_upscaler_lab, "param_label");
	gtk_widget_set_halign(hires_upscaler_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_hires_col1), hires_upscaler_lab);
	
	hires_upscaler_dd = gen_const_dd(LIST_HIRES_UPSCALERS, &app_data->hires_upscaler_index);
	gtk_box_append (GTK_BOX (box_hires_col1), hires_upscaler_dd);
	
	//Set Hires Scale Widgets

	hires_scale_lab = gtk_label_new ("Hires Scale");
	gtk_widget_set_halign(hires_scale_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(hires_scale_lab, "param_label");
	gtk_box_append (GTK_BOX (box_hires_col1), hires_scale_lab);
	
	hires_scale_spin = gtk_spin_button_new_with_range (1.0, 4.0, 0.05);
	gtk_widget_add_css_class(hires_scale_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(hires_scale_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(hires_scale_spin), app_data->hires_scale_value);
	g_signal_connect (hires_scale_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->hires_scale_value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(hires_scale_spin),
	"Controls how much the image is enlarged during the Hires fix step.\nA value of 1.2 increases the image size by 20%, 2.0 doubles it,\nand 1.0 keeps the original size.");
	stop_spinbutton_scroll(hires_scale_spin);
	gtk_box_append (GTK_BOX (box_hires_col1), hires_scale_spin);
	
	//Set Hires Steps Widgets

	hires_steps_lab = gtk_label_new ("Hires Steps");
	gtk_widget_set_halign(hires_steps_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(hires_steps_lab, "param_label");
	gtk_box_append (GTK_BOX (box_hires_col2), hires_steps_lab);
	
	hires_steps_spin = gtk_spin_button_new_with_range (1.0, 50.0, 1.0);
	gtk_widget_add_css_class(hires_steps_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(hires_steps_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(hires_steps_spin), app_data->hires_steps_value);
	g_signal_connect (hires_steps_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->hires_steps_value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(hires_steps_spin),
	"Number of refinement steps used during Hires processing.\nMore steps = better detail, longer generation time.");
	stop_spinbutton_scroll(hires_steps_spin);
	gtk_box_append (GTK_BOX (box_hires_col2), hires_steps_spin);
	
	//Set Hires Denoise Str Widgets

	hires_denoise_lab = gtk_label_new ("Hires Denoise Str");
	gtk_widget_set_halign(hires_denoise_lab, LABEL_ALIGNMENT);
	gtk_widget_add_css_class(hires_denoise_lab, "param_label");
	gtk_box_append (GTK_BOX (box_hires_col2), hires_denoise_lab);
	
	hires_denoise_spin = gtk_spin_button_new_with_range (0, 1.0, 0.05);
	gtk_widget_add_css_class(hires_denoise_spin, "custom_spin");
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(hires_denoise_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(hires_denoise_spin), app_data->hires_denoise_value);
	g_signal_connect (hires_denoise_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->hires_denoise_value);
	gtk_widget_set_tooltip_text(GTK_WIDGET(hires_denoise_spin),
	"Controls how much the image is altered during Hires processing.\nHigher values allow greater changes and new details.");
	stop_spinbutton_scroll(hires_denoise_spin);
	gtk_box_append (GTK_BOX (box_hires_col2), hires_denoise_spin);
	
	
	//Set Extra Options Widgets
	
	extra_opts_expander = gtk_expander_new ("Extra Options");
	gtk_widget_add_css_class(extra_opts_expander, "param_label");
	gtk_box_append (GTK_BOX (box_properties), extra_opts_expander);
	
	box_extra_opts = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_widget_add_css_class(box_extra_opts, "inner_box");
	gtk_box_set_homogeneous (GTK_BOX (box_extra_opts), FALSE);
	gtk_expander_set_child(GTK_EXPANDER(extra_opts_expander), box_extra_opts);
	
	box_extra_opts_row1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_extra_opts_row1), TRUE);
	gtk_box_append (GTK_BOX (box_extra_opts), box_extra_opts_row1);
	
	box_extra_opts_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_extra_opts_col1), TRUE);
	gtk_box_append (GTK_BOX (box_extra_opts_row1), box_extra_opts_col1);
	
	box_extra_opts_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_extra_opts_col2), TRUE);
	gtk_box_append (GTK_BOX (box_extra_opts_row1), box_extra_opts_col2);
	
	//Set Extra Options Widgets
	
	fa_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_add_css_class(fa_separator, "horiz_separator");
	gtk_box_append (GTK_BOX (box_extra_opts), fa_separator);
	
	fa_toggle_lab = gtk_label_new ("Flash Attention ⓘ");
	gtk_widget_add_css_class(fa_toggle_lab, "param_label");
	gtk_widget_set_tooltip_text(GTK_WIDGET(fa_toggle_lab), "Enables a faster, more memory-efficient attention method that\nreduces VRAM usage and can speed up image generation.");
	gtk_widget_set_halign(fa_toggle_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_extra_opts), fa_toggle_lab);
	
	box_fa_toggle = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_fa_toggle), TRUE);
	gtk_box_append (GTK_BOX (box_extra_opts), box_fa_toggle);
	
	fa_off_btn = gtk_toggle_button_new_with_label("Disabled");
	gtk_widget_add_css_class(fa_off_btn, "toggle_btn");
	gtk_widget_set_hexpand(fa_off_btn, TRUE);
	gtk_widget_set_vexpand(fa_off_btn, FALSE);
	
	fa_diffusion_btn = gtk_toggle_button_new_with_label("Diffusion");
	gtk_widget_add_css_class(fa_diffusion_btn, "toggle_btn");
	gtk_widget_set_hexpand(fa_diffusion_btn, TRUE);
	gtk_widget_set_vexpand(fa_diffusion_btn, FALSE);
	
	fa_full_btn = gtk_toggle_button_new_with_label("Full");
	gtk_widget_add_css_class(fa_full_btn, "toggle_btn");
	gtk_widget_set_hexpand(fa_full_btn, TRUE);
	gtk_widget_set_vexpand(fa_full_btn, FALSE);
	
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(fa_diffusion_btn), GTK_TOGGLE_BUTTON(fa_off_btn));
	gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(fa_full_btn), GTK_TOGGLE_BUTTON(fa_off_btn));
	
	if (app_data->flash_attn_value == 2) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fa_full_btn), TRUE);
	} else if (app_data->flash_attn_value == 1) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fa_diffusion_btn), TRUE);
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fa_off_btn), TRUE);
	}
	
	g_signal_connect(fa_off_btn, "toggled", G_CALLBACK(toggle_fa_options), &app_data->flash_attn_value);
	g_signal_connect(fa_diffusion_btn, "toggled", G_CALLBACK(toggle_fa_options), &app_data->flash_attn_value);
	g_signal_connect(fa_full_btn, "toggled", G_CALLBACK(toggle_fa_options), &app_data->flash_attn_value);
	
	gtk_box_append(GTK_BOX(box_fa_toggle), fa_off_btn);
	gtk_box_append(GTK_BOX(box_fa_toggle), fa_diffusion_btn);
	gtk_box_append(GTK_BOX(box_fa_toggle), fa_full_btn);
	
	mmap_check = gtk_check_button_new_with_label("Enable MMap");
	gtk_widget_add_css_class(mmap_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(mmap_check), app_data->mmap_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(mmap_check), "Speeds up model loading and reduces system RAM usage by\nreading files directly from your disk");
	g_signal_connect(mmap_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->mmap_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col1), mmap_check);
	
	taesd_check = gtk_check_button_new_with_label("Enable TAESD");
	gtk_widget_add_css_class(taesd_check, "custom_check");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(taesd_check), app_data->taesd_bool == 1 ? TRUE : FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(taesd_check), "Use Tiny AutoEncoder for fast decoding\n(low quality).");
	g_signal_connect(taesd_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->taesd_bool);
	gtk_box_append (GTK_BOX (box_extra_opts_col1), taesd_check);
	
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
	
	// VAE Tiling Widgets
	
	vae_tiling_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_add_css_class(vae_tiling_separator, "horiz_separator");
	gtk_widget_set_margin_top(vae_tiling_separator, MEDIUM_SPACING);
	gtk_box_append (GTK_BOX (box_extra_opts), vae_tiling_separator);
	
	vae_tiling_lab = gtk_label_new ("VAE Tiling ⓘ");
	gtk_widget_add_css_class(vae_tiling_lab, "param_label");
	gtk_widget_set_halign(vae_tiling_lab, LABEL_ALIGNMENT);
	gtk_widget_set_tooltip_text(GTK_WIDGET(vae_tiling_lab), "Process VAE in tiles to reduce memory usage.");
	gtk_box_append (GTK_BOX (box_extra_opts), vae_tiling_lab);
	
	vae_tiling_dd = gen_const_dd(LIST_VAE_TILE_SIZES, &app_data->vae_tiling_index);
	gtk_box_append (GTK_BOX (box_extra_opts), vae_tiling_dd);
	
	// Diffusion Backend Widgets
	
	model_backend_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_add_css_class(model_backend_separator, "horiz_separator");
	gtk_box_append (GTK_BOX (box_extra_opts), model_backend_separator);
	
	model_backend_lab = gtk_label_new ("Model Backend");
	gtk_widget_add_css_class(model_backend_lab, "param_label");
	gtk_widget_set_halign(model_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_extra_opts), model_backend_lab);
	
	box_model_backend = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_model_backend), TRUE);
	gtk_widget_set_hexpand (box_model_backend, TRUE);
	gtk_widget_set_margin_bottom(box_model_backend, MEDIUM_SPACING);
	gtk_box_append (GTK_BOX (box_extra_opts), box_model_backend);
	
	box_model_backend_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_model_backend), box_model_backend_col1);
	
	box_model_backend_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_model_backend), box_model_backend_col2);
	
	model_runtime_backend_lab = gtk_label_new ("Execution Device");
	gtk_widget_add_css_class(model_runtime_backend_lab, "param_label");
	gtk_widget_set_halign(model_runtime_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_model_backend_col1), model_runtime_backend_lab);
	
	model_runtime_backend_dd = gen_const_dd(LIST_BACKENDS, &app_data->model_runtime_backend_index);
	gtk_box_append (GTK_BOX (box_model_backend_col1), model_runtime_backend_dd);

	model_parameter_backend_lab = gtk_label_new ("Offload Device");
	gtk_widget_add_css_class(model_parameter_backend_lab, "param_label");
	gtk_widget_set_halign(model_parameter_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_model_backend_col2), model_parameter_backend_lab);
	
	model_parameter_backend_dd = gen_const_dd(LIST_BACKENDS, &app_data->model_param_backend_index);
	gtk_box_append (GTK_BOX (box_model_backend_col2), model_parameter_backend_dd);
	
	// Text-Encoder Backend Widgets
	
	te_backend_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_add_css_class(te_backend_separator, "horiz_separator");
	gtk_box_append (GTK_BOX (box_extra_opts), te_backend_separator);
	
	te_backend_lab = gtk_label_new ("Text Encoder Backend");
	gtk_widget_add_css_class(te_backend_lab, "param_label");
	gtk_widget_set_halign(te_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_extra_opts), te_backend_lab);
	
	box_te_backend = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_te_backend), TRUE);
	gtk_widget_set_hexpand (box_te_backend, TRUE);
	gtk_widget_set_margin_bottom(box_te_backend, MEDIUM_SPACING);
	gtk_box_append (GTK_BOX (box_extra_opts), box_te_backend);
	
	box_te_backend_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_te_backend), box_te_backend_col1);
	
	box_te_backend_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_te_backend), box_te_backend_col2);
	
	te_runtime_backend_lab = gtk_label_new ("Execution Device");
	gtk_widget_add_css_class(te_runtime_backend_lab, "param_label");
	gtk_widget_set_halign(te_runtime_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_te_backend_col1), te_runtime_backend_lab);
	
	te_runtime_backend_dd = gen_const_dd(LIST_BACKENDS, &app_data->te_runtime_backend_index);
	gtk_box_append (GTK_BOX (box_te_backend_col1), te_runtime_backend_dd);

	te_parameter_backend_lab = gtk_label_new ("Offload Device");
	gtk_widget_add_css_class(te_parameter_backend_lab, "param_label");
	gtk_widget_set_halign(te_parameter_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_te_backend_col2), te_parameter_backend_lab);
	
	te_parameter_backend_dd = gen_const_dd(LIST_BACKENDS, &app_data->te_param_backend_index);
	gtk_box_append (GTK_BOX (box_te_backend_col2), te_parameter_backend_dd);
	
	// VAE Backend Widgets
	
	vae_backend_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_add_css_class(vae_backend_separator, "horiz_separator");
	gtk_box_append (GTK_BOX (box_extra_opts), vae_backend_separator);
	
	vae_backend_lab = gtk_label_new ("VAE Backend");
	gtk_widget_add_css_class(vae_backend_lab, "param_label");
	gtk_widget_set_halign(vae_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_extra_opts), vae_backend_lab);
	
	box_vae_backend = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_vae_backend), TRUE);
	gtk_widget_set_hexpand (box_vae_backend, TRUE);
	gtk_widget_set_margin_bottom(box_vae_backend, MEDIUM_SPACING);
	gtk_box_append (GTK_BOX (box_extra_opts), box_vae_backend);
	
	box_vae_backend_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_vae_backend), box_vae_backend_col1);
	
	box_vae_backend_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_vae_backend), box_vae_backend_col2);
	
	vae_runtime_backend_lab = gtk_label_new ("Execution Device");
	gtk_widget_add_css_class(vae_runtime_backend_lab, "param_label");
	gtk_widget_set_halign(vae_runtime_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_vae_backend_col1), vae_runtime_backend_lab);
	
	vae_runtime_backend_dd = gen_const_dd(LIST_BACKENDS, &app_data->vae_runtime_backend_index);
	gtk_box_append (GTK_BOX (box_vae_backend_col1), vae_runtime_backend_dd);

	vae_parameter_backend_lab = gtk_label_new ("Offload Device");
	gtk_widget_add_css_class(vae_parameter_backend_lab, "param_label");
	gtk_widget_set_halign(vae_parameter_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_vae_backend_col2), vae_parameter_backend_lab);
	
	vae_parameter_backend_dd = gen_const_dd(LIST_BACKENDS, &app_data->vae_param_backend_index);
	gtk_box_append (GTK_BOX (box_vae_backend_col2), vae_parameter_backend_dd);
	
	// CNet Backend Widgets
	
	cnet_backend_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_add_css_class(cnet_backend_separator, "horiz_separator");
	gtk_box_append (GTK_BOX (box_extra_opts), cnet_backend_separator);
	
	cnet_backend_lab = gtk_label_new ("CNet Backend");
	gtk_widget_add_css_class(cnet_backend_lab, "param_label");
	gtk_widget_set_halign(cnet_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_extra_opts), cnet_backend_lab);
	
	box_cnet_backend = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_cnet_backend), TRUE);
	gtk_widget_set_hexpand (box_cnet_backend, TRUE);
	gtk_widget_set_margin_bottom(box_cnet_backend, MEDIUM_SPACING);
	gtk_box_append (GTK_BOX (box_extra_opts), box_cnet_backend);
	
	box_cnet_backend_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_cnet_backend), box_cnet_backend_col1);
	
	box_cnet_backend_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_cnet_backend), box_cnet_backend_col2);
	
	cnet_runtime_backend_lab = gtk_label_new ("Execution Device");
	gtk_widget_add_css_class(cnet_runtime_backend_lab, "param_label");
	gtk_widget_set_halign(cnet_runtime_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_cnet_backend_col1), cnet_runtime_backend_lab);
	
	cnet_runtime_backend_dd = gen_const_dd(LIST_BACKENDS, &app_data->cnet_runtime_backend_index);
	gtk_box_append (GTK_BOX (box_cnet_backend_col1), cnet_runtime_backend_dd);

	cnet_parameter_backend_lab = gtk_label_new ("Offload Device");
	gtk_widget_add_css_class(cnet_parameter_backend_lab, "param_label");
	gtk_widget_set_halign(cnet_parameter_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_cnet_backend_col2), cnet_parameter_backend_lab);
	
	cnet_parameter_backend_dd = gen_const_dd(LIST_BACKENDS, &app_data->cnet_param_backend_index);
	gtk_box_append (GTK_BOX (box_cnet_backend_col2), cnet_parameter_backend_dd);
	
	// Upscaler Backend Widgets
	
	upscaler_backend_separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_add_css_class(upscaler_backend_separator, "horiz_separator");
	gtk_box_append (GTK_BOX (box_extra_opts), upscaler_backend_separator);
	
	upscaler_backend_lab = gtk_label_new ("Upscaler Backend");
	gtk_widget_add_css_class(upscaler_backend_lab, "param_label");
	gtk_widget_set_halign(upscaler_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_extra_opts), upscaler_backend_lab);
	
	box_upscaler_backend = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_upscaler_backend), TRUE);
	gtk_widget_set_hexpand (box_upscaler_backend, TRUE);
	gtk_widget_set_margin_bottom(box_upscaler_backend, MEDIUM_SPACING);
	gtk_box_append (GTK_BOX (box_extra_opts), box_upscaler_backend);
	
	box_upscaler_backend_col1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_upscaler_backend), box_upscaler_backend_col1);
	
	box_upscaler_backend_col2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, SMALL_SPACING);
	gtk_box_append (GTK_BOX (box_upscaler_backend), box_upscaler_backend_col2);
	
	upscaler_runtime_backend_lab = gtk_label_new ("Execution Device");
	gtk_widget_add_css_class(upscaler_runtime_backend_lab, "param_label");
	gtk_widget_set_halign(upscaler_runtime_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_upscaler_backend_col1), upscaler_runtime_backend_lab);
	
	upscaler_runtime_backend_dd = gen_const_dd(LIST_BACKENDS, &app_data->upscaler_runtime_backend_index);
	gtk_box_append (GTK_BOX (box_upscaler_backend_col1), upscaler_runtime_backend_dd);

	upscaler_parameter_backend_lab = gtk_label_new ("Offload Device");
	gtk_widget_add_css_class(upscaler_parameter_backend_lab, "param_label");
	gtk_widget_set_halign(upscaler_parameter_backend_lab, LABEL_ALIGNMENT);
	gtk_box_append (GTK_BOX (box_upscaler_backend_col2), upscaler_parameter_backend_lab);
	
	upscaler_parameter_backend_dd = gen_const_dd(LIST_BACKENDS, &app_data->upscaler_param_backend_index);
	gtk_box_append (GTK_BOX (box_upscaler_backend_col2), upscaler_parameter_backend_dd);
	
	//Set Generation Box
	box_generation = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_box_set_homogeneous (GTK_BOX (box_generation), FALSE);
	gtk_box_append (GTK_BOX (box_left), box_generation);

	sd_halt_btn = gtk_button_new_from_icon_name ("process-stop-symbolic");
	gtk_widget_add_css_class(sd_halt_btn, "custom_btn");
	gtk_widget_set_focusable(sd_halt_btn, FALSE);
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

	if (app_data->preview_image_files->len > 0) {
		preview_img = gtk_picture_new_for_filename
		(g_ptr_array_index(app_data->preview_image_files, app_data->preview_image_index));
	} else {
		g_printerr("No images in 'outputs' directory.\n");
		preview_img = gtk_picture_new_for_filename
		(DEFAULT_IMG_PATH);
	}
	gtk_picture_set_content_fit(GTK_PICTURE(preview_img), GTK_CONTENT_FIT_CONTAIN);
	gtk_box_append (GTK_BOX (boxr_img), preview_img);
	
	//Set Box Right Button bar bottom
	boxr_bottom_bar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, ZERO_SPACING);
	gtk_widget_add_css_class(boxr_bottom_bar, "left_box");
	gtk_box_set_homogeneous (GTK_BOX (boxr_bottom_bar), TRUE);
	gtk_box_append (GTK_BOX (box_right), boxr_bottom_bar);
	
	//Set Box Right Button Bar Left Box
	boxr_bottom_left_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_widget_add_css_class(boxr_bottom_left_box, "left_box");
	gtk_box_set_homogeneous (GTK_BOX (boxr_bottom_left_box), TRUE);
	gtk_box_append (GTK_BOX (boxr_bottom_bar), boxr_bottom_left_box);
	
	//Previous Img by 10 Button
	prev_10_img_button = gtk_button_new_from_icon_name ("go-last-symbolic-rtl");
	gtk_widget_add_css_class(prev_10_img_button, "custom_btn");
	gtk_widget_set_focusable(prev_10_img_button, FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(prev_10_img_button), "Jump back 10 images");
	gtk_widget_set_size_request(GTK_WIDGET(prev_10_img_button), 60, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_left_box), prev_10_img_button);
	
	//Previous Img Button
	prev_img_button = gtk_button_new_from_icon_name ("go-previous-symbolic");
	gtk_widget_add_css_class(prev_img_button, "custom_btn");
	gtk_widget_set_focusable(prev_img_button, FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(prev_img_button), "Previous Image.");
	gtk_widget_set_size_request(GTK_WIDGET(prev_img_button), 60, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_left_box), prev_img_button);
	
	//Next Img Button
	next_img_button = gtk_button_new_from_icon_name ("go-next-symbolic");
	gtk_widget_add_css_class(next_img_button, "custom_btn");
	gtk_widget_set_focusable(next_img_button, FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(next_img_button), "Next Image.");
	gtk_widget_set_size_request(GTK_WIDGET(next_img_button), 60, -1);
	gtk_box_append(GTK_BOX(boxr_bottom_left_box), next_img_button);
	
	//Next Img by 10 Button
	next_10_img_button = gtk_button_new_from_icon_name ("go-last-symbolic");
	gtk_widget_add_css_class(next_10_img_button, "custom_btn");
	gtk_widget_set_focusable(next_10_img_button, FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(next_10_img_button), "Jump forward 10 images.");
	gtk_widget_set_size_request(GTK_WIDGET(next_10_img_button), 60, -1);
	gtk_box_append(GTK_BOX(boxr_bottom_left_box), next_10_img_button);
	
	//Img index label
	img_index_label = gtk_label_new (app_data->preview_label_string->str);
	gtk_widget_set_size_request(GTK_WIDGET(img_index_label), 120, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_bar), img_index_label);
	
	//Set Box Right Button Bar Right Box
	boxr_bottom_right_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, SMALL_SPACING);
	gtk_widget_add_css_class(boxr_bottom_right_box, "left_box");
	gtk_box_set_homogeneous (GTK_BOX (boxr_bottom_right_box), TRUE);
	gtk_box_append (GTK_BOX (boxr_bottom_bar), boxr_bottom_right_box);
	
	//Load info from current preview image
	load_from_current_btn = gtk_button_new_from_icon_name ("insert-image-symbolic");
	gtk_widget_add_css_class(load_from_current_btn, "custom_btn");
	gtk_widget_set_focusable(load_from_current_btn, FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(load_from_current_btn), "Load prompt and generation parameters from the displayed image.");
	gtk_widget_set_size_request(GTK_WIDGET(load_from_current_btn), 60, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_right_box), load_from_current_btn);
	
	//Set img2img file from current preview image
	set_img2img_from_preview_btn = gtk_button_new_from_icon_name ("x-office-drawing-symbolic");
	gtk_widget_add_css_class(set_img2img_from_preview_btn, "custom_btn");
	gtk_widget_set_focusable(set_img2img_from_preview_btn, FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(set_img2img_from_preview_btn),
	"Sets the current preview image as the template for img2img/ControlNet processing.");
	gtk_widget_set_size_request(GTK_WIDGET(set_img2img_from_preview_btn), 60, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_right_box), set_img2img_from_preview_btn);
	
	//Hide Img Button
	hide_img_btn = gtk_button_new_from_icon_name ("view-reveal-symbolic");
	gtk_widget_add_css_class(hide_img_btn, "custom_btn");
	gtk_widget_set_focusable(hide_img_btn, FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(hide_img_btn), "Toggle displayed image visibility.");
	gtk_widget_set_size_request(GTK_WIDGET(hide_img_btn), 60, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_right_box), hide_img_btn);

	//Send Img to system trash Button
	to_trash_btn = gtk_button_new_from_icon_name ("edit-delete-symbolic");
	gtk_widget_add_css_class(to_trash_btn, "custom_btn");
	gtk_widget_set_focusable(to_trash_btn, FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(to_trash_btn), "Move to Trash (can be restored).");
	gtk_widget_set_size_request(GTK_WIDGET(to_trash_btn), 60, -1);
	gtk_box_append (GTK_BOX (boxr_bottom_right_box), to_trash_btn);
	
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
	reset_d->cnet_strength_spin = cnet_strength_spin;
	reset_d->denoise_spin = denoise_spin;
	reset_d->seed_entry = seed_entry;
	reset_d->upscale_spin = upscale_spin;
	reset_d->lora_dd = lora_dd;
	reset_d->embedding_dd = embedding_dd;
	reset_d->sampler_dd = sampler_dd;
	reset_d->scheduler_dd = scheduler_dd;
	reset_d->width_dd = width_dd;
	reset_d->height_dd = height_dd;
	reset_d->hires_upscaler_dd = hires_upscaler_dd;
	reset_d->hires_scale_spin = hires_scale_spin;
	reset_d->hires_steps_spin = hires_steps_spin;
	reset_d->hires_denoise_spin = hires_denoise_spin;
	reset_d->steps_spin = steps_spin;
	reset_d->batch_count_spin = batch_count_spin;
	reset_d->kontext_check = kontext_check;
	reset_d->sd_based_check = sd_based_check;
	reset_d->llm_check = llm_check;
	reset_d->mmap_check = mmap_check;
	reset_d->fa_off_btn = fa_off_btn;
	reset_d->taesd_check = taesd_check;
	reset_d->update_cache_check = update_cache_check;
	reset_d->verbose_check = verbose_check;
	reset_d->vae_tiling_dd = vae_tiling_dd;
	reset_d->model_runtime_backend_dd = model_runtime_backend_dd;
	reset_d->model_parameter_backend_dd = model_parameter_backend_dd;
	reset_d->te_runtime_backend_dd = te_runtime_backend_dd;
	reset_d->te_parameter_backend_dd = te_parameter_backend_dd;
	reset_d->vae_runtime_backend_dd = vae_runtime_backend_dd;
	reset_d->vae_parameter_backend_dd = vae_parameter_backend_dd;
	reset_d->cnet_runtime_backend_dd = cnet_runtime_backend_dd;
	reset_d->cnet_parameter_backend_dd = cnet_parameter_backend_dd;
	reset_d->upscaler_runtime_backend_dd = upscaler_runtime_backend_dd;
	reset_d->upscaler_parameter_backend_dd = upscaler_parameter_backend_dd;
	g_signal_connect (reset_default_btn, "clicked", G_CALLBACK (reset_default_btn_cb), reset_d);
	g_signal_connect (reset_default_btn, "destroy", G_CALLBACK (on_reset_default_btn_destroy), reset_d);

	preview_d->hide_img_btn = hide_img_btn;
	preview_d->image_widget = preview_img;
	preview_d->img_index_label = img_index_label;
	g_signal_connect (prev_10_img_button, "clicked", G_CALLBACK (navigate_10_img_prev), preview_d);
	g_signal_connect (prev_img_button, "clicked", G_CALLBACK (navigate_img_prev), preview_d);
	g_signal_connect (next_img_button, "clicked", G_CALLBACK (navigate_img_next), preview_d);
	g_signal_connect (next_10_img_button, "clicked", G_CALLBACK (navigate_10_img_next), preview_d);
	g_signal_connect (hide_img_btn, "clicked", G_CALLBACK (hide_img_btn_cb), preview_d);
	g_signal_connect (hide_img_btn, "destroy", G_CALLBACK (on_hide_img_btn_destroy), preview_d);
	g_signal_connect (to_trash_btn, "clicked", G_CALLBACK (send_to_trash), preview_d);

	load_png_info_d = g_new0 (LoadPNGData, 1);
	load_png_info_d->image_files = app_data->preview_image_files;
	load_png_info_d->current_image_index = &app_data->preview_image_index;
	load_png_info_d->win = win;
	load_png_info_d->pos_tb = pos_tb;
	load_png_info_d->neg_tb = neg_tb;
	load_png_info_d->steps_spin = steps_spin;
	load_png_info_d->cfg_spin = cfg_spin;
	load_png_info_d->clip_skip_spin = clip_skip_spin;
	load_png_info_d->seed_entry = seed_entry;
	load_png_info_d->width_dd = width_dd;
	load_png_info_d->height_dd = height_dd;
	load_png_info_d->checkpoint_dd = checkpoint_dd;
	load_png_info_d->vae_dd = vae_dd;
	load_png_info_d->sampler_dd = sampler_dd;
	load_png_info_d->scheduler_dd = scheduler_dd;
	load_png_info_d->hires_upscaler_dd = hires_upscaler_dd;
	load_png_info_d->hires_scale_spin = hires_scale_spin;
	load_png_info_d->hires_steps_spin = hires_steps_spin;
	load_png_info_d->hires_denoise_spin = hires_denoise_spin;
	g_signal_connect (load_from_img_btn, "clicked", G_CALLBACK (load_from_img_btn_cb), load_png_info_d);
	g_signal_connect (load_from_current_btn, "clicked", G_CALLBACK (load_from_img_preview), load_png_info_d);
	g_signal_connect (load_from_img_btn, "destroy", G_CALLBACK (on_load_from_img_btn_destroy), load_png_info_d);
	
	load_img2img_file_d = g_new0 (LoadImg2ImgData, 1);
	load_img2img_file_d->win = win;
	load_img2img_file_d->img2img_expander = img2img_expander;
	load_img2img_file_d->overlay_img2img = overlay_img2img;
	load_img2img_file_d->inpaint_check = inpaint_check;
	load_img2img_file_d->image_wgt = preview_img2img;
	load_img2img_file_d->img2img_file_path = app_data->img2img_file_path;
	g_signal_connect (load_img2img_btn, "clicked", G_CALLBACK (load_img2img_btn_cb), load_img2img_file_d);
	g_signal_connect (clear_img2img_btn, "clicked", G_CALLBACK (clear_img2img_btn_cb), load_img2img_file_d);
	g_signal_connect (mask_img2img_btn, "clicked", G_CALLBACK (show_mask_area), load_img2img_file_d);
	g_signal_connect (clear_mask_btn, "clicked", G_CALLBACK (clear_mask_btn_cb), load_img2img_file_d);
	g_signal_connect (clear_img2img_btn, "destroy", G_CALLBACK (on_clear_img2img_btn_destroy), load_img2img_file_d);
	
	load_img2img_from_preview_d = g_new0 (LoadImg2ImgFromPreviewData, 1);
	load_img2img_from_preview_d->current_image_index = &app_data->preview_image_index;
	load_img2img_from_preview_d->image_files = app_data->preview_image_files;
	load_img2img_from_preview_d->img2img_file_path = app_data->img2img_file_path;
	load_img2img_from_preview_d->image_wgt = preview_img2img;
	load_img2img_from_preview_d->img2img_expander = img2img_expander;
	load_img2img_from_preview_d->overlay_img2img = overlay_img2img;
	load_img2img_from_preview_d->inpaint_check = inpaint_check;
	g_signal_connect (set_img2img_from_preview_btn, "clicked", G_CALLBACK (set_current_preview_to_img2img), load_img2img_from_preview_d);
	g_signal_connect (set_img2img_from_preview_btn, "destroy", G_CALLBACK (on_set_img2img_from_preview_btn_destroy), load_img2img_from_preview_d);

	gen_d = g_new0 (GenerationData, 1);
	gen_d->app_data = app_data;
	gen_d->pos_p = pos_tb;
	gen_d->neg_p = neg_tb;
	gen_d->preview_image_widget = preview_img;
	gen_d->preview_label = img_index_label;
	gen_d->preview_image_toggle_visibility_btn = hide_img_btn;
	gen_d->halt_btn = sd_halt_btn;
	gen_d->win = win;
	g_signal_connect (generate_btn, "clicked", G_CALLBACK (prepare_gen_data), gen_d);
	g_signal_connect (generate_btn, "destroy", G_CALLBACK (on_generate_btn_destroy), gen_d);

	g_signal_connect (info_btn, "clicked", G_CALLBACK (show_info_message), win);
	g_signal_connect (win, "close-request", G_CALLBACK (close_app_callback), user_data);
	
	gtk_window_present (GTK_WINDOW (win));
}

int
main (int argc, char **argv)
{
	// This fixes locale-dependent decimal separators.
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

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
	
	data->preview_label_string = g_string_new("0 / 0");
	if (data->preview_label_string == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->preview_image_files = g_ptr_array_new_full(1, g_free);
	if (data->preview_image_files == NULL) {
		g_error("Failed to allocate GPtrArray.");
		return 1;
	}
	
	data->sd_cmd_array = g_ptr_array_new_full(1, g_free);
	if (data->sd_cmd_array == NULL) {
		g_error("Failed to allocate GPtrArray.");
		return 1;
	}
	
	data->preview_image_index = 0;
	data->sdpid = 0;
	
	int s;

	app = gtk_application_new ("com.github.LuizAlcantara.NeuralPixel", G_APPLICATION_NON_UNIQUE);
	g_signal_connect (app, "activate", G_CALLBACK (app_activate), data);
	s = g_application_run (G_APPLICATION (app), argc, argv);
	if (data != NULL) {
		g_free(data);
	}
	g_object_unref (app);
	return s;
}

