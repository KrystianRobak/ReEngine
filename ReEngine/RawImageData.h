#pragma once
#include <memory>
#include "stb/stb_image.h"
struct RawImageData {
    int width;
    int height;
    int channels;

    std::shared_ptr<unsigned char> pixels;

    RawImageData(int w, int h, int c, unsigned char* data)
        : width(w), height(h), channels(c),
        pixels(data, stbi_image_free)
    {
    }

    bool isValid() const {
        return pixels != nullptr && width > 0 && height > 0;
    }
};