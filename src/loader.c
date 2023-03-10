#include "loader.h"

EFI_STATUS get_volume(EFI_HANDLE image, EFI_FILE_HANDLE* fh) {
    EFI_STATUS status;
    EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
    EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_LOADED_IMAGE *loaded_image = NULL;
    EFI_FILE_IO_INTERFACE *IOVolume;
    EFI_FILE_HANDLE Volume;

    TRY BS->HandleProtocol(image, &lipGuid, (void **) &loaded_image); UNW;
    TRY BS->HandleProtocol(loaded_image->DeviceHandle, &fsGuid, (VOID *) &IOVolume); UNW;
    TRY IOVolume->OpenVolume(IOVolume, &Volume); UNW;

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

    TRY Volume->Open(Volume, &FileHandle, file_name, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    if (EFI_ERROR(status)) {
        Print(L"Could not open file.");
        return status;
    }

    UINT64 ReadSize = file_size(FileHandle);
    UINT8 *Buffer = AllocatePool(ReadSize);

    TRY FileHandle->Read(FileHandle, &ReadSize, Buffer);
    if (EFI_ERROR(status)) {
        Print(L"Could not read file.");
        return status;
    }
    TRY FileHandle->Close(FileHandle); UNW;

    *buffer = Buffer;
    *size = ReadSize;
    return status;
}

EFI_STATUS load_data (UINT8** buffer, UINT64* size, CHAR16* file_name, EFI_HANDLE image) {

}
