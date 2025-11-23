#pragma once
#include <cstdarg>

void fopen_s(FILE** file, const char* name, const char* mode) {
     *file = std::fopen(name, mode);
}

int fscanf_s(FILE* stream, const char* format, ...) {
     va_list args;
     va_start(args, format);
     int ret = std::vfscanf(stream, format, args);
     va_end(args);
     return ret;
}