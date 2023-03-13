# directories & files
SRC_DIR := ./src
OBJ_DIR := ./obj
OUT_DIR := ./out
IN_DIR := ./in
GNU_EFI_DIR := ./gnu-efi-code
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

# data & iso
IMG_SIZE := 192000
INPUT_FILE := video.mp4
FRAME_COUNT := 50
FPS = 10
FRAME_WIDTH := 1280
FRAME_HEIGHT := 720
# NOTE:
# frame height and width also have to be adjusted in render.h
# FPS count should be reflected in render delay in render.h

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
	ffmpeg -i $(IN_DIR)/$(INPUT_FILE) -frames $(FRAME_COUNT) -r $(FPS) -pix_fmt bgra -s $(FRAME_WIDTH)x$(FRAME_HEIGHT) -f rawvideo $(OUT_DIR)/data

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

QEMU: $(OUT_DIR)/fat.img
	qemu-system-x86_64 -bios /usr/share/ovmf/OVMF.fd -net none -m 512m -cdrom $(OUT_DIR)/fat.img

# -------- Clean

clean-content:
	rm $(OBJ_DIR)/*
	rm $(OUT_DIR)/*

clean:
	rm -r $(OBJ_DIR)
	rm -r $(OUT_DIR)

