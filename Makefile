SRC_DIR := ./src
OBJ_DIR := ./obj
OUT_DIR := ./out
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))

CC := x86_64-w64-mingw32-gcc
CFLAGS := -ffreestanding -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/efi/protocol
LDFLAGS := -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main

all: $(OUT_DIR)/fat.img

# -------- Directories / C sources

$(OBJ_DIR):
	mkdir -p $@

$(OUT_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OUT_DIR)/BOOTX64.EFI: $(OBJ_FILES) | $(OUT_DIR)
	$(CC) $(LDFLAGS) -o $@ $^

# -------- FAT-12 UEFI Image

$(OUT_DIR)/fat.img: $(OUT_DIR)/BOOTX64.EFI
	dd if=/dev/zero of=$(OUT_DIR)/fat.img bs=1k count=1440
	mformat -i $(OUT_DIR)/fat.img -f 1440 ::
	mmd -i $(OUT_DIR)/fat.img ::/EFI
	mmd -i $(OUT_DIR)/fat.img ::/EFI/BOOT
	mcopy -i $(OUT_DIR)/fat.img $(OUT_DIR)/BOOTX64.EFI ::/EFI/BOOT

# -------- QEMU

$(OUT_DIR)/cdimage.iso: $(OUT_DIR)/fat.img
	mkdir -p $(OUT_DIR)/iso
	cp $(OUT_DIR)/fat.img $(OUT_DIR)/iso
	xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o $(OUT_DIR)/cdimage.iso $(OUT_DIR)/iso

$(OUT_DIR)/bios.bin:
	cp /usr/share/ovmf/OVMF.fd $(OUT_DIR)/bios.bin

QEMU: $(OUT_DIR)/cdimage.iso $(OUT_DIR)/bios.bin
	qemu-system-x86_64 -drive file=${OUT_DIR}/bios.bin,format=raw,if=pflash -net none -cdrom $(OUT_DIR)/cdimage.iso

# -------- Clean

clean-content:
	rm $(OBJ_DIR)/*
	rm $(OUT_DIR)/*

clean:
	rm -r $(OBJ_DIR)
	rm -r $(OUT_DIR)

