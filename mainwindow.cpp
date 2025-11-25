#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , imageHandler(nullptr)
    , loadCommand(nullptr)
{
    setupUI();
    imageHandler = ImageHandler::createHandler(ImageHandler::Progressive);
}

MainWindow::~MainWindow()
{
    delete imageHandler;
    delete loadCommand;
}

void MainWindow::setupUI()
{
    setWindowTitle("JPEG Viewer with Progressive Loading");
    // Увеличиваем размер окна для лучшего отображения изображений
    setMinimumSize(1200, 900);
    resize(1200, 900);
    
    // Создаем status bar для отображения сообщений
    statusBar()->showMessage("Ready");
    
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Кнопки загрузки и сохранения
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    loadButton = new QPushButton("Load JPEG", this);
    saveButton = new QPushButton("Save JPEG", this);
    nextScanButton = new QPushButton(">", this);
    nextScanButton->setEnabled(false);
    nextScanButton->setMaximumWidth(50);
    
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(nextScanButton);
    buttonLayout->addStretch();
    
    mainLayout->addLayout(buttonLayout);
    
    // Отображение изображения - увеличиваем размер
    imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("QLabel { border: 2px solid gray; background-color: #2b2b2b; }");
    imageLabel->setMinimumSize(1000, 700);
    imageLabel->setText("No image loaded");
    // Разрешаем масштабирование изображения
    imageLabel->setScaledContents(false);
    mainLayout->addWidget(imageLabel);
    
    // Настройки сохранения
    QHBoxLayout* saveOptionsLayout = new QHBoxLayout();
    
    progressiveCheckBox = new QCheckBox("Progressive", this);
    saveOptionsLayout->addWidget(progressiveCheckBox);
    
    saveOptionsLayout->addWidget(new QLabel("DCT Method:", this));
    dctComboBox = new QComboBox(this);
    dctComboBox->addItem("Integer", 0);
    dctComboBox->addItem("Fast", 1);
    dctComboBox->addItem("Float", 2);
    saveOptionsLayout->addWidget(dctComboBox);
    
    saveOptionsLayout->addWidget(new QLabel("Quality:", this));
    qualitySlider = new QSlider(Qt::Horizontal, this);
    qualitySlider->setRange(0, 100);
    qualitySlider->setValue(75);
    saveOptionsLayout->addWidget(qualitySlider);
    
    qualitySpinBox = new QSpinBox(this);
    qualitySpinBox->setRange(0, 100);
    qualitySpinBox->setValue(75);
    saveOptionsLayout->addWidget(qualitySpinBox);
    
    saveOptionsLayout->addStretch();
    
    mainLayout->addLayout(saveOptionsLayout);
    mainLayout->addStretch();
    
    // Подключение сигналов
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoadButtonClicked);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveButtonClicked);
    connect(nextScanButton, &QPushButton::clicked, this, &MainWindow::onNextScanButtonClicked);
    connect(qualitySlider, &QSlider::valueChanged, this, &MainWindow::onQualityChanged);
    connect(qualitySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            qualitySlider, &QSlider::setValue);
    connect(qualitySlider, &QSlider::valueChanged, 
            qualitySpinBox, &QSpinBox::setValue);
}

void MainWindow::onLoadButtonClicked()
{
    QString filename = QFileDialog::getOpenFileName(this, 
        "Load JPEG Image", "", "JPEG Images (*.jpg *.jpeg)");
    
    if (filename.isEmpty()) {
        return;
    }
    
    // Определяем тип обработчика на основе файла
    QFileInfo fileInfo(filename);
    ImageHandler::HandlerType handlerType = ImageHandler::Progressive;
    
    // Пересоздаем обработчик если нужно
    if (imageHandler) {
        delete imageHandler;
    }
    imageHandler = ImageHandler::createHandler(handlerType);
    
    // Создаем команду загрузки
    if (loadCommand) {
        delete loadCommand;
    }
    loadCommand = new LoadImageCommand(imageHandler, filename, this);
    loadCommand->execute();
    
    updateNextScanButton();
}

void MainWindow::onSaveButtonClicked()
{
    if (currentImage.isNull()) {
        QMessageBox::warning(this, "Warning", "No image to save");
        return;
    }
    
    QString filename = QFileDialog::getSaveFileName(this, 
        "Save JPEG Image", "", "JPEG Images (*.jpg *.jpeg)");
    
    if (filename.isEmpty()) {
        return;
    }
    
    int quality = qualitySlider->value();
    bool progressive = progressiveCheckBox->isChecked();
    int dctMethod = dctComboBox->currentData().toInt();
    
    SaveImageCommand saveCommand(imageHandler, filename, currentImage, 
                                 quality, progressive, dctMethod);
    
    if (saveCommand.execute()) {
        QMessageBox::information(this, "Success", "Image saved successfully");
    } else {
        QMessageBox::critical(this, "Error", "Failed to save image");
    }
}

void MainWindow::onNextScanButtonClicked()
{
    if (loadCommand && loadCommand->canLoadNextScan()) {
        // Загружаем следующее приближение
        loadCommand->executeNextScan();
        // Обновляем состояние кнопки
        updateNextScanButton();
        // Показываем сообщение о загрузке следующего скана
        statusBar()->showMessage(QString("Loaded next scan. Click '>' to load more."), 2000);
    } else {
        statusBar()->showMessage("No more scans available", 2000);
    }
}

void MainWindow::onQualityChanged(int value)
{
    Q_UNUSED(value);
    // Обновление происходит автоматически через сигналы
}

void MainWindow::onImageLoaded(const QImage& image)
{
    currentImage = image;
    updateImageDisplay(image);
    // Обновляем кнопку после загрузки изображения
    updateNextScanButton();
}

void MainWindow::onLoadError(const QString& error)
{
    QMessageBox::critical(this, "Error", error);
    currentImage = QImage();
    updateImageDisplay(QImage());
    updateNextScanButton();
}

void MainWindow::updateImageDisplay(const QImage& image)
{
    if (image.isNull()) {
        imageLabel->setText("No image loaded");
        imageLabel->setPixmap(QPixmap());
        return;
    }
    
    QPixmap pixmap = QPixmap::fromImage(image);
    QSize labelSize = imageLabel->size();
    
    // Масштабируем изображение, чтобы оно помещалось в label, но сохраняем пропорции
    // Используем максимальный размер для лучшего отображения
    QSize scaledSize = pixmap.size();
    scaledSize.scale(labelSize, Qt::KeepAspectRatio);
    
    if (scaledSize != pixmap.size()) {
        pixmap = pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    imageLabel->setPixmap(pixmap);
    imageLabel->setText("");
}

void MainWindow::updateNextScanButton()
{
    if (loadCommand && imageHandler) {
        bool canLoad = loadCommand->canLoadNextScan();
        nextScanButton->setEnabled(canLoad);
        // Отладочная информация (можно убрать после проверки)
        // qDebug() << "updateNextScanButton: canLoad =" << canLoad;
    } else {
        nextScanButton->setEnabled(false);
    }
}

