# directories & files
SRC_DIR := ./src
OBJ_DIR := ./obj
OUT_DIR := ./out
IN_DIR := ./in
GNU_EFI_DIR := ./gnu-efi-code
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# data & iso
IMG_SIZE := 128000
INPUT_FILE := video.mp4
FRAME_COUNT := 25
FRAME_WIDTH := 1280
FRAME_HEIGHT := 720

# compilation
CROSS_COMPILE := x86_64-w64-mingw32-
CC := $(CROSS_COMPILE)gcc
CFLAGS := -ffreestanding -I$(GNU_EFI_DIR)/inc -I$(GNU_EFI_DIR)/inc/x86_64 -I$(GNU_EFI_DIR)/inc/protocol
LDFLAGS := -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main

all: $(OUT_DIR)/fat.img

# -------- GNU-EFI library

$(GNU_EFI_DIR)/lib/libefi.a:
	CROSS_COMPILE=$(CROSS_COMPILE) make -C $(GNU_EFI_DIR)/lib

# -------- ffmpeg pre-processing

$(OUT_DIR)/data: $(IN_DIR)/$(INPUT_FILE)
	ffmpeg -i $(IN_DIR)/$(INPUT_FILE) -frames $(FRAME_COUNT) -r 5 -pix_fmt bgra -s $(FRAME_WIDTH)x$(FRAME_HEIGHT) -f rawvideo $(OUT_DIR)/data

# -------- Directories / C sources

$(OBJ_DIR):
	mkdir -p $@

$(OUT_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT_DIR)/BOOTX64.EFI: $(OBJ_FILES) $(GNU_EFI_DIR)/lib/libefi.a | $(OUT_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# -------- FAT-12 UEFI Image

$(OUT_DIR)/fat.img: $(OUT_DIR)/BOOTX64.EFI $(OUT_DIR)/data
	dd if=/dev/zero of=$(OUT_DIR)/fat.img bs=1k count=$(IMG_SIZE)
	mformat -i $(OUT_DIR)/fat.img -F ::
	mmd -i $(OUT_DIR)/fat.img ::/EFI
	mmd -i $(OUT_DIR)/fat.img ::/EFI/BOOT
	mcopy -i $(OUT_DIR)/fat.img $(OUT_DIR)/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i $(OUT_DIR)/fat.img $(OUT_DIR)/data ::

# -------- QEMU

$(OUT_DIR)/cdimage.iso: $(OUT_DIR)/fat.img
	mkdir -p $(OUT_DIR)/iso
	cp $(OUT_DIR)/fat.img $(OUT_DIR)/iso
	xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o $(OUT_DIR)/cdimage.iso $(OUT_DIR)/iso

$(OUT_DIR)/bios.bin:
	cp /usr/share/ovmf/OVMF.fd $(OUT_DIR)/bios.bin

QEMU: $(OUT_DIR)/cdimage.iso $(OUT_DIR)/bios.bin
	qemu-system-x86_64 -drive file=${OUT_DIR}/bios.bin,format=raw,if=pflash -m 512M -net none -cdrom $(OUT_DIR)/cdimage.iso

# -------- Clean

clean-content:
	rm $(OBJ_DIR)/*
	rm $(OUT_DIR)/*

clean:
	rm -r $(OBJ_DIR)
	rm -r $(OUT_DIR)

