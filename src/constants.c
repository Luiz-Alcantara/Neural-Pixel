// constants.c
#include <stdio.h>
#include "constants.h"

const char* APP_NAME_VERSION = "Neural Pixel v0.5.1";
const char* APP_AUTHOR = "Copyright © 2024-2026, Luiz Alcantara";
const char* APP_DESC = "\nA free and simple GTK4 frontend for stable-diffusion.cpp";
const char* APP_DESC2 = "This application includes 'stable-diffusion.cpp'\n created by @leejet";
const char* NP_GITHUB = "https://github.com/Luiz-Alcantara/Neural-Pixel";
const char* SDCPP_GITHUB = "https://github.com/leejet/stable-diffusion.cpp";

const char* CACHE_PATH = "./.cache";
const char* MODELS_PATH = "./models";
const char* CHECKPOINTS_PATH = "./models/checkpoints";
const char* CLIPS_PATH = "./models/clips";
const char* CONTROLNET_PATH = "./models/controlnet";
const char* EMBEDDINGS_PATH = "./models/embeddings";
const char* LORAS_PATH = "./models/loras";
const char* TEXT_ENCODERS_PATH = "./models/text_encoders";
const char* UNET_PATH = "./models/unet";
const char* UPSCALES_PATH = "./models/upscale_models";
const char* VAES_PATH = "./models/vae";
const char* OUTPUTS_PATH = "./outputs";


const char* LIST_RESOLUTIONS_STR[] = {"64", "128", "192", "256", "320", "384", "448", "512", "576", "640", "704", "768", "832", "896", "960", "1024", "1088", "1152", "1216", "1280", "1344", "1408", "1472", "1536", "1600", "1664", "1728", "1792", "1856", "1920", "1984", "2048", NULL};
const size_t LIST_RESOLUTIONS_STR_COUNT = sizeof(LIST_RESOLUTIONS_STR) / sizeof(LIST_RESOLUTIONS_STR[0]);

const char* LIST_STEPS_STR[] = {"1", "2", "4", "8", "12", "16", "20", "24", "30", "36", "42", "50", "60", NULL};
const size_t LIST_STEPS_STR_COUNT = sizeof(LIST_STEPS_STR) / sizeof(LIST_STEPS_STR[0]);

const char* LIST_SAMPLES[] = {"ddim_trailing", "dpm++2m", "dpm++2mv2", "dpm++2s_a", "dpm2", "euler", "euler_a", "heun", "ipndm", "ipndm_v", "lcm", "tcd", NULL};
const size_t LIST_SAMPLES_COUNT = sizeof(LIST_SAMPLES) / sizeof(LIST_SAMPLES[0]);  

const char* LIST_SCHEDULES[] = {"ays", "discrete", "exponential", "gits", "karras", "kl_optimal", "lcm", "sgm_uniform", "simple", "smoothstep", NULL};
const size_t LIST_SCHEDULES_COUNT = sizeof(LIST_SCHEDULES) / sizeof(LIST_SCHEDULES[0]); 
 
const char* POSITIVE_PROMPT = "masterpiece, ultra-detailed snow globe glowing softly on a wooden table, magical internal light illuminating a miniature winter village inside, tiny houses with warm windows, snow-covered pine trees, swirling snow particles, crystal glass sphere with smooth reflections, subtle frost and condensation, ornate metal base, cinematic lighting, dark cozy room background, deep shadows with soft rim light, volumetric glow, dreamy atmosphere, shallow depth of field, macro perspective, highly detailed, fantasy realism, dramatic contrast, beautiful composition";
const char* NEGATIVE_PROMPT = "worst quality, low quality, blurry, flat lighting, harsh shadows, oversaturated, plastic look, bad reflections, warped glass, extra objects, cluttered composition, cartoon, anime, photorealistic face, text, watermark, logo";
const char* OPTIONAL_ITEMS = "None";
const char* DEFAULT_IMG_PATH = "./resources/example.png";
const char* EMPTY_IMG_PATH = "./resources/empty.png";
const int DEFAULT_MODELS = 0;
const int DEFAULT_SAMPLER = 6;
const int DEFAULT_SCHEDULER = 4;
const int DEFAULT_SIZE = 7;
const int ENABLED_OPT = 1;
const int DISABLED_OPT = 0;
const int LONGLONG_STR_SIZE = 21;
const int ZERO_SPACING = 0;
const int SMALL_SPACING = 2;
const int MEDIUM_SPACING = 6;
const int LARGE_SPACING = 12;
const int HUGE_SPACING = 24;
const int LABEL_ALIGNMENT = 3;
const int DD_STRING_LEN = 40;
const long long int DEFAULT_SEED = -1;
const double DEFAULT_CFG = 6;
const double DEFAULT_DENOISE = 0.75;
const double DEFAULT_CLIP_SKIP = 0;
const double DEFAULT_RP_UPSCALE = 1;
const double DEFAULT_N_STEPS = 20;
const double DEFAULT_BATCH_COUNT = 1;
