#pragma once

#include "bmp.h"

// Позиция водяного знака на основном изображении.
enum class WatermarkPosition {
    BottomRight,   // правый нижний угол (по умолчанию)
    Center,        // по центру
    TopLeft        // левый верхний угол
};

// Накладывает водяной знак mark на изображение base с коэффициентом
// прозрачности alpha (0.0 — только base, 1.0 — только mark).
// Возвращает новое изображение — копию base с изменённой областью.
// Если mark больше base, накладывается только пересекающаяся область
// (лишняя часть водяного знака отбрасывается).
BmpImage applyWatermark(BmpImage base, const BmpImage& mark, double alpha = 0.5, WatermarkPosition position = WatermarkPosition::BottomRight);
