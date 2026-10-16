#include "ImageCodec.h"
#include <cstdlib>
#include <cstring>
#include <climits>
#include <memory>

#define STBI_ONLY_JPEG
#define STBI_ONLY_TGA
#define STB_IMAGE_IMPLEMENTATION
#include "../../../lib/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../../lib/stb/stb_image_write.h"

namespace ImageCodec {
Image::~Image() { stbi_image_free(pixels); }

int PaddedExtent(int size, int maxSize)
{
    if (size <= 0 || size > maxSize) return 0;
    int padded = 1;
    while (padded < size) {
        if (padded > maxSize / 2) return 0;
        padded *= 2;
    }
    return padded <= maxSize ? padded : 0;
}

bool Decode(FILE* file, int prefix, Format format, int maxSize, Image& out)
{
    if (!file || prefix < 0 || maxSize <= 0 || fseek(file, 0, SEEK_END)) return false;
    const long end = ftell(file);
    if (end <= prefix || end - prefix > INT_MAX - 2 || fseek(file, prefix, SEEK_SET)) return false;
    size_t length = static_cast<size_t>(end - prefix);
    std::unique_ptr<unsigned char, decltype(&free)> bytes(
        static_cast<unsigned char*>(malloc(length + 2)), &free);
    if (!bytes || fread(bytes.get(), 1, length, file) != length) return false;
    unsigned char* data = bytes.get();
    const int channels = format == Format::Jpeg ? 3 : 4;
    if (format == Format::Jpeg) {
        if (length < 2 || data[0] != 0xff || data[1] != 0xd8) return false;
        // IJG's stdio source injects EOI at EOF (JWRN_JPEG_EOF). Keep that
        // recovery: stb otherwise fills an unfinished final scan differently.
        if (data[length - 2] != 0xff || data[length - 1] != 0xd9) {
            data[length++] = 0xff;
            data[length++] = 0xd9;
        }
    } else {
        if (length < 18 || data[0] != 0 || data[1] != 0 || data[2] != 2 || data[16] != 32)
            return false;
        const int width = data[12] | (data[13] << 8);
        const int height = data[14] | (data[15] << 8);
        if (!PaddedExtent(width, maxSize) || !PaddedExtent(height, maxSize) ||
            static_cast<size_t>(width) * height > (length - 18) / 4) return false;
        // The legacy loader always reversed storage rows, ignoring origin bits.
        // Normalize the local header only; assets on disk are never modified.
        data[17] &= ~0x30;
    }
    int width = 0, height = 0, sourceChannels = 0;
    if (!stbi_info_from_memory(data, static_cast<int>(length), &width, &height, &sourceChannels) ||
        !PaddedExtent(width, maxSize) || !PaddedExtent(height, maxSize)) return false;
    const size_t paddedWidth = static_cast<size_t>(PaddedExtent(width, maxSize));
    const size_t paddedHeight = static_cast<size_t>(PaddedExtent(height, maxSize));
    if (paddedWidth > static_cast<size_t>(INT_MAX) / channels / paddedHeight) return false;
    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(length),
        &width, &height, &sourceChannels, channels);
    if (!pixels) return false;
    stbi_image_free(out.pixels);
    out.pixels = pixels;
    out.width = width; out.height = height; out.channels = channels;
    return true;
}

struct Writer { FILE* file; bool ok; };
static void Write(void* context, void* data, int size)
{
    Writer* writer = static_cast<Writer*>(context);
    if (writer->ok && fwrite(data, 1, size, writer->file) != static_cast<size_t>(size))
        writer->ok = false;
}
bool WriteJpeg(FILE* file, int width, int height, const void* framebuffer, int quality)
{
    if (!file || !framebuffer || width <= 0 || height <= 0 || width > 65535 || height > 65535 ||
        static_cast<size_t>(width) * height > static_cast<size_t>(INT_MAX) / 3) return false;
    const size_t stride = static_cast<size_t>(width) * 3;
    std::unique_ptr<unsigned char, decltype(&free)> topDown(
        static_cast<unsigned char*>(malloc(stride * height)), &free);
    if (!topDown) return false;
    for (int y = 0; y < height; ++y)
        memcpy(topDown.get() + y * stride,
            static_cast<const unsigned char*>(framebuffer) + (height - 1 - y) * stride, stride);
    // IJG clamps to 1..100; stb's special quality=0 default must not replace it.
    if (quality < 1) quality = 1;
    if (quality > 100) quality = 100;
    Writer writer = { file, true };
    const int result = stbi_write_jpg_to_func(Write, &writer, width, height, 3, topDown.get(), quality);
    return result != 0 && writer.ok && !ferror(file);
}
}
