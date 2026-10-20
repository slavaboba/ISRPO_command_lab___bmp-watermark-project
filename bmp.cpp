#include "bmp.h"

#include <algorithm>
#include <fstream>

namespace {
#pragma pack(push, 1);
struct BitmapFileHeader {
    uint16_t bfType;       // 'BM' (0x4D42)
    uint32_t bfSize;       // общий размер файла
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;    // смещение до пиксельных данных
};

struct BitmapInfoHeader {
    uint32_t biSize;           // = 40
    int32_t  biWidth;
    int32_t  biHeight;         // > 0 — bottom-up, < 0 — top-down
    uint16_t biPlanes;         // = 1
    uint16_t biBitCount;       // = 24
    uint32_t biCompression;    // = 0 (BI_RGB)
    uint32_t biSizeImage;      // может быть 0 для BI_RGB
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)
static_assert(sizeof(BitmapFileHeader) == 14, "BITMAPFILEHEADER must be 14 bytes");
static_assert(sizeof(BitmapInfoHeader) == 40, "BITMAPINFOHEADER must be 40 bytes");

constexpr uint16_t BMP_SIGNATURE = 0x4D42; // 'BM'
constexpr uint16_t BMP_BITS_PER_PIXEL = 24;
constexpr uint32_t BMP_BI_RGB = 0;
constexpr uint32_t BMP_HEADER_OFFSET = 14 + 40; // 54

// Строка в 24-бит BMP выравнивается по 4 байта (padding)
int paddedRowSize(int width) {
    return ((width * 3 + 3) / 4) * 4;
}

}

bool BmpImage::load(const std::string& path) {
    lastError_.clear();

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        setError("Не удалось открыть файл: " + path);
        return false;
    }

    BitmapFileHeader fileHeader{};
    in.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    if (!in) {
        setError("Не удалось прочитать BITMAPFILEHEADER");
        return false;
    }
    if (fileHeader.bfType != BMP_SIGNATURE) {
        setError("Файл не является BMP (неверная сигнатура)");
        return false;
    }

    BitmapInfoHeader infoHeader{};
    in.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
    if (!in) {
        setError("Не удалось прочитать BITMAPINFOHEADER");
        return false;
    }

    if (infoHeader.biBitCount != BMP_BITS_PER_PIXEL) {
        setError("Поддерживаются только 24-битные BMP");
        return false;
    }
    if (infoHeader.biCompression != BMP_BI_RGB) {
        setError("Поддерживаются только несжатые BMP (BI_RGB)");
        return false;
    }
    if (infoHeader.biWidth <= 0 || infoHeader.biHeight == 0) {
        setError("Некорректные размеры изображения");
        return false;
    }
    if (infoHeader.biHeight < 0) {
        setError("BMP с top-down порядком (biHeight < 0) не поддерживается");
        return false;
    }

    const int w = infoHeader.biWidth;
    const int h = infoHeader.biHeight;
    const int rowSize = paddedRowSize(w);

    // Проверим, что в файле достаточно данных до пиксельного массива.
    in.seekg(0, std::ios::end);
    const std::streamoff fileSize = in.tellg();
    if (fileSize < 0) {
        setError("Не удалось определить размер файла");
        return false;
    }
    const std::streamoff pixelDataSize = static_cast<std::streamoff>(rowSize) * h;
    if (fileSize < static_cast<std::streamoff>(fileHeader.bfOffBits) + pixelDataSize) {
        setError("Файл повреждён: недостаточно пиксельных данных");
        return false;
    }

    // Переходим к началу пиксельного массива.
    in.seekg(static_cast<std::streamoff>(fileHeader.bfOffBits), std::ios::beg);
    if (!in) {
        setError("Не удалось перейти к пиксельным данным");
        return false;
    }

    // Выделяем память и читаем построчно.
    width_ = w;
    height_ = h;
    pixels_.assign(static_cast<std::size_t>(w) * h * 3, 0);

    std::vector<uint8_t> row(static_cast<std::size_t>(rowSize), 0);

    for (int fileY = 0; fileY < h; ++fileY) {
        in.read(reinterpret_cast<char*>(row.data()), rowSize);
        if (!in) {
            setError("Не удалось прочитать строку пиксельных данных");
            pixels_.clear();
            width_ = height_ = 0;
            return false;
        }

        // В файле строки идут снизу вверх — переворачиваем.
        const int y = h - 1 - fileY;

        for (int x = 0; x < w; ++x) {
            const uint8_t b = row[static_cast<std::size_t>(x) * 3 + 0];
            const uint8_t g = row[static_cast<std::size_t>(x) * 3 + 1];
            const uint8_t r = row[static_cast<std::size_t>(x) * 3 + 2];

            const std::size_t idx = (static_cast<std::size_t>(y) * w + x) * 3;
            pixels_[idx + 0] = r;
            pixels_[idx + 1] = g;
            pixels_[idx + 2] = b;
        }
        // padding-байты в конце строки просто игнорируем.
    }

    return true;
}
bool BmpImage::save(const std::string& path) const {
    lastError_.clear();
    if (empty()) {
        setError("Нечего сохранять, изображение пустое");
        return false;
    }
    const int w = width_;
    const int h = height_;
    const int rowSize = paddedRowSize(w);
    const std::size_t pixelDataSize  = static_cast<std::size_t>(rowSize) * h;
    const uint32_t fileSize = BMP_HEADER_OFFSET + static_cast<uint32_t>(pixelDataSize);

    BitmapFileHeader fileHeader{};
    fileHeader.bfType = BMP_SIGNATURE;
    fileHeader.bfSize = fileSize;
    fileHeader.bfReserved1 = 0;
    fileHeader.bfReserved2 = 0;
    fileHeader.bfOffBits = BMP_HEADER_OFFSET;

    BitmapInfoHeader infoHeader{};
    infoHeader.biSize = sizeof(BitmapInfoHeader);
    infoHeader.biWidth = w;
    infoHeader.biHeight = h; // > 0 — bottom-up, как в файле
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = BMP_BITS_PER_PIXEL;
    infoHeader.biCompression = BMP_BI_RGB;
    infoHeader.biSizeImage = static_cast<uint32_t>(pixelDataSize);
    infoHeader.biXPelsPerMeter = 2835; // ~72 DPI
    infoHeader.biYPelsPerMeter = 2835;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        setError("Не удалось открыть файл для записи: " + path);
        return false;
    }

    out.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    out.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));
    if (!out) {
        setError("Ошибка записи заголовков BMP");
        return false;
    }
    std::vector<uint8_t> row(static_cast<std::size_t>(rowSize), 0);
     for (int fileY = 0; fileY < h; fileY++) {
        const int y = h - 1 - fileY;

        // Обнуляем padding в конце строки.
        std::fill(row.begin() + static_cast<std::size_t>(w) * 3, row.end(), 0);

        for (int x = 0; x < w; ++x) {
            const std::size_t idx = (static_cast<std::size_t>(y) * w + x) * 3;
            row[static_cast<std::size_t>(x) * 3 + 2] = pixels_[idx + 0]; // R
            row[static_cast<std::size_t>(x) * 3 + 1] = pixels_[idx + 1]; // G
            row[static_cast<std::size_t>(x) * 3 + 0] = pixels_[idx + 2]; // B
        }

        out.write(reinterpret_cast<const char*>(row.data()), rowSize);
        if (!out) {
            setError("Ошибка записи пиксельных данных");
            return false;
        }
    }

    return true;
}

void BmpImage::create(int width, int height) {
    lastError_.clear();
    if (width <= 0; height <= 0) {
        width_ = height_ = 0;
        pixels_.clear();
        return;
    }
    width_ = width;
    height_ = height;
    pixels_.assign(static_cast<std::size_t>(width) * height * 3, 0);
}