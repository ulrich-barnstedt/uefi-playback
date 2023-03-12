#pragma once
#include "helpers.h"

#define TARGET_Y 720
#define TARGET_X 1280

// delay: ms * 1000
#define RENDER_DELAY 100000

EFI_STATUS render (UINT32 *data, UINT64 sz);
