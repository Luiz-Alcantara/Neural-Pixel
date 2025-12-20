<div align="center">

# Neural-Pixel
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/Luiz-Alcantara/Neural-Pixel)

**A simple GUI wrapper for stable-diffusion.cpp written using C and GTK 4.**
![Screenshot1](https://github.com/Luiz-Alcantara/Neural-Pixel/blob/main/screenshots/img2.png?raw=true)
</div>

With Neural Pixel, you can use Stable Diffusion on practically any GPU that supports Vulkan and has at least 3GB of VRAM. This is a simple way to generate your images without having to deal with CUDA/ROCm installations or hundreds of Python dependencies.


## Features
- Supported models:
    - SD 1.4, SD 1.5, SD 1.5 LCM, SD 1.5 Turbo, SD 1.5 Hyper, SD 2.0 and SD 2.1
        - Some SD 1.X and SDXL Distilled models work too, see [this](https://github.com/leejet/stable-diffusion.cpp/blob/master/docs/distilled_sd.md).
    - SDXL 1.0, SDXL Lightning, SDXL Hyper, Pony and Illustrious
        - If you get a black image when using SDXL based models, try using the VAE: [SDXL VAE FP16 Fix](https://huggingface.co/madebyollin/sdxl-vae-fp16-fix/blob/main/sdxl_vae.safetensors).
    - SD 3.0 and SD 3.5
    - Flux-dev and Flux-schnell
        - Flux models needs to be converted to `GGUF` to work. You can either download a pre-converted model ([FLUX.1-dev](https://huggingface.co/leejet/FLUX.1-dev-gguf/tree/main) or [FLUX.1-schnell](https://huggingface.co/leejet/FLUX.1-schnell-gguf/tree/main))
        - Or convert your own:
        ```
		./sd -M convert -m path_to_your_model.safetensors -o your_model_converted.q8_0.gguf -v --type q8_0
		```
        - You'll also need: [Flux VAE](https://huggingface.co/black-forest-labs/FLUX.1-dev/blob/main/ae.safetensors), [clip_l](https://huggingface.co/comfyanonymous/flux_text_encoders/blob/main/clip_l.safetensors), and [t5xxl](https://huggingface.co/comfyanonymous/flux_text_encoders/blob/main/t5xxl_fp16.safetensors).
    - Chroma and Chroma1-Radiance
    - Qwen Image
- Supported add-ons:
    - LoRA and LCM-LoRA
    - Embeddings
    - Control Net for SD 1.5
    - TAESD (Faster latent decoding/Worse quality)
    - ESRGAN Upscaler (Only [RealESRGAN_x4plus_anime_6B.pth](https://github.com/xinntao/Real-ESRGAN/releases/download/v0.2.2.4/RealESRGAN_x4plus_anime_6B.pth) works for now.)

- Supported weight formats:
    - Pytorch checkpoint (.ckpt or .pth)
    - Safetensors
    - GGUF

- Supported platforms:
    - Linux
    - Windows

- Supported backends:
    - The files available on the releases page support only CPU and Vulkan.
    - But you can compile sd.cpp for your own backend by following [this](https://github.com/leejet/stable-diffusion.cpp/blob/master/docs/build.md), then simply replace the 'sd' binary in the Neural-Pixel folder with your newly compiled binary, and the app will work as expected.

## Notes

- No video support at the moment.
- You can change the application's theme color in the `resources/styles.css` file.

## Linux Requirements

- Linux Distro with kernel >= 5.14 (Tested on RHEL 9, Fedora 42, and Arch Linux).

- GTK >= 4.12, Vulkan(If using GPU) and libpng installed.
- Install deps on Arch (Change vulkan packages according to your GPU, this is for AMD GPUs):
```
sudo pacman -S gtk4 libpng zlib vulkan-icd-loader vulkan-radeon vulkan-tools
```

- Install deps on Debian/Ubuntu/Mint (Change vulkan packages according to your GPU, this is for AMD GPUs):
```
sudo apt install libgtk-4-1 libpng16-16 zlib1g vulkan-tools mesa-vulkan-drivers
```

- Install deps on Fedora/RHEL (Change vulkan packages according to your GPU, this is for AMD GPUs):
```
sudo dnf install gtk4 libpng zlib vulkan-tools mesa-vulkan-drivers
```

- Install deps on OpenSUSE (Change vulkan packages according to your GPU, this is for AMD GPUs):
```
sudo zypper install libgtk-4-1 libpng16-16 libz1 vulkan-tools libvulkan_radeon libvulkan1
```

- A GPU or iGPU with at least 3GB of VRAM for Vulkan.

## Running on Linux

Download the Linux bundle in the releases tab, extract it and run the "run_neural_pixel" file.
If you want to see errors and details, start the application from a terminal and enable the "Terminal Verbose" option in "Extra Opts."

## Windows Requirements

- Microsoft Visual C++ Redistributable latest: [vc_redist](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170).
- A GPU or iGPU with at least 3GB of VRAM for Vulkan.

## Running on Windows

Download the Windows bundle in the releases tab, extract it and run the "neural_pixel.bat" file.
You can directly run the "neural pixel" binary, but the variable that defines the dark theme will not be applied, so the app may look weird.

## Recommended checkpoints

- For realistic style: [Photon](https://huggingface.co/sam749/Photon-v1/blob/main/photon_v1.safetensors).
- For anime style: [SoteMix](https://civitai.com/models/72182/sotemix).

## Build

You'll need **GTK 4** and the **libpng development libraries** installed.
Then, clone this repository using:
```
git clone https://github.com/Luiz-Alcantara/Neural-Pixel.git
```
Next, navigate into the cloned directory and run:
```
mkdir build && cd build && cmake .. && make
```

To build on Windows Use MSYS2.

To build sd.cpp follow the instructions on its github page: [Stable-diffusion.cpp](https://github.com/leejet/stable-diffusion.cpp).

## Credits

- This project is a GUI to [stable-diffusion.cpp](https://github.com/leejet/stable-diffusion.cpp) by [@leejet](https://github.com/leejet).

## Donations

- PayPal: [`Donate`](https://www.paypal.com/donate/?hosted_button_id=G29L2QHNWDJHJ)

- Bitcoin
```
bc1qhxxgy52s2ps9j2gyzfxtykccrrpkzpu9uvnhhe
```
- Ethereum
```
0x5da0a849D04085C7F1943871F469cb77394aa84b
```
- Litecoin
```
ltc1q8fu7j3zyckl0w4e6m2q85xc69ywvtpnjzdjhvq
```
- BAT (Brave's Basic Attention Tokens)
```
0x8c6bB60AAC4565582471B8BA812058aeafc05cd7
```
