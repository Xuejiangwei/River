#pragma once

#include "RiverHead.h"
#include <stdio.h>

FILE* W_OpenFile(const char* path, const char* mode);

uint64 FileGetSize(FILE* f);