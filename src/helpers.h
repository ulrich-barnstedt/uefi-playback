#pragma once
#include <efi.h>
#include <efilib.h>

#define TRY status =
#define UNW if (EFI_ERROR(status)) return status // EFI "un-wrap"
#define PRINTLN(x) TRY ST->ConOut->OutputString(ST->ConOut, L"" x "\r\n"); UNW
#define PRINT(x) TRY ST->ConOut->OutputString(ST->ConOut, x); UNW

EFI_STATUS wait_for_keypress ();
EFI_STATUS get_keypress (CHAR16* ch);
CHAR16* fmt_num (int v, CHAR16 *ptr, int size);
