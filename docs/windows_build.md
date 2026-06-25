## Build (Windows)

Download and install `MSYS2`, You can get it [here](https://www.msys2.org/).

Launch `MSYS2 MINGW64` and install the build tools and dependencies:
```
pacman -S base-devel cmake git mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-gtk4 mingw-w64-x86_64-libpng mingw-w64-x86_64-ninja mingw-w64-x86_64-toolchain ninja
```
Clone this repository and create the build folder:
```
git clone https://github.com/Luiz-Alcantara/Neural-Pixel.git && cd Neural-Pixel && mkdir build && cd build
```
Then build the app using:
```
cmake .. -G "MSYS Makefiles" && make
```
The compiled binary will be in the path: `...\Neural-Pixel\build\bin\neural_pixel.exe`
