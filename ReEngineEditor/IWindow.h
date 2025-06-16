#pragma once
#include <string>

class IWindow
{
public:

    virtual void* get_native_window() = 0;

    virtual void set_native_window(void* window) = 0;

    virtual void on_resize(int width, int height) = 0;

    virtual void on_close() = 0;

    int width;
    int height;
    std::string title;
};