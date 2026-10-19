#pragma once
#include <cstdio>

namespace ImageCodec {
enum class Format { Jpeg, Tga32 };
struct Image {
    unsigned char* pixels = nullptr;
    int width = 0, height = 0, channels = 0;
    Image() = default;
    ~Image();
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;
};
// FILE ownership stays with the caller. prefix is relative to the file start.
// TGA retains the client's raw 32-bit, bottom-up convention, including its
// rejection of 24-bit assets. Failure leaves the destination unchanged.
bool Decode(FILE* file, int prefix, Format format, int maxSize, Image& out);
int PaddedExtent(int size, int maxSize);
// Framebuffer is RGB, bottom-up. Writes through the caller's filesystem FILE.
bool WriteJpeg(FILE* file, int width, int height, const void* framebuffer, int quality);
}
