# UEFI-Playback

_A small kernel written in C which can play videos via UEFI's GOP interface._  

This project makes use of the `Graphics Output Protocol` supported by UEFI 2.0+, by loading raw frames from disk and rendering them.
Headers for the UEFI "API" are taken from [`gnu-efi`](https://sourceforge.net/projects/gnu-efi/) (and only headers, other functions provided by `gnu-efi` are not used).

### Dependencies and limitations

This project requires the following dependencies to run:`binutils-mingw-w64 gcc-mingw-w64 mtools make ffmpeg`  
Further dependencies are needed to run it in a VM: `qemu ovmf`  
Known limitations:
 - Due to frames being stored uncompressed and being loaded all at once, large video are not recommended and will have long loading times
 - Due to inefficiencies in copying from RAM to the framebuffer, only low framerates can be achieved and tearing will occur
 - Whilst using the headers of `gnu-efi`, the toolchain is *NOT* set up in a way that the `efi` library itself from `gnu-efi` can be used.

### Run

You can try it for yourself:
1. Put a file named `video.mp4` in `./in`
2. Run `make`
3. Burn the resulting `./out/fat.img` onto a USB-stick and boot off it

You can also run it in a VM:
1. Put a file named `video.mp4` in `./in`
2. Run `make QEMU`

### Configuration

Some parameters such as target resolution, frame rate, folder names, ... can be adjusted in the `Makefile`.
However, some of these parameters also have to be changed in the source code (as noted in the `Makefile`).