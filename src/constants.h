#ifndef CONSTANTS_H
#define CONSTANTS_H

extern const char* SD_FILES_PATH;
extern const char* MODELS_PATH;
extern const char* CLIPS_PATH;
extern const char* CONTROLNET_PATH;
extern const char* EMBEDDINGS_PATH;
extern const char* LORAS_PATH;
extern const char* TEXT_ENCODERS_PATH;
extern const char* UNET_PATH;
extern const char* UPSCALES_PATH;
extern const char* VAES_PATH;

extern const char* LIST_RESOLUTIONS_STR[];
extern const char* LIST_STEPS_STR[];
extern const char* LIST_SAMPLES[];
extern const char* LIST_SCHEDULES[];

extern const char* POSITIVE_PROMPT;
extern const char* NEGATIVE_PROMPT;
extern const char* OPTIONAL_ITEMS;
extern const char* DEFAULT_IMG_PATH;
extern const int DEFAULT_MODELS;
extern const int DEFAULT_SAMPLE;
extern const int DEFAULT_SCHEDULE;
extern const int DEFAULT_N_STEPS;
extern const int DEFAULT_SIZE;
extern const int DEFAULT_BATCH_SIZE;
extern const int DEFAULT_OPT_VRAM;
extern const double DEFAULT_CFG;
extern const double DEFAULT_DENOISE;
extern const double DEFAULT_SEED;
extern const double DEFAULT_RP_UPSCALE;

#endif  // CONSTANTS_H
