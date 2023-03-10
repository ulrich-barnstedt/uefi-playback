#include "helpers.h"

EFI_STATUS get_keypress (CHAR16* ch) {
    EFI_STATUS status;
    EFI_INPUT_KEY Key;
    UINTN KeyEvent = 0;

    TRY ST->ConIn->Reset(ST->ConIn, FALSE); UNW;
    TRY BS->WaitForEvent(1, &ST->ConIn->WaitForKey, &KeyEvent); UNW;
    TRY ST->ConIn->ReadKeyStroke(ST->ConIn, &Key); UNW;
    if (ch != NULL) *ch = Key.UnicodeChar;

    return status;
}

EFI_STATUS wait_for_keypress () {
    return get_keypress(NULL);
}

CHAR16* fmt_num (int v, CHAR16 *ptr, int size) {
    ptr[size - 1] = L'\0';
    ptr[size - 2] = L'\0';

    int i = size - 2;
    for (; i >= 0 && (v > 0 || i == size - 2); i--) {
        ptr[i] = 48 + (v % 10);
        v /= 10;
    }

    return ptr + i + 1;
}

void* malloc (UINTN size) {
    EFI_STATUS status;

    void* buf;
    TRY BS->AllocatePool(PoolAllocationType, size, &buf);
    if (EFI_ERROR(status)) {
        EPRINTLN("Malloc failed.");
        return NULL;
    }

    return buf;
}

void free (void *ptr) {
    EFI_STATUS status;

    TRY BS->FreePool(ptr);
    if (EFI_ERROR(status)) {
        EPRINTLN("Free failed.");
    }
}

// see gnu-efi GrowBuffer
BOOLEAN expand_buffer (EFI_STATUS* status, void** buffer, UINTN buffer_size) {
    BOOLEAN try_again;

    if (!*buffer && buffer_size) {
        *status = EFI_BUFFER_TOO_SMALL;
    }

    try_again = FALSE;
    if (*status == EFI_BUFFER_TOO_SMALL) {
        if (*buffer) {
            free(*buffer);
        }

        *buffer = malloc(buffer_size);

        if (*buffer) {
            try_again = TRUE;
        } else {
            *status = EFI_OUT_OF_RESOURCES;
        }
    }

    if (!try_again && EFI_ERROR(*status) && *buffer) {
        free(*buffer);
        *buffer = NULL;
    }

    return try_again;
}
