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
    
    Q_UNUSED(progressive);
    Q_UNUSED(dctMethod);
    
    return image.save(filename, "JPEG", quality);
}

bool ProgressiveJPEGStrategy::loadImage(const QString& filename, QImage& image) {
    currentFilename = filename;
    currentScan = 0;
    
    QImageReader reader(filename);
    reader.setAutoTransform(true);
    
    QFile file(filename);
    isProgressive = false;
    if (file.open(QFile::ReadOnly)) {
        QByteArray header = file.read(2048);
        for (int i = 0; i < header.size() - 1; ++i) {
            if (static_cast<unsigned char>(header[i]) == 0xFF && 
                static_cast<unsigned char>(header[i+1]) == 0xC2) {
                isProgressive = true;
                break;
            }
        }
        file.close();
    }
    
    QByteArray format = reader.format();
    if (format == "jpeg" || format == "jpg") {
        if (reader.canRead()) {
            originalImage = reader.read();
            if (!originalImage.isNull()) {
                if (!isProgressive) {
                    isProgressive = true;
                }
                
                if (originalImage.format() != QImage::Format_RGB32 &&
                    originalImage.format() != QImage::Format_ARGB32) {
                    originalImage = originalImage.convertToFormat(QImage::Format_RGB32);
                }

                currentScan = 1;
                image = applyBlur(originalImage, 8);
                
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
    
    int blurRadius = qMax(0, 8 - (currentScan - 1) * 2);
    
    if (blurRadius > 0) {
        image = applyBlur(originalImage, blurRadius);
    } else {
        image = originalImage;
    }
    
    return !image.isNull();
}

bool ProgressiveJPEGStrategy::hasMoreScans() const {
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

    QImage result = QImage(image.size(), image.format());
    
    int step = qMax(1, radius / 2);
    
    for (int y = 0; y < result.height(); y += step) {
        for (int x = 0; x < result.width(); x += step) {
            int r = 0, g = 0, b = 0, count = 0;
            
            int yStart = qMax(0, y - radius);
            int yEnd = qMin(result.height() - 1, y + radius);
            int xStart = qMax(0, x - radius);
            int xEnd = qMin(result.width() - 1, x + radius);
            
            for (int py = yStart; py <= yEnd; py++) {
                for (int px = xStart; px <= xEnd; px++) {
                    QColor c = image.pixelColor(px, py);
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
                QColor avgColor(r, g, b);
                
                for (int py = y; py < qMin(result.height(), y + step); py++) {
                    for (int px = x; px < qMin(result.width(), x + step); px++) {
                        result.setPixelColor(px, py, avgColor);
                    }
                }
            }
        }
    }
    
    return result;
}

bool ProgressiveJPEGStrategy::saveImage(const QString& filename, const QImage& image, 
                                        int quality, bool progressive, int dctMethod) {
    
    Q_UNUSED(progressive);
    Q_UNUSED(dctMethod);
    
    return image.save(filename, "JPEG", quality);
}
