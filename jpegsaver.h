#ifndef JPEGSAVER_H
#define JPEGSAVER_H

#include "imagehandler.h"
#include <QtCore/QObject>
#include <QtGui/QImage>
#include <QtCore/QString>

class SaveImageCommand {
public:
    SaveImageCommand(ImageHandler* handler, const QString& filename, const QImage& image,
                    int quality, bool progressive, int dctMethod)
        : handler(handler), filename(filename), image(image),
          quality(quality), progressive(progressive), dctMethod(dctMethod) {}
    
    bool execute() {
        return handler->saveImage(filename, image, quality, progressive, dctMethod);
    }

private:
    ImageHandler* handler;
    QString filename;
    QImage image;
    int quality;
    bool progressive;
    int dctMethod;
};

#endif // JPEGSAVER_H

