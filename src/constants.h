#ifndef CONSTANTS_H
#define CONSTANTS_H

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
extern const int DEFAULT_N_STEPS;
extern const int DEFAULT_SIZE;
extern const int DEFAULT_BATCH_SIZE;
extern const int DEFAULT_OPT_VRAM;
extern const int LONGLONG_STR_SIZE;
extern const long long int DEFAULT_SEED;
extern const double DEFAULT_CFG;
extern const double DEFAULT_DENOISE;
extern const double DEFAULT_CLIP_SKIP;
extern const double DEFAULT_RP_UPSCALE;

#endif  // CONSTANTS_H
