#include "bmp.h"
#include "watermark.h"
#include <iostream>
#include <utility>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <base.bmp> <watermark.bmp> <output.bmp>\n";
        return 1;
    }

    BmpImage base;
    if (!base.load(argv[1])) {
        std::cerr << "Failed to load base: " << base.lastError() << "\n";
        return 1;
    }

    BmpImage mark;
    if (!mark.load(argv[2])) {
        std::cerr << "Failed to load watermark: " << mark.lastError() << "\n";
        return 1;
    }

    // Накладываем водяной знак (прозрачность 0.5, позиция - правый нижний угол)
    BmpImage result = applyWatermark(std::move(base), mark, 0.5,
                                     WatermarkPosition::BottomRight);

    if (!result.save(argv[3])) {
        std::cerr << "Failed to save result: " << result.lastError() << "\n";
        return 1;
    }

    std::cout << "Success! Watermarked image saved to " << argv[3] << "\n";
    return 0;
}