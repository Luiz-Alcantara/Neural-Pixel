## Supported Models
| Supported models | Supported variations | Notes |
|--------|--------|-------|
| Anima  | Base | The turbo LoRA also works |
| SD 1.4 | Only base tested | None |
| SD 1.5 | Base, LCM, Turbo & Hyper | Some SD 1.5 Distilled models work too, check [this](https://github.com/leejet/stable-diffusion.cpp/blob/master/docs/distilled_sd.md). |
| SD 2.X | Only base tested | None |
|  SDXL  | Base, Lightning, Hyper, Pony, Illustrious & NoobAI | Some SDXL Distilled models work too, check [this](https://github.com/leejet/stable-diffusion.cpp/blob/master/docs/distilled_sd.md). |
| SD 3.X | Only base tested | None |
| Flux.1 | Dev, Schnell and Kontext | Flux models needs to be converted to `GGUF` to work, check [this](https://github.com/leejet/stable-diffusion.cpp/blob/master/docs/flux.md). You'll also need: [Flux VAE](https://huggingface.co/black-forest-labs/FLUX.1-dev/blob/main/ae.safetensors), [clip_l](https://huggingface.co/comfyanonymous/flux_text_encoders/blob/main/clip_l.safetensors), and [t5xxl](https://huggingface.co/comfyanonymous/flux_text_encoders/blob/main/t5xxl_fp16.safetensors). |
| Chroma1 | Base, Radiance | None |
| Qwen | Only base tested | None |

## Supported Add-ons
| Supported add-ons | Notes |
|--------|-------|
| LoRA and LCM-LoRA | None |
| Embeddings | None |
| Control Net | SD 1.5 Only |
| TAESD | Faster latent decoding/Worse quality |
| ESRGAN Upscaler | None |

## Supported Features
| Supported features | Notes |
|--------|-------|
| txt2img | Generate images from text prompts |
| img2img / Flux Kontext | Transform existing images using prompts |
| Mask-based Inpainting | Modify specific areas of an image using a mask |
| Hires Fix | Upscales and refines images with added detail |

## Supported Weight Formats
| Supported weight formats |
|--------|
| Pytorch checkpoint (.ckpt or .pth) |
| Safetensors |
| GGUF |

## Supported Platforms
| Supported platforms |
|--------|
| Linux |
| Windows |

## Supported Backends
| Supported backends | Notes |
|--------|-------|
| CPU and Vulkan | Included with the release packages (cpu mode must be configured in the backend manager). |
| CUDA, ROCm... | To compile sd.cpp for other backends, follow the instructions [here](https://github.com/leejet/stable-diffusion.cpp/blob/master/docs/build.md), then simply replace the "sd" binary in the Neural-Pixel folder with your newly compiled one (rename if needed). |
