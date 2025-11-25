#ifndef JPEGSTRATEGY_H
#define JPEGSTRATEGY_H

#include <QtGui/QImage>
#include <QtCore/QString>

// Strategy Pattern: Интерфейс для различных стратегий обработки JPEG
class JPEGStrategy {
public:
    virtual ~JPEGStrategy() = default;
    virtual bool loadImage(const QString& filename, QImage& image) = 0;
    virtual bool saveImage(const QString& filename, const QImage& image, 
                          int quality, bool progressive, int dctMethod) = 0;
};

// Конкретная стратегия для стандартной загрузки
class StandardJPEGStrategy : public JPEGStrategy {
public:
    bool loadImage(const QString& filename, QImage& image) override;
    bool saveImage(const QString& filename, const QImage& image, 
                  int quality, bool progressive, int dctMethod) override;
};

// Конкретная стратегия для прогрессивной загрузки
class ProgressiveJPEGStrategy : public JPEGStrategy {
public:
    bool loadImage(const QString& filename, QImage& image) override;
    bool saveImage(const QString& filename, const QImage& image, 
                  int quality, bool progressive, int dctMethod) override;
    
    // Загрузка следующего приближения
    bool loadNextScan(QImage& image);
    
    // Проверка, есть ли еще приближения
    bool hasMoreScans() const;
    
    // Сброс состояния
    void reset();

private:
    QString currentFilename;
    int currentScan = 0;
    bool isProgressive = false;
    QImage originalImage; // Сохраняем оригинальное изображение для постепенного улучшения
    
    // Вспомогательная функция для применения размытия
    QImage applyBlur(const QImage& image, int radius);
};

#endif // JPEGSTRATEGY_H

