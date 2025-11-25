#ifndef JPEGLOADER_H
#define JPEGLOADER_H

#include "imagehandler.h"
#include <QtCore/QObject>
#include <QtGui/QImage>
#include <QtCore/QString>

// Observer Pattern: Уведомления о загрузке изображений
class ImageLoadObserver {
public:
    virtual ~ImageLoadObserver() = default;
    virtual void onImageLoaded(const QImage& image) = 0;
    virtual void onLoadError(const QString& error) = 0;
};

// Command Pattern: Команда загрузки изображения
class LoadImageCommand {
public:
    LoadImageCommand(ImageHandler* handler, const QString& filename, ImageLoadObserver* observer)
        : handler(handler), filename(filename), observer(observer) {}
    
    void execute() {
        QImage image;
        if (handler->loadImage(filename, image)) {
            if (observer) {
                observer->onImageLoaded(image);
            }
        } else {
            if (observer) {
                observer->onLoadError("Failed to load image: " + filename);
            }
        }
    }
    
    void executeNextScan() {
        QImage image;
        ProgressiveImageHandler* progHandler = dynamic_cast<ProgressiveImageHandler*>(handler);
        if (progHandler && progHandler->loadNextScan(image)) {
            if (observer) {
                observer->onImageLoaded(image);
            }
        } else {
            if (observer) {
                observer->onLoadError("No more scans available");
            }
        }
    }
    
    bool canLoadNextScan() const {
        ProgressiveImageHandler* progHandler = dynamic_cast<ProgressiveImageHandler*>(handler);
        return progHandler && progHandler->hasMoreScans();
    }

private:
    ImageHandler* handler;
    QString filename;
    ImageLoadObserver* observer;
};

#endif // JPEGLOADER_H

