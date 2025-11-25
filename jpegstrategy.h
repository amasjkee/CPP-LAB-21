#ifndef JPEGSTRATEGY_H
#define JPEGSTRATEGY_H

#include <QtGui/QImage>
#include <QtCore/QString>

class JPEGStrategy {
public:
    virtual ~JPEGStrategy() = default;
    virtual bool loadImage(const QString& filename, QImage& image) = 0;
    virtual bool saveImage(const QString& filename, const QImage& image, 
                          int quality, bool progressive, int dctMethod) = 0;
};

class StandardJPEGStrategy : public JPEGStrategy {
public:
    bool loadImage(const QString& filename, QImage& image) override;
    bool saveImage(const QString& filename, const QImage& image, 
                  int quality, bool progressive, int dctMethod) override;
};

class ProgressiveJPEGStrategy : public JPEGStrategy {
public:
    bool loadImage(const QString& filename, QImage& image) override;
    bool saveImage(const QString& filename, const QImage& image, 
                  int quality, bool progressive, int dctMethod) override;

    bool loadNextScan(QImage& image);

    bool hasMoreScans() const;

    void reset();

private:
    QString currentFilename;
    int currentScan = 0;
    bool isProgressive = false;
    QImage originalImage;

    QImage applyBlur(const QImage& image, int radius);
};

#endif // JPEGSTRATEGY_H

