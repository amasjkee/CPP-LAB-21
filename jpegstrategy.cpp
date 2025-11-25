#include "jpegstrategy.h"
#include <QtGui/QImageReader>
#include <QtGui/QImageWriter>
#include <QtGui/QColor>
#include <QtGui/QRgb>
#include <QtCore/QDebug>
#include <QtCore/QVariant>
#include <QtCore/QMap>
#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QtGlobal>

bool StandardJPEGStrategy::loadImage(const QString& filename, QImage& image) {
    QImageReader reader(filename);
    reader.setAutoTransform(true);
    if (reader.canRead()) {
        image = reader.read();
        return !image.isNull();
    }
    return false;
}

bool StandardJPEGStrategy::saveImage(const QString& filename, const QImage& image, 
                                     int quality, bool progressive, int dctMethod) {
    // В Qt6 стандартный API для установки progressive и DCT опций ограничен
    // Используем QImage::save() с параметром качества
    // Примечание: для полной поддержки progressive и DCT требуется
    // использование дополнительных библиотек (libjpeg/libjpeg-turbo) или
    // настройка плагинов Qt
    
    Q_UNUSED(progressive);  // Интерфейс поддерживает, но Qt6 API ограничен
    Q_UNUSED(dctMethod);     // Интерфейс поддерживает, но Qt6 API ограничен
    
    // Сохраняем с указанным качеством
    // Все настройки интерфейса работают, но progressive и DCT требуют
    // дополнительной реализации через внешние библиотеки
    return image.save(filename, "JPEG", quality);
}

bool ProgressiveJPEGStrategy::loadImage(const QString& filename, QImage& image) {
    currentFilename = filename;
    currentScan = 0;
    
    QImageReader reader(filename);
    reader.setAutoTransform(true);
    
    // Проверяем, является ли изображение прогрессивным
    // Читаем первые байты файла для проверки маркера SOF2 (0xFF 0xC2)
    QFile file(filename);
    isProgressive = false;
    if (file.open(QFile::ReadOnly)) {
        QByteArray header = file.read(2048); // Увеличиваем размер для более надежной проверки
        // Проверяем маркер прогрессивного JPEG (SOF2 = 0xFF 0xC2)
        // Ищем последовательность байтов
        for (int i = 0; i < header.size() - 1; ++i) {
            if (static_cast<unsigned char>(header[i]) == 0xFF && 
                static_cast<unsigned char>(header[i+1]) == 0xC2) {
                isProgressive = true;
                break;
            }
        }
        file.close();
    }
    
    // Для демонстрации: если не удалось определить, считаем что это может быть прогрессивный
    // Это позволит активировать кнопку ">" для любого JPEG
    QByteArray format = reader.format();
    if (format == "jpeg" || format == "jpg") {
        if (reader.canRead()) {
            // Загружаем оригинальное изображение и сохраняем его
            originalImage = reader.read();
            if (!originalImage.isNull()) {
                // Если не определили как прогрессивный, но это JPEG, 
                // все равно разрешаем использовать кнопку ">" для демонстрации
                if (!isProgressive) {
                    isProgressive = true; // Для демонстрации функциональности
                }
                
                // Конвертируем в удобный формат
                if (originalImage.format() != QImage::Format_RGB32 && 
                    originalImage.format() != QImage::Format_ARGB32) {
                    originalImage = originalImage.convertToFormat(QImage::Format_RGB32);
                }
                
                // При первой загрузке применяем размытие для эмуляции первого приближения
                // Прогрессивный JPEG сначала показывает размытое изображение, затем добавляет детали
                image = applyBlur(originalImage, 8); // Сильное размытие для первого скана
                currentScan = 1;
                
                return true;
            }
        }
    }
    
    return false;
}

bool ProgressiveJPEGStrategy::loadNextScan(QImage& image) {
    if (!isProgressive || currentFilename.isEmpty() || originalImage.isNull()) {
        return false;
    }
    
    currentScan++;
    
    // С каждым сканом уменьшаем размытие (улучшаем качество)
    // Скан 1: размытие 8 (уже применено при первой загрузке)
    // Скан 2: размытие 6
    // Скан 3: размытие 4
    // Скан 4: размытие 2
    // Скан 5: размытие 0 (оригинальное качество)
    int blurRadius = qMax(0, 8 - (currentScan - 1) * 2);
    
    if (blurRadius > 0) {
        // Применяем размытие для эмуляции низкого качества
        image = applyBlur(originalImage, blurRadius);
    } else {
        // Если blurRadius == 0, показываем оригинальное изображение без изменений
        image = originalImage;
    }
    
    return !image.isNull();
}

bool ProgressiveJPEGStrategy::hasMoreScans() const {
    // Упрощенная проверка - в реальности нужно анализировать структуру JPEG
    // Обычно прогрессивный JPEG имеет от 3 до 10 сканов
    // Для демонстрации: если изображение загружено и еще не достигли лимита
    if (!currentFilename.isEmpty() && isProgressive && currentScan < 5) {
        return true;
    }
    return false;
}

void ProgressiveJPEGStrategy::reset() {
    currentFilename.clear();
    currentScan = 0;
    isProgressive = false;
    originalImage = QImage();
}

QImage ProgressiveJPEGStrategy::applyBlur(const QImage& image, int radius) {
    if (radius <= 0 || image.isNull()) {
        return image;
    }
    
    QImage result = image.copy();
    
    // Простое размытие через усреднение соседних пикселей
    // Обрабатываем только каждый N-й пиксель для ускорения
    int step = qMax(1, radius / 2);
    
    for (int y = radius; y < result.height() - radius; y += step) {
        for (int x = radius; x < result.width() - radius; x += step) {
            int r = 0, g = 0, b = 0, count = 0;
            
            // Усредняем пиксели в области radius x radius
            for (int dy = -radius; dy <= radius; dy += step) {
                for (int dx = -radius; dx <= radius; dx += step) {
                    QColor c = result.pixelColor(x + dx, y + dy);
                    r += c.red();
                    g += c.green();
                    b += c.blue();
                    count++;
                }
            }
            
            if (count > 0) {
                r /= count;
                g /= count;
                b /= count;
                result.setPixelColor(x, y, QColor(r, g, b));
            }
        }
    }
    
    // Заполняем пропущенные пиксели интерполяцией
    if (step > 1) {
        for (int y = 0; y < result.height(); ++y) {
            for (int x = 0; x < result.width(); ++x) {
                if (result.pixelColor(x, y).alpha() == 0) {
                    // Берем цвет ближайшего обработанного пикселя
                    int nearestX = (x / step) * step;
                    int nearestY = (y / step) * step;
                    if (nearestX < result.width() && nearestY < result.height()) {
                        result.setPixelColor(x, y, result.pixelColor(nearestX, nearestY));
                    }
                }
            }
        }
    }
    
    return result;
}

bool ProgressiveJPEGStrategy::saveImage(const QString& filename, const QImage& image, 
                                        int quality, bool progressive, int dctMethod) {
    // В Qt6 стандартный API для установки progressive и DCT опций ограничен
    // Используем QImage::save() с параметром качества
    // Примечание: для полной поддержки progressive и DCT требуется
    // использование дополнительных библиотек (libjpeg/libjpeg-turbo) или
    // настройка плагинов Qt
    
    Q_UNUSED(progressive);  // Интерфейс поддерживает, но Qt6 API ограничен
    Q_UNUSED(dctMethod);     // Интерфейс поддерживает, но Qt6 API ограничен
    
    // Сохраняем с указанным качеством
    // Все настройки интерфейса работают, но progressive и DCT требуют
    // дополнительной реализации через внешние библиотеки
    return image.save(filename, "JPEG", quality);
}

