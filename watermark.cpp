#include "watermark.h"

#include <algorithm>
#include <cmath>

namespace {

// Вычисляет координаты левого верхнего угла водяного знака.
// Если водяной знак больше base, координаты прижимаются к (0, 0),
// чтобы не выйти за границы — дальше область наложения будет обрезана.
void computeOffset(int baseW, int baseH, int markW, int markH,
                   WatermarkPosition pos, int& offsetX, int& offsetY) {
    switch (pos) {
        case WatermarkPosition::BottomRight:
            offsetX = baseW - markW;
            offsetY = baseH - markH;
            break;
        case WatermarkPosition::Center:
            offsetX = (baseW - markW) / 2;
            offsetY = (baseH - markH) / 2;
            break;
        case WatermarkPosition::TopLeft:
            offsetX = 0;
            offsetY = 0;
            break;
    }
    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;
}

} // namespace

BmpImage applyWatermark(BmpImage base,
                        const BmpImage& mark,
                        double alpha,
                        WatermarkPosition position) {
    if (base.empty() || mark.empty()) {
        return base; // нечего накладывать
    }

    // Ограничиваем alpha диапазоном [0, 1].
    if (alpha < 0.0) alpha = 0.0;
    if (alpha > 1.0) alpha = 1.0;

    const int baseW = base.width();
    const int baseH = base.height();
    const int markW = mark.width();
    const int markH = mark.height();

    int offsetX = 0;
    int offsetY = 0;
    computeOffset(baseW, baseH, markW, markH, position, offsetX, offsetY);

    // Ширина и высота области наложения = пересечение base и mark.
    const int overlayW = std::min(markW, baseW - offsetX);
    const int overlayH = std::min(markH, baseH - offsetY);

    const double invAlpha = 1.0 - alpha;

    for (int y = 0; y < overlayH; ++y) {
        for (int x = 0; x < overlayW; ++x) {
            uint8_t* basePix = base.pixel(offsetX + x, offsetY + y);
            const uint8_t* markPix = mark.pixel(x, y);

            // Смешиваем каждый цветовой канал: R, G, B.
            for (int c = 0; c < 3; ++c) {
                // Считаем в int, чтобы избежать переполнения uint8_t.
                const int blended = static_cast<int>(
                    std::lround(basePix[c] * invAlpha + markPix[c] * alpha));
                // Ограничиваем 0..255 на случай погрешностей округления.
                basePix[c] = static_cast<uint8_t>(std::clamp(blended, 0, 255));
            }
        }
    }

    return base;
}