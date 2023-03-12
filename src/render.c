#include "render.h"

EFI_STATUS configure_gop(EFI_GRAPHICS_OUTPUT_PROTOCOL **gop_struct) {
    EFI_STATUS status;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    UINTN SizeOfInfo, numModes, nativeMode;

    TRY BS->LocateProtocol(&gopGuid, NULL, (void **) &gop);
    if (EFI_ERROR(status)) {
        EPRINTLN("Could not locate GOP.");
        return status;
    }
    *gop_struct = gop;

    TRY gop->QueryMode(gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &SizeOfInfo, &info);
    if (status == EFI_NOT_STARTED) TRY gop->SetMode(gop, 0);
    if (EFI_ERROR(status)) {
        EPRINTLN("Unable to get native GOP mode.");
        return status;
    } else {
        nativeMode = gop->Mode->Mode;
        numModes = gop->Mode->MaxMode;
    }

    CHAR16 buf[6];
    PRINTLN("Available modes:");
    int target = -1;
    for (int i = 0; i < numModes; i++) {
        TRY gop->QueryMode(gop, i, &SizeOfInfo, &info); UNW;

        if (info->HorizontalResolution >= TARGET_X && info->VerticalResolution >= TARGET_Y) {
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
        EPRINTLN("Your system does not support the minimum targeted resolution.");
        return EFI_UNSUPPORTED;
    }

    TRY gop->SetMode(gop, target); UNW;

    return status;
}

EFI_STATUS render (UINT32* data, UINT64 sz) {
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    UINT64 frame_size = sizeof(UINT32) * TARGET_Y * TARGET_X;
    UINT64 frame_count = sz / frame_size;
    UINT32* at = data;

    PRINTLN("Initializing GOP ...");
    TRY configure_gop(&gop); UNW;

    PRINTLN("Rendering ...");

    for (int f = 0; f < frame_count; f++) {
        for (int y = 0; y < TARGET_Y; y++) {
            for (int x = 0; x < TARGET_X; x++) {
                *((uint32_t *) (gop->Mode->FrameBufferBase + 4 * gop->Mode->Info->PixelsPerScanLine * y + 4 * x)) = *at;
                at += 1;
            }
        }

        TRY BS->Stall(RENDER_DELAY); UNW;
    }

    return status;
}
