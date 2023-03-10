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
    for (; i >= 0 && v > 0; i--) {
        ptr[i] = 48 + (v % 10);
        v /= 10;
    }

    return ptr + i + 1;
}