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
