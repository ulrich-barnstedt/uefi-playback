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

EFI_STATUS file_size (EFI_FILE_HANDLE FileHandle, UINT64* sz) {
    EFI_STATUS status = EFI_SUCCESS;
    EFI_FILE_INFO *FileInfo;
    UINTN BufferSize;

    FileInfo = NULL;
    BufferSize = SIZE_OF_EFI_FILE_INFO + 200;

    while (expand_buffer(&status, (VOID **) &FileInfo, BufferSize)) {
        TRY FileHandle->GetInfo(FileHandle, &GenericFileInfo, &BufferSize, FileInfo);
    }

    *sz = FileInfo->FileSize;
    free(FileInfo);

    return status;
}

EFI_STATUS read_file (UINT32** buffer, UINT64* size, CHAR16* file_name, EFI_FILE_HANDLE Volume) {
    EFI_STATUS status;
    EFI_FILE_HANDLE FileHandle;

    TRY Volume->Open(Volume, &FileHandle, file_name, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
    if (EFI_ERROR(status)) {
        EPRINTLN("Could not open file.");
        return status;
    }

    UINT64 ReadSize;
    TRY file_size(FileHandle, &ReadSize); UNW;
    UINT32 *Buffer = malloc(ReadSize);

    CHAR16 buf[20];
    PRINT(L"Reading ");
    PRINT(fmt_num(ReadSize, buf, 20));
    PRINT(L" bytes ...\r\n");

    TRY FileHandle->Read(FileHandle, &ReadSize, Buffer);
    if (EFI_ERROR(status)) {
        EPRINTLN("Could not read file.");
        return status;
    }
    TRY FileHandle->Close(FileHandle); UNW;

    *buffer = Buffer;
    *size = ReadSize;
    return status;
}

EFI_STATUS load_data (UINT32** buffer, UINT64* size, CHAR16* file_name, EFI_HANDLE image) {
    EFI_STATUS status;
    EFI_FILE_HANDLE volume;

    PRINTLN("Getting volume handle ...");
    TRY get_volume(image, &volume); UNW;

    PRINTLN("Loading data ...");
    TRY read_file(buffer, size, file_name, volume); UNW;

    PRINTLN("Loaded data.");
    return status;
}
