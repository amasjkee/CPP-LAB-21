QT += core gui widgets

CONFIG += c++17

TARGET = jpeg_viewer
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    jpegloader.cpp \
    jpegsaver.cpp \
    imagehandler.cpp \
    jpegstrategy.cpp

HEADERS += \
    mainwindow.h \
    jpegloader.h \
    jpegsaver.h \
    imagehandler.h \
    jpegstrategy.h

