#include "imagehandler.h"

ImageHandler* ImageHandler::createHandler(HandlerType type) {
    switch (type) {
        case Standard:
            return new StandardImageHandler();
        case Progressive:
            return new ProgressiveImageHandler();
        default:
            return new StandardImageHandler();
    }
}

