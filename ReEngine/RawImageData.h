#pragma once
#include <memory>
#include "stb/stb_image.h" // For the custom deleter

// A simple, renderer-agnostic container for raw pixel data.
struct RawImageData {
    int width;
    int height;
    int channels;
    // Use a shared_ptr with a custom deleter to automatically call stbi_image_free.
    std::shared_ptr<unsigned char> pixels;

    RawImageData(int w, int h, int c, unsigned char* data)
        : width(w), height(h), channels(c),
        pixels(data, stbi_image_free) // The magic happens here!
    {
    }

    // Check if the data is valid.
    bool isValid() const {
        return pixels != nullptr && width > 0 && height > 0;
    }
};