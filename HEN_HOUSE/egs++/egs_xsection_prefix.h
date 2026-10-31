#ifndef EGS_XSECTION_PREFIX_H_
#define EGS_XSECTION_PREFIX_H_

#include <cstring>
#include <string>

#include "egs_path_length.h"

inline bool egsCopyXsectionPrefix(const std::string &value, char *target) {
    if (value.size() > EGS_MAX_XSECTION_PREFIX_LENGTH) {
        return false;
    }
    std::memset(target, ' ', EGS_MAX_XSECTION_PREFIX_LENGTH);
    std::memcpy(target, value.data(), value.size());
    return true;
}

#endif