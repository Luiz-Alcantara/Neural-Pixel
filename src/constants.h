#ifndef CONSTANTS_H
#define CONSTANTS_H

extern const char* APP_NAME_VERSION;
extern const char* APP_AUTHOR;
extern const char* APP_DESC;
extern const char* APP_DESC2;
extern const char* NP_GITHUB;
extern const char* SDCPP_GITHUB;

extern const char* CACHE_PATH;
extern const char* MODELS_PATH;
extern const char* CHECKPOINTS_PATH;
extern const char* CLIPS_PATH;
extern const char* CONTROLNET_PATH;
extern const char* EMBEDDINGS_PATH;
extern const char* LORAS_PATH;
extern const char* TEXT_ENCODERS_PATH;
extern const char* UNET_PATH;
extern const char* UPSCALES_PATH;
extern const char* VAES_PATH;
extern const char* OUTPUTS_PATH;

extern const char* LIST_RESOLUTIONS_STR[];
extern const size_t LIST_RESOLUTIONS_STR_COUNT;

extern const char* LIST_STEPS_STR[];
extern const size_t LIST_STEPS_STR_COUNT;

extern const char* LIST_SAMPLES[];
extern const size_t LIST_SAMPLES_COUNT;

extern const char* LIST_SCHEDULES[];
extern const size_t LIST_SCHEDULES_COUNT;

extern const char* POSITIVE_PROMPT;
extern const char* NEGATIVE_PROMPT;
extern const char* OPTIONAL_ITEMS;
extern const char* DEFAULT_IMG_PATH;
extern const char* EMPTY_IMG_PATH;
extern const int DEFAULT_MODELS;
extern const int DEFAULT_SAMPLER;
extern const int DEFAULT_SCHEDULER;
extern const int DEFAULT_SIZE;
extern const int ENABLED_OPT;
extern const int DISABLED_OPT;
extern const int LONGLONG_STR_SIZE;
extern const int ZERO_SPACING;
extern const int SMALL_SPACING;
extern const int MEDIUM_SPACING;
extern const int LARGE_SPACING;
extern const int HUGE_SPACING;
extern const int LABEL_ALIGNMENT;
extern const int DD_STRING_LEN;
extern const long long int DEFAULT_SEED;
extern const double DEFAULT_CFG;
extern const double DEFAULT_CNET_STRENGTH;
extern const double DEFAULT_DENOISE;
extern const double DEFAULT_CLIP_SKIP;
extern const double DEFAULT_RP_UPSCALE;
extern const double DEFAULT_N_STEPS;
extern const double DEFAULT_BATCH_COUNT;

#endif  // CONSTANTS_H
