#pragma once
#include <efi.h>
#include <efilib.h>

#define TRY status =
#define UNW if (EFI_ERROR(status)) return status // EFI "un-wrap"
#define PRINTLN(x) TRY ST->ConOut->OutputString(ST->ConOut, L"" x "\r\n"); UNW
#define PRINT(x) TRY ST->ConOut->OutputString(ST->ConOut, x); UNW
#define EPRINTLN(x) ST->ConOut->OutputString(ST->ConOut, L"" x "\r\n"); wait_for_keypress()
#define EPRINT(x) ST->ConOut->OutputString(ST->ConOut, x)

EFI_STATUS wait_for_keypress ();
EFI_STATUS get_keypress (CHAR16* ch);
CHAR16* fmt_num (int v, CHAR16 *ptr, int size);
void* malloc (UINTN size);
void free (void *ptr);
BOOLEAN expand_buffer (EFI_STATUS* status, void** buffer, UINTN buffer_size);
