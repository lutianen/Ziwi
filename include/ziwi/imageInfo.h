/**
 * @brief
 */

#pragma once

#include <string>

enum ImageType { UNKNOWN = -1, RAW, PNG, JPG, TIFF, SVG };

struct ImageInfo {
    unsigned char* data_;
    std::string name_;
    ImageType type_;
};