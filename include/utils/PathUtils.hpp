#include <cstring>

bool ends_with_ext(const char* path, const char* ext) {
    size_t path_len = std::strlen(path);
    size_t ext_len  = std::strlen(ext);

    if (path_len < ext_len)
        return false;

    return std::strcmp(path + path_len - ext_len, ext) == 0;
}
