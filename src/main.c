#include <efi.h>
#include <efilib.h>
#include "helpers.h"
#include "loader.h"
#include "render.h"

/* TODO:
 * test which gnu-efi calls work, and refactor all others to use own code (direct efi functions)
 */

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    ST = SystemTable;
    BS = ST->BootServices;
    RT = ST->RuntimeServices;

    TRY ST->ConOut->ClearScreen(ST->ConOut); UNW;
    PRINTLN("UEFI/RR-m9 System 1.0");
    PRINTLN("Ulrich Barnstedt 2023");
    PRINTLN("");
    PRINTLN("Press any key to continue ...");
    TRY wait_for_keypress(); UNW;
    PRINTLN("");

    UINT8* data;
    UINT64 sz;
    TRY load_data(&data, &sz, L"", ImageHandle); UNW;

    TRY render(data, sz); UNW;
    //TODO: free data pool

    PRINTLN("Press any key to exit ...");
    TRY wait_for_keypress(); UNW;
    return status;
}
