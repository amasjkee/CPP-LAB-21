#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtGui/QImage>
#include "jpegloader.h"
#include "jpegsaver.h"
#include "imagehandler.h"

class MainWindow : public QMainWindow, public ImageLoadObserver
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // Реализация ImageLoadObserver
    void onImageLoaded(const QImage& image) override;
    void onLoadError(const QString& error) override;

private slots:
    void onLoadButtonClicked();
    void onSaveButtonClicked();
    void onNextScanButtonClicked();
    void onQualityChanged(int value);

private:
    
    QLabel* imageLabel;
    QPushButton* loadButton;
    QPushButton* saveButton;
    QPushButton* nextScanButton;
    QCheckBox* progressiveCheckBox;
    QComboBox* dctComboBox;
    QSlider* qualitySlider;
    QSpinBox* qualitySpinBox;
    
    QImage currentImage;
    ImageHandler* imageHandler;
    LoadImageCommand* loadCommand;
    
    void setupUI();
    void updateImageDisplay(const QImage& image);
    void updateNextScanButton();
};

#endif // MAINWINDOW_H

