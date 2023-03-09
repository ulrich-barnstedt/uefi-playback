#include <efi.h>
#include <efilib.h>

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

    EFI_TRY ST->ConIn->Reset(ST->ConIn, FALSE);
    EFI_UNW;
    BS->WaitForEvent(1, &ST->ConIn->WaitForKey, &KeyEvent);
    ST->ConIn->ReadKeyStroke(ST->ConIn, &Key);

    // PRINT(L"KEY: ");
    // CHAR16 buf[6];
    // PRINT(fmt_to_buf(Key.ScanCode, buf, 6));
    // PRINT(L"\r\n");

    return status;
}

EFI_STATUS configure_gop (EFI_GRAPHICS_OUTPUT_PROTOCOL **gop_struct) {
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

EFI_STATUS render() {
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

    PRINTLN("Initializing GOP ...");
    EFI_TRY configure_gop(&gop);
    EFI_UNW;

    for (int x = 0; x < 1280; x++) {
        for (int y = 0; y < 720; y++) {
            *((uint32_t*)(gop->Mode->FrameBufferBase + 4 * gop->Mode->Info->PixelsPerScanLine * y + 4 * x)) = 0x000000FF;
        }
    }

    return status;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    ST = SystemTable;
    BS = ST->BootServices;

    EFI_TRY ST->ConOut->ClearScreen(ST->ConOut);
    EFI_UNW;
    PRINTLN("UEFI/RR-m9 System 1.0");
    PRINTLN("Ulrich Barnstedt 2023");
    PRINTLN("");
    PRINTLN("Press any key to continue ...");

    EFI_TRY wait_for_keypress();
    EFI_UNW;
    PRINTLN("");

    EFI_TRY render();
    EFI_UNW;

    PRINTLN("Press any key to exit ...");
    EFI_TRY wait_for_keypress();
    EFI_UNW;
    return status;
}
