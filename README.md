<div align="center">

# Neural-Pixel
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/Luiz-Alcantara/Neural-Pixel)
[![Download](https://img.shields.io/github/downloads/Luiz-Alcantara/Neural-Pixel/total.svg)](https://github.com/Luiz-Alcantara/Neural-Pixel/releases/latest)

**A simple GUI wrapper for stable-diffusion.cpp written using C and GTK 4.**
![Screenshot1](https://github.com/Luiz-Alcantara/Neural-Pixel/blob/main/screenshots/img1.png?raw=true)
</div>

Neural Pixel is a fast, Vulkan-powered image generation tool that runs on almost any GPU from 2014+ (requires at least 2GB VRAM for SD 1.5 or 3GB for SDXL). Skip the CUDA/ROCm headache and Python dependency hell, Neural Pixel is simple, portable, and high-performing!

## Compatibility

- Neural Pixel supports leading image generation models such as SDXL and FLUX, plus a broad range of community models, extensions, and runtimes.
- By default, the release ZIP packages include support for Vulkan and CPU inference only.
- See docs/compatibility.md for the full list of supported models, features, formats, platforms, and backends.

## Notes

- No video support at the moment.
- You can change the application's theme color in the `resources/styles.css` file.

## Linux Requirements (usually preinstalled on most systems)

- Linux Distro with kernel >= 5.14 (Tested on RHEL 9, Fedora 42, and Arch Linux).

- Required:
    - GTK >= 4.12
    - libpng
    - zlib

- Optional (Vulkan backend):
    - Vulkan (driver, loader and tools)

- A GPU or iGPU with at least 2GB of VRAM for Vulkan.

## Running on Linux

Download the Linux bundle:
[![Linux](https://img.shields.io/badge/Linux-v0.7.2-orange?style=flat-square&logo=linux)](https://github.com/Luiz-Alcantara/Neural-Pixel/releases/download/v0.7.2/NeuralPixel-Linux_v0.7.2.zip)

Then extract it and run the "run_neural_pixel" file.
If you want to see errors and details, start the application from a terminal and enable the "Terminal Verbose" option in "Extra Opts."

## Windows Requirements

- Microsoft Visual C++ Redistributable latest: [vc_redist](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170).
- A GPU or iGPU with at least 2GB of VRAM for Vulkan.

## Running on Windows

Download the Windows bundle:
[![Windows](https://img.shields.io/badge/Windows-v0.7.2-blue?style=flat-square&logo=windows)](https://github.com/Luiz-Alcantara/Neural-Pixel/releases/download/v0.7.2/NeuralPixel-Windows_v0.7.2.zip)

Then extract it and run the "neural_pixel.bat" file.
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

- This project is a GUI for [stable-diffusion.cpp](https://github.com/leejet/stable-diffusion.cpp).

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
