#include "render.h"

EFI_STATUS configure_gop(EFI_GRAPHICS_OUTPUT_PROTOCOL **gop_struct) {
    EFI_STATUS status;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN SizeOfInfo, numModes, nativeMode;

    TRY BS->LocateProtocol(&gopGuid, NULL, (void **) &gop);
    if (EFI_ERROR(status)) {
        PRINTLN("Could not locate GOP.");
        return status;
    }
    *gop_struct = gop;

    TRY gop->QueryMode(gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &SizeOfInfo, &info);
    if (status == EFI_NOT_STARTED) TRY gop->SetMode(gop, 0);
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
        TRY gop->QueryMode(gop, i, &SizeOfInfo, &info);
        UNW;

        if (info->HorizontalResolution == 1280 && info->VerticalResolution == 720) {
            target = i;
        }

        PRINT(L"  ");
        PRINT(fmt_num(i, buf, 6));
        PRINT(L": ");
        PRINT(fmt_num(info->HorizontalResolution, buf, 6));
        PRINT(L"x");
        PRINT(fmt_num(info->VerticalResolution, buf, 6));
        PRINT(L"; FMT=");
        PRINT(fmt_num(info->PixelFormat, buf, 6));
        PRINT(i == nativeMode ? L" (current)" : L"");
        PRINT(L"\r\n");
    }

    if (target != -1) {
        PRINT(L"Selecting GOP mode ");
        PRINT(fmt_num(target, buf, 6));
        PRINT(L"\r\n");
    } else {
        PRINTLN("Your system does not support the required 1280x720 mode.");
        PRINTLN("Press any key to exit ...");
        TRY wait_for_keypress();
        UNW;

        return EFI_UNSUPPORTED;
    }

    TRY gop->SetMode(gop, target);
    UNW;

    return status;
}

EFI_STATUS render(UINT8* data, UINT64 sz) {
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

    PRINTLN("Initializing GOP ...");
    TRY configure_gop(&gop); UNW;

    for (int x = 0; x < 1280; x++) {
        for (int y = 0; y < 720; y++) {
            *((uint32_t *) (gop->Mode->FrameBufferBase + 4 * gop->Mode->Info->PixelsPerScanLine * y + 4 * x)) = 0x000000FF;
        }
    }

    return status;
}
