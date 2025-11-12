// constants.c
#include <stdio.h>
#include "constants.h"

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


const char* LIST_RESOLUTIONS_STR[] = {"64", "128", "192", "256", "320", "384", "448", "512", "576", "640", "704", "768", "832", "896", "960", "1024", "1088", "1152", "1216", "1280", NULL};
const size_t LIST_RESOLUTIONS_STR_COUNT = sizeof(LIST_RESOLUTIONS_STR) / sizeof(LIST_RESOLUTIONS_STR[0]);

const char* LIST_STEPS_STR[] = {"1", "2", "4", "8", "12", "16", "20", "24", "30", "36", "42", "50", "60", NULL};
const size_t LIST_STEPS_STR_COUNT = sizeof(LIST_STEPS_STR) / sizeof(LIST_STEPS_STR[0]);

const char* LIST_SAMPLES[] = {"ddim_trailing", "dpm++2m", "dpm++2mv2", "dpm++2s_a", "dpm2", "euler", "euler_a", "heun", "ipndm", "ipndm_v", "lcm", "tcd", NULL};
const size_t LIST_SAMPLES_COUNT = sizeof(LIST_SAMPLES) / sizeof(LIST_SAMPLES[0]);  

const char* LIST_SCHEDULES[] = {"ays", "discrete", "exponential", "gits", "karras", "sgm_uniform", "simple", "smoothstep", NULL};
const size_t LIST_SCHEDULES_COUNT = sizeof(LIST_SCHEDULES) / sizeof(LIST_SCHEDULES[0]); 


const char* POSITIVE_PROMPT = "A colossal ancient tree towering over a vast landscape, its massive branches stretching into the sky, glowing blue particles gracefully drifting down from its leaves, bathed in ethereal light. The scene is filled with vibrant colors, lush greens, radiant blues, and golden hues from a dramatic sunset. An epic cinematic background with distant mountains, swirling clouds, and mystical atmosphere, ultra-detailed, 8K resolution, fantasy concept art, volumetric lighting, masterpiece, epic scale";
const char* NEGATIVE_PROMPT = "blurry, grainy, low resolution, cartoon, low detail, distorted, text, watermark";
const char* OPTIONAL_ITEMS = "None";
const char* DEFAULT_IMG_PATH = "./resources/example.png";
const char* EMPTY_IMG_PATH = "./resources/empty.png";
const int DEFAULT_MODELS = 0;
const int DEFAULT_SAMPLER = 6;
const int DEFAULT_SCHEDULER = 4;
const int DEFAULT_N_STEPS = 7;
const int DEFAULT_SIZE = 7;
const int DEFAULT_BATCH_SIZE = 0;
const int DEFAULT_OPT_VRAM = 0;
const int LONGLONG_STR_SIZE = 21;
const long long int DEFAULT_SEED = -1;
const double DEFAULT_CFG = 6;
const double DEFAULT_DENOISE = 0.75;
const double DEFAULT_CLIP_SKIP = 0;
const double DEFAULT_RP_UPSCALE = 1;
