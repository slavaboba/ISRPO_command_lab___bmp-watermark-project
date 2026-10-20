#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// Класс для работы с 24-битными несжатыми BMP-изображениями (BI_RGB)
//
// Внутреннее представление:
// - пиксели хранятся в порядке "сверху вниз" (top-down)
// - цветовые компоненты хранятся в порядке R, G, B
// - координаты (0, 0) — левый верхний угол
// - каждый пиксель занимает 3 байта

class BmpImage {
public:
    BmpImage() = default;

    //Загружаем 24 битный файл
    //true если удалось, иначе false
    bool load(const std::string& path);

    //сохраняем текущее изображение как 24 битный файл
    //true если удалось, иначе fasle
    bool save(const std::string& path) const;
    
    //создает изображение(все пиксели черные)
    void create(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }
    bool empty() const { return width_ <= 0 || height_ <= 0; }

    // Проверка, что координаты попадают в изображение
    bool inBounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < width_ && y < height_;
    }

    // Доступ к пикселю. Возвращает указатель на 3 байта: R, G, B
    // Перед вызовом проверьте inBounds(x, y).
    uint8_t* pixel(int x, int y) {
        return &pixels_[(static_cast<std::size_t>(y) * width_ + x) * 3];
    }
    const uint8_t* pixel(int x, int y) const {
        return &pixels_[(static_cast<std::size_t>(y) * width_ + x) * 3];
    }
    // Прямой доступ к массиву пикселей (для наложения водяного знака)
    std::vector<uint8_t>& data() { return pixels_; }
    const std::vector<uint8_t>& data() const { return pixels_; }

    // Текст последней ошибки
    const std::string& lastError() const { return lastError_; }

    private:
        void setError(const std::string& msg) const { lastError_ = msg; }

        int width_ = 0;
        int height_ = 0;
        std::vector<uint8_t> pixels_; // R, G, B; размер = width_ * height_ * 3

        mutable std::string lastError_;
};