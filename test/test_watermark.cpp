#include "bmp.h"
#include "watermark.h"
#include <cassert>
#include <iostream>

// Проверяем базовый сценарий: base 2x2 (все пиксели 100),
// mark 1x1 (200), alpha=0.5 → в правом нижнем углу должен
// получиться пиксель 150, остальные не меняются.
int main() {
    BmpImage base;
    base.create(2, 2);
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < 2; ++x) {
            uint8_t* p = base.pixel(x, y);
            p[0] = 100; p[1] = 100; p[2] = 100;
        }

    BmpImage mark;
    mark.create(1, 1);
    mark.pixel(0, 0)[0] = 200;
    mark.pixel(0, 0)[1] = 200;
    mark.pixel(0, 0)[2] = 200;

    BmpImage result = applyWatermark(base, mark, 0.5,
                                     WatermarkPosition::BottomRight);

    // Водяной знак 1x1 в правом нижнем углу — координата (1, 1).
    const uint8_t* p11 = result.pixel(1, 1);
    assert(p11[0] == 150 && p11[1] == 150 && p11[2] == 150);

    // Пиксель (0, 0) не должен измениться.
    const uint8_t* p00 = result.pixel(0, 0);
    assert(p00[0] == 100 && p00[1] == 100 && p00[2] == 100);

    std::cout << "Watermark test passed.\n";
    return 0;
}