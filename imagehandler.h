#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include "jpegstrategy.h"
#include <QtGui/QImage>
#include <QtCore/QString>
#include <QtCore/QtGlobal>

// Factory Pattern: Создание обработчиков изображений
class ImageHandler {
public:
    enum HandlerType {
        Standard,
        Progressive
    };
    
    static ImageHandler* createHandler(HandlerType type);
    virtual ~ImageHandler() = default;
    
    virtual bool loadImage(const QString& filename, QImage& image) = 0;
    virtual bool saveImage(const QString& filename, const QImage& image, 
                          int quality, bool progressive, int dctMethod) = 0;
    
    // Для прогрессивной загрузки
    virtual bool loadNextScan(QImage& image) { Q_UNUSED(image); return false; }
    virtual bool hasMoreScans() const { return false; }
    virtual void reset() {}

protected:
    JPEGStrategy* strategy;
    ImageHandler(JPEGStrategy* strategy) : strategy(strategy) {}
};

class StandardImageHandler : public ImageHandler {
public:
    StandardImageHandler() : ImageHandler(new StandardJPEGStrategy()) {}
    ~StandardImageHandler() { delete strategy; }
    
    bool loadImage(const QString& filename, QImage& image) override {
        return strategy->loadImage(filename, image);
    }
    
    bool saveImage(const QString& filename, const QImage& image, 
                  int quality, bool progressive, int dctMethod) override {
        return strategy->saveImage(filename, image, quality, progressive, dctMethod);
    }
};

class ProgressiveImageHandler : public ImageHandler {
public:
    ProgressiveImageHandler() : ImageHandler(new ProgressiveJPEGStrategy()) {}
    ~ProgressiveImageHandler() { delete strategy; }
    
    bool loadImage(const QString& filename, QImage& image) override {
        return strategy->loadImage(filename, image);
    }
    
    bool saveImage(const QString& filename, const QImage& image, 
                  int quality, bool progressive, int dctMethod) override {
        return strategy->saveImage(filename, image, quality, progressive, dctMethod);
    }
    
    bool loadNextScan(QImage& image) override {
        ProgressiveJPEGStrategy* progStrategy = dynamic_cast<ProgressiveJPEGStrategy*>(strategy);
        if (progStrategy) {
            return progStrategy->loadNextScan(image);
        }
        return false;
    }
    
    bool hasMoreScans() const override {
        ProgressiveJPEGStrategy* progStrategy = dynamic_cast<ProgressiveJPEGStrategy*>(strategy);
        if (progStrategy) {
            return progStrategy->hasMoreScans();
        }
        return false;
    }
    
    void reset() override {
        ProgressiveJPEGStrategy* progStrategy = dynamic_cast<ProgressiveJPEGStrategy*>(strategy);
        if (progStrategy) {
            progStrategy->reset();
        }
    }
};

#endif // IMAGEHANDLER_H

