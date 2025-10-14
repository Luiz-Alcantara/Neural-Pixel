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
#include "global.h"
#include "handle_cache.h"
#include "png_utils.h"
#include "str_utils.h"
#include "structs.h"
#include "widgets_cb.h"

int globalSDPID = 0;

static void
app_activate (GApplication *app, gpointer user_data)
{
	AppStartData *app_data = user_data;
	load_cache(user_data);

	//Defining GTK Widgets
	GtkWidget *win;

	GtkCssProvider *css_provider;

	GtkWidget *box;

	GtkWidget *box_left, *boxl_topbar;
	GtkWidget *quit_btn, *reload_btn, *reset_default_btn, *load_from_img_btn;

	GtkWidget *box_r1 ,*box_r1_c1, *box_r1_c2;
	GtkWidget *pp_lab;
	GtkWidget *pos_scr;
	GtkWidget *pos_tv;
	GtkTextBuffer *pos_tb;

	GtkWidget *box_r2, *box_r2_c1, *box_r2_c2;
	GtkWidget *np_lab;
	GtkWidget *neg_scr;
	GtkWidget *neg_tv;
	GtkTextBuffer *neg_tb;

	GtkWidget *box_r3, *box_r3_c1, *box_r3_c2;
	GtkWidget *generate_btn;
	GtkWidget *model_lab, *vae_lab;
	GtkWidget *model_dd, *vae_dd;

	GtkWidget *box_r4, *box_r4_c1, *box_r4_c2;
	GtkWidget *cnet_lab, *upscale_lab;
	GtkWidget *cnet_dd, *upscale_dd;
	
	GtkWidget *box_r5, *box_r5_c1, *box_r5_c2, *box_r5_c3;
	GtkWidget *clip_l_lab, *clip_g_lab, *t5xxl_lab;
	GtkWidget *clip_l_dd, *clip_g_dd, *t5xxl_dd;

	GtkWidget *box_r6, *box_r6_c1, *box_r6_c2, *box_r6_c3, *box_r6_c4, *box_r6_c5;
	GtkWidget *cfg_lab, *denoise_lab, *clip_skip_lab, *upscale_str_lab, *seed_lab;
	GtkWidget *cfg_spin, *denoise_spin, *clip_skip_spin, *upscale_spin, *seed_entry;

	GtkWidget *box_r7, *box_r7_c1, *box_r7_c2;
	GtkWidget *lora_lab, *embedding_lab;
	GtkWidget *lora_dd, *embedding_dd;

	GtkWidget *box_r8, *box_r8_c1, *box_r8_c2, *box_r8_c3, *box_r8_c4, *box_r8_c5, *box_r8_c6;
	GtkWidget *sample_lab, *schedule_lab, *steps_lab, *width_lab, *height_lab, *n_images_lab;
	GtkWidget *sample_dd, *schedule_dd, *steps_dd, *width_dd, *height_dd, *batch_dd;

	GtkWidget *box_r9, *box_r9_c1, *box_r9_c2, *box_r9_c3;
	GtkWidget *sd_halt_btn, *low_vram_btn;

	GtkWidget *vram_popover;

	GtkWidget *opts_box;
	GtkWidget *cpu_check, *tiling_check, *clip_check, *cnet_check, *vae_check, *flash_check, *taesd_check, *verbose_check;

	GtkWidget *box_right, *boxr_topbar, *boxr_img;
	GtkWidget *hide_img_btn, *load_img2img_btn, *clear_img2img_btn;
	GtkWidget *preview_img;

	ReloadDropDownData *reload_d;
	ResetCbData *reset_d;
	GenerationData *gen_d;
	PreviewImageData *preview_d;
	LoadPNGData *load_png_info_d;
	LoadImg2ImgData *load_img2img_file_d;
	SeedEntryData *seed_entry_d;
	//End defining GTK Widgets;

	win = gtk_application_window_new (GTK_APPLICATION (app));
	gtk_window_set_title (GTK_WINDOW (win), "Neural Pixel");
	gtk_window_set_default_size (GTK_WINDOW (win), 900, 800);

	//Load css file 
	css_provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_file (css_provider, g_file_new_for_path ("resources/styles.css"));
	gtk_style_context_add_provider_for_display (gtk_widget_get_display (win), GTK_STYLE_PROVIDER (css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	//Main box
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_set_homogeneous (GTK_BOX (box), TRUE);
	gtk_window_set_child (GTK_WINDOW (win), box);

	//Set Box Left and Right
	box_left = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_widget_set_hexpand (box_left, TRUE);
	gtk_box_append (GTK_BOX (box), box_left);
	box_right = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_widget_set_name(box_right, "image_window");
	gtk_box_append (GTK_BOX (box), box_right);


	//Set Box Left Button bar
	boxl_topbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_set_homogeneous (GTK_BOX (boxl_topbar), TRUE);
	gtk_box_append (GTK_BOX (box_left), boxl_topbar);

	quit_btn = gtk_button_new_with_label ("Close App");
	gtk_box_append (GTK_BOX (boxl_topbar), quit_btn);
	reload_btn = gtk_button_new_with_label ("Refresh Files");
	gtk_box_append (GTK_BOX (boxl_topbar), reload_btn);
	reset_default_btn = gtk_button_new_with_label ("Reset to Default");
	gtk_box_append (GTK_BOX (boxl_topbar), reset_default_btn);
	load_from_img_btn = gtk_button_new_with_label ("Load PNG Info");
	gtk_box_append (GTK_BOX (boxl_topbar), load_from_img_btn);

	//Set Box Row=1
	box_r1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_set_homogeneous (GTK_BOX (box_r1), TRUE);
	gtk_box_append (GTK_BOX (box_left), box_r1);

	//Create Row 1 Labels
	pp_lab = gtk_label_new ("Positive Prompt:");
	np_lab = gtk_label_new ("Negative Prompt:");

	//Set Box Row=2 and col 1 and 2
	box_r2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_widget_set_hexpand (box_r2, TRUE);
	gtk_box_set_homogeneous (GTK_BOX (box_r2), TRUE);
	gtk_box_append (GTK_BOX (box_left), box_r2);

	box_r2_c1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r2), box_r2_c1);
	gtk_box_append (GTK_BOX (box_r2_c1), pp_lab);

	box_r2_c2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r2), box_r2_c2);
	gtk_box_append (GTK_BOX (box_r2_c2), np_lab);

	//Set positive textview
	pos_scr = gtk_scrolled_window_new ();
	pos_tv = gtk_text_view_new ();
	pos_tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (pos_tv));
	gtk_widget_set_hexpand (pos_tv, TRUE);
	gtk_widget_set_vexpand (pos_tv, TRUE);
	load_pp_cache(pos_tb);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (pos_tv), GTK_WRAP_WORD_CHAR);
	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (pos_scr), pos_tv);

	gtk_box_append (GTK_BOX (box_r2_c1), pos_scr);

	//Set negative textview
	neg_scr = gtk_scrolled_window_new ();
	neg_tv = gtk_text_view_new ();
	neg_tb = gtk_text_view_get_buffer (GTK_TEXT_VIEW (neg_tv));
	gtk_widget_set_hexpand (neg_tv, TRUE);
	gtk_widget_set_vexpand (neg_tv, TRUE);
	load_np_cache(neg_tb);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (neg_tv), GTK_WRAP_WORD_CHAR);
	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (neg_scr), neg_tv);

	gtk_box_append (GTK_BOX (box_r2_c2), neg_scr);

	//Set Box Row 3, col 1 and 2
	box_r3 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_set_homogeneous (GTK_BOX (box_r3), TRUE);
	gtk_box_append (GTK_BOX (box_left), box_r3);

	box_r3_c1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r3), box_r3_c1);

	box_r3_c2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r3), box_r3_c2);

	generate_btn = gtk_button_new_with_label ("Generate");
	gtk_widget_set_name(generate_btn, "gen_btn");

	model_lab = gtk_label_new ("Model:");
	vae_lab = gtk_label_new ("VAE:");

	gtk_box_append (GTK_BOX (box_r3_c1), model_lab);
	gtk_box_append (GTK_BOX (box_r3_c2), vae_lab);

	model_dd = gen_path_dd(CHECKPOINTS_PATH, 22, NULL, 0, app_data->model_string, generate_btn, app, 1);
	gtk_box_append (GTK_BOX (box_r3_c1), model_dd);

	vae_dd = gen_path_dd(VAES_PATH, 22, NULL, 0, app_data->vae_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_r3_c2), vae_dd);

	//Set Box Row 4, col 1 and 2
	box_r4 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_set_homogeneous (GTK_BOX (box_r4), TRUE);
	gtk_box_append (GTK_BOX (box_left), box_r4);

	box_r4_c1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r4), box_r4_c1);

	box_r4_c2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r4), box_r4_c2);

	cnet_lab = gtk_label_new ("Control Net:");
	upscale_lab = gtk_label_new ("Upscale:");

	gtk_box_append (GTK_BOX (box_r4_c1), cnet_lab);
	gtk_box_append (GTK_BOX (box_r4_c2), upscale_lab);

	cnet_dd = gen_path_dd(CONTROLNET_PATH, 22, NULL, 0, app_data->cnet_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_r4_c1), cnet_dd);

	upscale_dd = gen_path_dd(UPSCALES_PATH, 22, NULL, 0, app_data->upscale_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_r4_c2), upscale_dd);


	//Set Box Row 5, col 1, 2 and 3
	box_r5 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_set_homogeneous (GTK_BOX (box_r5), TRUE);
	gtk_box_append (GTK_BOX (box_left), box_r5);
	
	box_r5_c1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r5), box_r5_c1);

	box_r5_c2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r5), box_r5_c2);

	box_r5_c3 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r5), box_r5_c3);
	
	clip_l_lab = gtk_label_new ("Clip_l:");
	clip_g_lab = gtk_label_new ("Clip_g:");
	t5xxl_lab = gtk_label_new ("T5xxl:");
	
	gtk_box_append (GTK_BOX (box_r5_c1), clip_l_lab);
	gtk_box_append (GTK_BOX (box_r5_c2), clip_g_lab);
	gtk_box_append (GTK_BOX (box_r5_c3), t5xxl_lab);
	
	clip_l_dd = gen_path_dd(CLIPS_PATH, 22, NULL, 0, app_data->clip_l_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_r5_c1), clip_l_dd);
	
	clip_g_dd = gen_path_dd(CLIPS_PATH, 22, NULL, 0, app_data->clip_g_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_r5_c2), clip_g_dd);
	
	t5xxl_dd = gen_path_dd(TEXT_ENCODERS_PATH, 22, NULL, 0, app_data->t5xxl_string, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_r5_c3), t5xxl_dd);


	//Set Box Row 6, col 1 to 5
	box_r6 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_append (GTK_BOX (box_left), box_r6);

	box_r6_c1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r6), box_r6_c1);

	box_r6_c2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r6), box_r6_c2);

	box_r6_c3 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r6), box_r6_c3);

	box_r6_c4 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r6), box_r6_c4);
	
	box_r6_c5 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_widget_set_hexpand (box_r6_c5, TRUE);
	gtk_box_append (GTK_BOX (box_r6), box_r6_c5);

	cfg_lab = gtk_label_new ("CFG Scale:");
	gtk_box_append (GTK_BOX (box_r6_c1), cfg_lab);
	cfg_spin = gtk_spin_button_new_with_range (1, 30.0, 0.5);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(cfg_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(cfg_spin), app_data->cfg_value);
	g_signal_connect (cfg_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->cfg_value);
	gtk_box_append (GTK_BOX (box_r6_c1), cfg_spin);

	denoise_lab = gtk_label_new ("Denoise Str:");
	gtk_box_append (GTK_BOX (box_r6_c2), denoise_lab);
	denoise_spin = gtk_spin_button_new_with_range (0, 1.0, 0.05);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(denoise_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(denoise_spin), app_data->denoise_value);
	g_signal_connect (denoise_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->denoise_value);
	gtk_box_append (GTK_BOX (box_r6_c2), denoise_spin);
	
	clip_skip_lab = gtk_label_new ("Clip Skip:");
	gtk_box_append (GTK_BOX (box_r6_c3), clip_skip_lab);
	clip_skip_spin = gtk_spin_button_new_with_range (0, 12.0, 1.0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(clip_skip_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(clip_skip_spin), app_data->clip_skip_value);
	g_signal_connect (clip_skip_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->clip_skip_value);
	gtk_box_append (GTK_BOX (box_r6_c3), clip_skip_spin);

	upscale_str_lab = gtk_label_new ("Upscale Runs:");
	gtk_box_append (GTK_BOX (box_r6_c4), upscale_str_lab);
	upscale_spin = gtk_spin_button_new_with_range (1.0, 8.0, 1.0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(upscale_spin), TRUE);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON(upscale_spin), app_data->up_repeat_value);
	g_signal_connect (upscale_spin, "value-changed", G_CALLBACK (set_spin_value_to_var), &app_data->up_repeat_value);
	gtk_box_append (GTK_BOX (box_r6_c4), upscale_spin);
	
	seed_lab = gtk_label_new ("Seed:");
	gtk_box_append (GTK_BOX (box_r6_c5), seed_lab);
	seed_entry = gtk_entry_new();
	gtk_entry_set_icon_from_icon_name(GTK_ENTRY(seed_entry), GTK_ENTRY_ICON_SECONDARY, "media-playlist-shuffle-symbolic");
	gtk_entry_set_icon_activatable(GTK_ENTRY(seed_entry), GTK_ENTRY_ICON_SECONDARY, TRUE);
	gtk_entry_set_icon_tooltip_text(GTK_ENTRY(seed_entry), GTK_ENTRY_ICON_SECONDARY, "Sets to -1 = random seed");
	g_signal_connect(seed_entry, "icon-release", G_CALLBACK(random_seed_btn_toggle), NULL);
	char seed_str[LONGLONG_STR_SIZE];
	snprintf(seed_str, sizeof(seed_str), "%lld", app_data->seed_value);
	gtk_editable_set_text(GTK_EDITABLE(seed_entry), seed_str);
	gtk_box_append (GTK_BOX (box_r6_c5), seed_entry);


	//Set Box Row 7, col 1, and 2
	box_r7 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_set_homogeneous (GTK_BOX (box_r7), TRUE);
	gtk_box_append (GTK_BOX (box_left), box_r7);

	box_r7_c1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r7), box_r7_c1);

	box_r7_c2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r7), box_r7_c2);

	lora_lab = gtk_label_new ("Add LoRA:");
	embedding_lab = gtk_label_new ("Add Embedding:");

	gtk_box_append (GTK_BOX (box_r7_c1), lora_lab);
	gtk_box_append (GTK_BOX (box_r7_c2), embedding_lab);

	lora_dd = gen_path_dd(LORAS_PATH, 48, pos_tb, 1, NULL, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_r7_c1), lora_dd);

	embedding_dd = gen_path_dd(EMBEDDINGS_PATH, 48, neg_tb, 0, NULL, NULL, app, 0);
	gtk_box_append (GTK_BOX (box_r7_c2), embedding_dd);

	//Set Box Row 8, col 1 to 6

	box_r8 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_set_homogeneous (GTK_BOX (box_r8), TRUE);
	gtk_box_append (GTK_BOX (box_left), box_r8);

	box_r8_c1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r8), box_r8_c1);

	box_r8_c2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r8), box_r8_c2);

	box_r8_c3 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r8), box_r8_c3);

	box_r8_c4 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r8), box_r8_c4);

	box_r8_c5 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r8), box_r8_c5);

	box_r8_c6 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_append (GTK_BOX (box_r8), box_r8_c6);

	//Set DropDown for Sample, Schedule, Steps, Width, Height and N_images
	sample_lab = gtk_label_new ("Sampler:");
	schedule_lab = gtk_label_new ("Scheduler:");
	steps_lab = gtk_label_new ("Steps:");
	width_lab = gtk_label_new ("Width:");
	height_lab = gtk_label_new ("Height:");
	n_images_lab = gtk_label_new ("Batch count:");

	gtk_box_append (GTK_BOX (box_r8_c1), sample_lab);
	gtk_box_append (GTK_BOX (box_r8_c2), schedule_lab);
	gtk_box_append (GTK_BOX (box_r8_c3), steps_lab);
	gtk_box_append (GTK_BOX (box_r8_c4), width_lab);
	gtk_box_append (GTK_BOX (box_r8_c5), height_lab);
	gtk_box_append (GTK_BOX (box_r8_c6), n_images_lab);

	sample_dd = gen_const_dd(LIST_SAMPLES, &app_data->sample_index);
	gtk_box_append (GTK_BOX (box_r8_c1), sample_dd);

	schedule_dd = gen_const_dd(LIST_SCHEDULES, &app_data->schedule_index);
	gtk_box_append (GTK_BOX (box_r8_c2), schedule_dd);

	steps_dd = gen_const_dd(LIST_STEPS_STR, &app_data->n_steps_index);
	gtk_box_append (GTK_BOX (box_r8_c3), steps_dd);

	width_dd = gen_const_dd(LIST_RESOLUTIONS_STR, &app_data->w_index);
	gtk_box_append (GTK_BOX (box_r8_c4), width_dd);

	height_dd = gen_const_dd(LIST_RESOLUTIONS_STR, &app_data->h_index);
	gtk_box_append (GTK_BOX (box_r8_c5), height_dd);

	batch_dd = gen_const_dd(LIST_STEPS_STR, &app_data->bs_index);
	gtk_box_append (GTK_BOX (box_r8_c6), batch_dd);

	//Set Box Row 9, col 1, 2 and 3
	box_r9 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_append (GTK_BOX (box_left), box_r9);

	box_r9_c1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_append (GTK_BOX (box_r9), box_r9_c1);

	box_r9_c2 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_append (GTK_BOX (box_r9), box_r9_c2);

	box_r9_c3 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_append (GTK_BOX (box_r9), box_r9_c3);

	sd_halt_btn = gtk_button_new_with_label ("Stop");
	g_signal_connect (sd_halt_btn, "clicked", G_CALLBACK (kill_stable_diffusion_process), NULL);
	gtk_widget_set_sensitive(GTK_WIDGET(sd_halt_btn), FALSE);
	gtk_box_append (GTK_BOX (box_r9_c1), sd_halt_btn);


	gtk_widget_set_hexpand (generate_btn, TRUE);
	gtk_box_append (GTK_BOX (box_r9_c2), generate_btn);

	low_vram_btn = gtk_menu_button_new();
	gtk_menu_button_set_label(GTK_MENU_BUTTON(low_vram_btn), "Extra Opts.");
	gtk_box_append (GTK_BOX (box_r9_c3), low_vram_btn);

	vram_popover = gtk_popover_new();
	gtk_widget_set_halign(vram_popover, GTK_ALIGN_START);
	gtk_widget_set_valign(vram_popover, GTK_ALIGN_START);

	opts_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	
	cpu_check = gtk_check_button_new_with_label("Run in CPU");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(cpu_check), app_data->cpu_bool == 1 ? TRUE : FALSE);
	g_signal_connect(cpu_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->cpu_bool);
	gtk_box_append (GTK_BOX (opts_box), cpu_check);

	tiling_check = gtk_check_button_new_with_label("VAE Tiling");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(tiling_check), app_data->vt_bool == 1 ? TRUE : FALSE);
	g_signal_connect(tiling_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->vt_bool);
	gtk_box_append (GTK_BOX (opts_box), tiling_check);

	clip_check = gtk_check_button_new_with_label("Keep Clip in CPU");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(clip_check), app_data->k_clip_bool == 1 ? TRUE : FALSE);
	g_signal_connect(clip_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->k_clip_bool);
	gtk_box_append (GTK_BOX (opts_box), clip_check);

	cnet_check = gtk_check_button_new_with_label("Keep CNet in CPU");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(cnet_check), app_data->k_cnet_bool == 1 ? TRUE : FALSE);
	g_signal_connect(cnet_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->k_cnet_bool);
	gtk_box_append (GTK_BOX (opts_box), cnet_check);

	vae_check = gtk_check_button_new_with_label("Keep VAE in CPU");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(vae_check), app_data->k_vae_bool == 1 ? TRUE : FALSE);
	g_signal_connect(vae_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->k_vae_bool);
	gtk_box_append (GTK_BOX (opts_box), vae_check);

	flash_check = gtk_check_button_new_with_label("Flash Attention");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(flash_check), app_data->fa_bool == 1 ? TRUE : FALSE);
	g_signal_connect(flash_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->fa_bool);
	gtk_box_append (GTK_BOX (opts_box), flash_check);
	
	taesd_check = gtk_check_button_new_with_label("Enable TAESD");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(taesd_check), app_data->taesd_bool == 1 ? TRUE : FALSE);
	g_signal_connect(taesd_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->taesd_bool);
	gtk_box_append (GTK_BOX (opts_box), taesd_check);
	
	verbose_check = gtk_check_button_new_with_label("Terminal Verbose");
	gtk_check_button_set_active(GTK_CHECK_BUTTON(verbose_check), app_data->verbose_bool == 1 ? TRUE : FALSE);
	g_signal_connect(verbose_check, "toggled", G_CALLBACK(toggle_extra_options), &app_data->verbose_bool);
	gtk_box_append (GTK_BOX (opts_box), verbose_check);

	gtk_popover_set_child(GTK_POPOVER(vram_popover), opts_box);

	gtk_menu_button_set_popover(GTK_MENU_BUTTON(low_vram_btn), vram_popover);

	//Set Box Right Button bar
	boxr_topbar = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_set_homogeneous (GTK_BOX (boxr_topbar), TRUE);
	gtk_box_append (GTK_BOX (box_right), boxr_topbar);

	hide_img_btn = gtk_button_new_from_icon_name ("view-reveal-symbolic");
	load_img2img_btn = gtk_button_new_with_label ("Load img2img");
	gtk_box_append (GTK_BOX (boxr_topbar), load_img2img_btn);
	clear_img2img_btn = gtk_button_new_with_label ("Clear img2img");
	gtk_box_append (GTK_BOX (boxr_topbar), clear_img2img_btn);

	//Set Box Right Image
	boxr_img = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
	gtk_box_set_homogeneous (GTK_BOX (boxr_img), TRUE);
	gtk_widget_set_hexpand (boxr_img, TRUE);
	gtk_widget_set_vexpand (boxr_img, TRUE);
	gtk_box_append (GTK_BOX (box_right), boxr_img);

	preview_img = gtk_picture_new_for_filename (DEFAULT_IMG_PATH);
	load_img_cache(preview_img);
	gtk_box_append (GTK_BOX (boxr_topbar), hide_img_btn);
	gtk_box_append (GTK_BOX (boxr_img), preview_img);
	
	seed_entry_d = g_new0 (SeedEntryData, 1);
	seed_entry_d->seed = &app_data->seed_value;
	seed_entry_d->win = win;
	g_signal_connect(gtk_editable_get_delegate(GTK_EDITABLE(seed_entry)),
	"insert-text", G_CALLBACK(seed_entry_int_filter), seed_entry_d);

	reload_d = g_new0 (ReloadDropDownData, 1);
	reload_d->app = app;
	reload_d->model_dd = model_dd;
	reload_d->vae_dd = vae_dd;
	reload_d->cnet_dd = cnet_dd;
	reload_d->upscale_dd = upscale_dd;
	reload_d->clip_l_dd = clip_l_dd;
	reload_d->clip_g_dd = clip_g_dd;
	reload_d->t5xxl_dd = t5xxl_dd;
	reload_d->lora_dd = lora_dd;
	reload_d->embedding_dd = embedding_dd;
	g_signal_connect (reload_btn, "clicked", G_CALLBACK (reload_dropdown), reload_d);
	g_signal_connect (reload_btn, "destroy", G_CALLBACK (on_reload_btn_destroy), reload_d);

	reset_d = g_new0 (ResetCbData, 1);
	reset_d->pos_tb = pos_tb;
	reset_d->neg_tb = neg_tb;
	reset_d->model_dd = model_dd;
	reset_d->vae_dd = vae_dd;
	reset_d->cnet_dd = cnet_dd;
	reset_d->upscale_dd = upscale_dd;
	reset_d->clip_l_dd = clip_l_dd;
	reset_d->clip_g_dd = clip_g_dd;
	reset_d->t5xxl_dd = t5xxl_dd;
	reset_d->cfg_spin = cfg_spin;
	reset_d->denoise_spin = denoise_spin;
	reset_d->seed_entry = seed_entry;
	reset_d->upscale_spin = upscale_spin;
	reset_d->lora_dd = lora_dd;
	reset_d->embedding_dd = embedding_dd;
	reset_d->sample_dd = sample_dd;
	reset_d->schedule_dd = schedule_dd;
	reset_d->steps_dd = steps_dd;
	reset_d->width_dd = width_dd;
	reset_d->height_dd = height_dd;
	reset_d->batch_dd = batch_dd;
	reset_d->cpu_check = cpu_check;
	reset_d->tiling_check = tiling_check;
	reset_d->clip_check = clip_check;
	reset_d->cnet_check = cnet_check;
	reset_d->vae_check = vae_check;
	reset_d->flash_check = flash_check;
	g_signal_connect (reset_default_btn, "clicked", G_CALLBACK (reset_default_btn_cb), reset_d);
	g_signal_connect (reset_default_btn, "destroy", G_CALLBACK (on_reset_default_btn_destroy), reset_d);

	preview_d = g_new0 (PreviewImageData, 1);
	preview_d->image_widget = preview_img;
	g_signal_connect (hide_img_btn, "clicked", G_CALLBACK (hide_img_btn_cb), preview_d);
	g_signal_connect (hide_img_btn, "destroy", G_CALLBACK (on_hide_img_btn_destroy), preview_d);

	load_png_info_d = g_new0 (LoadPNGData, 1);
	load_png_info_d->win = win;
	load_png_info_d->pos_tb = pos_tb;
	load_png_info_d->neg_tb = neg_tb;
	load_png_info_d->steps_dd = steps_dd;
	load_png_info_d->cfg_spin = cfg_spin;
	load_png_info_d->seed_entry = seed_entry;
	load_png_info_d->width_dd = width_dd;
	load_png_info_d->height_dd = height_dd;
	load_png_info_d->model_dd = model_dd;
	load_png_info_d->sample_dd = sample_dd;
	load_png_info_d->schedule_dd = schedule_dd;
	g_signal_connect (load_from_img_btn, "clicked", G_CALLBACK (load_from_img_btn_cb), load_png_info_d);
	g_signal_connect (load_from_img_btn, "destroy", G_CALLBACK (on_load_from_img_btn_destroy), load_png_info_d);
	
	load_img2img_file_d = g_new0 (LoadImg2ImgData, 1);
	load_img2img_file_d->win = win;
	load_img2img_file_d->image_wgt = preview_img;
	load_img2img_file_d->img2img_file_path = app_data->img2img_file_path;
	g_signal_connect (load_img2img_btn, "clicked", G_CALLBACK (load_img2img_btn_cb), load_img2img_file_d);
	g_signal_connect (clear_img2img_btn, "clicked", G_CALLBACK (clear_img2img_btn_cb), load_img2img_file_d);
	g_signal_connect (clear_img2img_btn, "destroy", G_CALLBACK (on_clear_img2img_btn_destroy), load_img2img_file_d);

	gen_d = g_new0 (GenerationData, 1);
	gen_d->model_string = app_data->model_string;
	gen_d->vae_string = app_data->vae_string;
	gen_d->cnet_string = app_data->cnet_string;
	gen_d->upscale_string = app_data->upscale_string;
	gen_d->clip_l_string = app_data->clip_l_string;
	gen_d->clip_g_string = app_data->clip_g_string;
	gen_d->t5xxl_string = app_data->t5xxl_string;
	gen_d->sample_index = &app_data->sample_index;
	gen_d->schedule_index = &app_data->schedule_index;
	gen_d->n_steps_index = &app_data->n_steps_index;
	gen_d->w_index = &app_data->w_index;
	gen_d->h_index = &app_data->h_index;
	gen_d->bs_index = &app_data->bs_index;
	gen_d->cpu_bool = &app_data->cpu_bool;
	gen_d->vt_bool = &app_data->vt_bool;
	gen_d->k_clip_bool = &app_data->k_clip_bool;
	gen_d->k_cnet_bool = &app_data->k_cnet_bool;
	gen_d->k_vae_bool = &app_data->k_vae_bool;
	gen_d->fa_bool = &app_data->fa_bool;
	gen_d->taesd_bool = &app_data->taesd_bool;
	gen_d->verbose_bool = &app_data->verbose_bool;
	gen_d->seed_value = &app_data->seed_value;
	gen_d->cfg_value = &app_data->cfg_value;
	gen_d->denoise_value = &app_data->denoise_value;
	gen_d->clip_skip_value = &app_data->clip_skip_value;
	gen_d->up_repeat_value = &app_data->up_repeat_value;
	gen_d->pos_p = pos_tb;
	gen_d->neg_p = neg_tb;
	gen_d->image_widget = preview_img;
	gen_d->show_img_btn = hide_img_btn;
	gen_d->halt_btn = sd_halt_btn;
	gen_d->win = win;
	gen_d->img2img_file_path = app_data->img2img_file_path;
	g_signal_connect (generate_btn, "clicked", G_CALLBACK (generate_cb), gen_d);
	g_signal_connect (generate_btn, "destroy", G_CALLBACK (on_generate_btn_destroy), gen_d);

	g_signal_connect (quit_btn, "clicked", G_CALLBACK (quit_btn_callback), win);
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
	data->model_string = NULL;
	data->vae_string = NULL;
	data->cnet_string = NULL;
	data->upscale_string = NULL;
	data->clip_l_string = NULL;
	data->clip_g_string = NULL;
	data->t5xxl_string = NULL;
	data->img2img_file_path = NULL;
	
	data->model_string = g_string_new("None");
	if (data->model_string == NULL) {
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
	
	data->upscale_string = g_string_new("None");
	if (data->upscale_string == NULL) {
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
	
	data->t5xxl_string = g_string_new("None");
	if (data->t5xxl_string == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
	data->img2img_file_path = g_string_new("None");
	if (data->img2img_file_path == NULL) {
		g_error("Failed to allocate GString.");
		return 1;
	}
	
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

