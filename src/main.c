#include "efi.h"
#include "efilib.h"

#define EFI_TRY status =
#define EFI_UNW if (EFI_ERROR(status)) return status // EFI "un-wrap"
#define PRINTLN(x) EFI_TRY ST->ConOut->OutputString(ST->ConOut, L"" x "\r\n"); EFI_UNW
#define PRINT(x) EFI_TRY ST->ConOut->OutputString(ST->ConOut, x); EFI_UNW

CHAR16 *fmt_to_buf(int v, CHAR16 *ptr, int size) {
    ptr[size - 1] = L'\0';
    ptr[size - 2] = L'\0';

    for (int i = size - 2; i >= 0; i--) {
        ptr[i] = 48 + (v % 10);
        v /= 10;
    }

    return ptr;
}

EFI_STATUS wait_for_keypress() {
    EFI_STATUS status;
    EFI_INPUT_KEY Key;
    UINTN KeyEvent = 0;

    EFI_TRY ST->ConIn->Reset(ST->ConIn, FALSE); EFI_UNW;
    EFI_TRY BS->WaitForEvent(1, &ST->ConIn->WaitForKey, &KeyEvent); EFI_UNW;
    EFI_TRY ST->ConIn->ReadKeyStroke(ST->ConIn, &Key); EFI_UNW;

    return status;
}

EFI_STATUS get_volume(EFI_HANDLE image, EFI_FILE_HANDLE* fh) {
    EFI_STATUS status;
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_FILE_IO_INTERFACE *IOVolume;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_FILE_HANDLE Volume;

    EFI_TRY BS->HandleProtocol(image, &lipGuid, (void **) &loaded_image); EFI_UNW;
    EFI_TRY BS->HandleProtocol(loaded_image->DeviceHandle, &fsGuid, (VOID *) &IOVolume); EFI_UNW;
    EFI_TRY IOVolume->OpenVolume(IOVolume, &Volume); EFI_UNW;

    *fh = Volume;
    return status;
}

UINT64 file_size (EFI_FILE_HANDLE FileHandle) {
    UINT64 sz;
    EFI_FILE_INFO *FileInfo;

    FileInfo = LibFileInfo(FileHandle);
    sz = FileInfo->FileSize;
    FreePool(FileInfo);

    return sz;
}

EFI_STATUS read_file (UINT8** buffer, UINT64* size, CHAR16* file_name, EFI_FILE_HANDLE Volume) {
    EFI_STATUS status;
    EFI_FILE_HANDLE FileHandle;

    EFI_TRY Volume->Open(Volume, &FileHandle, file_name, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    if (EFI_ERROR(status)) {
        PRINTLN("Could not open file.");
        return status;
    }

    UINT64 ReadSize = file_size(FileHandle);
    UINT8 *Buffer = AllocatePool(ReadSize);

    EFI_TRY FileHandle->Read(FileHandle, &ReadSize, Buffer);
    if (EFI_ERROR(status)) {
        PRINTLN("Could not read file.");
        return status;
    }
    EFI_TRY FileHandle->Close(FileHandle); EFI_UNW;

    *buffer = Buffer;
    *size = ReadSize;
    return status;
}

EFI_STATUS configure_gop(EFI_GRAPHICS_OUTPUT_PROTOCOL **gop_struct) {
    EFI_STATUS status;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN SizeOfInfo, numModes, nativeMode;

    EFI_TRY BS->LocateProtocol(&gopGuid, NULL, (void **) &gop);
    if (EFI_ERROR(status)) {
        PRINTLN("Could not locate GOP.");
        return status;
    }
    *gop_struct = gop;

    EFI_TRY gop->QueryMode(gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &SizeOfInfo, &info);
    if (status == EFI_NOT_STARTED) EFI_TRY gop->SetMode(gop, 0);
    if (EFI_ERROR(status)) {
        PRINTLN("Unable to get native GOP mode.");
        return status;
    } else {
        nativeMode = gop->Mode->Mode;
        numModes = gop->Mode->MaxMode;
    }

    CHAR16 buf[6];
    PRINTLN("Available modes:");
    int target = -1;
    for (int i = 0; i < numModes; i++) {
        EFI_TRY gop->QueryMode(gop, i, &SizeOfInfo, &info);
        EFI_UNW;

        if (info->HorizontalResolution == 1280 && info->VerticalResolution == 720) {
            target = i;
        }

        PRINT(L"  ");
        PRINT(fmt_to_buf(i, buf, 6));
        PRINT(L": ");
        PRINT(fmt_to_buf(info->HorizontalResolution, buf, 6));
        PRINT(L"x");
        PRINT(fmt_to_buf(info->VerticalResolution, buf, 6));
        PRINT(L"; FMT=");
        PRINT(fmt_to_buf(info->PixelFormat, buf, 6));
        PRINT(i == nativeMode ? L" (current)" : L"");
        PRINT(L"\r\n");
    }

    if (target != -1) {
        PRINT(L"Selecting GOP mode ");
        PRINT(fmt_to_buf(target, buf, 6));
        PRINT(L"\r\n");
    } else {
        PRINTLN(L"Your system does not support the required 1280x720 mode.");
        PRINTLN(L"Press any key to exit ...");
        EFI_TRY wait_for_keypress();
        EFI_UNW;

        return EFI_UNSUPPORTED;
    }

    EFI_TRY gop->SetMode(gop, target);
    EFI_UNW;

    return status;
}

EFI_STATUS render(UINT8* data, UINT64 sz) {
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

    PRINTLN("Initializing GOP ...");
    EFI_TRY configure_gop(&gop); EFI_UNW;

    for (int x = 0; x < 1280; x++) {
        for (int y = 0; y < 720; y++) {
            *((uint32_t *) (gop->Mode->FrameBufferBase + 4 * gop->Mode->Info->PixelsPerScanLine * y + 4 * x)) = 0x000000FF;
        }
    }

    return status;
}

/* TODO:
 * - refactor print to use gnu-efi print
 * - fix file IO
 * - code cleanup
 * - refactor into files
 */

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    ST = SystemTable;
    BS = ST->BootServices;

    EFI_FILE_HANDLE Volume;
    UINT8* data;
    UINT64 sz;
    EFI_TRY get_volume(ImageHandle, &Volume); EFI_UNW;
    EFI_TRY read_file(&data, &sz, L"", Volume); EFI_UNW;

    EFI_TRY ST->ConOut->ClearScreen(ST->ConOut); EFI_UNW;
    PRINTLN("UEFI/RR-m9 System 1.0");
    PRINTLN("Ulrich Barnstedt 2023");
    PRINTLN("");
    PRINTLN("Press any key to continue ...");

    EFI_TRY wait_for_keypress(); EFI_UNW;
    PRINTLN("");

    EFI_TRY render(data, sz); EFI_UNW;
    FreePool(data);

    PRINTLN("Press any key to exit ...");
    EFI_TRY wait_for_keypress(); EFI_UNW;
    return status;
}
