#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <QtCore/QDebug>
#include "imagehandler.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Демонстрация функционала
    qDebug() << "=== JPEG Viewer Application ===";
    qDebug() << "Demonstrating design patterns:";
    qDebug() << "1. Strategy Pattern - JPEG loading/saving strategies";
    qDebug() << "2. Factory Pattern - Image handler creation";
    qDebug() << "3. Observer Pattern - Image load notifications";
    qDebug() << "4. Command Pattern - Load/Save operations";
    qDebug() << "";
    
    // Демонстрация Factory Pattern
    qDebug() << "Factory Pattern: Creating handlers";
    ImageHandler* standardHandler = ImageHandler::createHandler(ImageHandler::Standard);
    ImageHandler* progressiveHandler = ImageHandler::createHandler(ImageHandler::Progressive);
    qDebug() << "Standard handler created:" << (standardHandler != nullptr);
    qDebug() << "Progressive handler created:" << (progressiveHandler != nullptr);
    delete standardHandler;
    delete progressiveHandler;
    
    // Демонстрация Strategy Pattern
    qDebug() << "";
    qDebug() << "Strategy Pattern: Different JPEG strategies available";
    StandardJPEGStrategy standardStrategy;
    ProgressiveJPEGStrategy progressiveStrategy;
    qDebug() << "Strategies initialized successfully";
    
    // Создание и отображение главного окна
    MainWindow window;
    window.show();
    
    qDebug() << "";
    qDebug() << "Application started. Use the UI to:";
    qDebug() << "- Load JPEG images (supports progressive loading)";
    qDebug() << "- Click '>' button to load next scan of progressive JPEG";
    qDebug() << "- Save JPEG with custom quality, DCT method, and progressive mode";
    
    return app.exec();
}

