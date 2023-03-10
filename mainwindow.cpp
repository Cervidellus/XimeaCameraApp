#include "mainwindow.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QPushButton>
#include <QComboBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTimer>

#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include <imagelabel.h>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      imageLabel_(new ImageLabel()),
      cameraSelector_(new QComboBox(this)),
      binningSelector_(new QComboBox(this)),
      exposureValueEdit_(new QLineEdit(this)),
      gainValueEdit_(new QLineEdit(this)),
      savePathEdit_(new QLineEdit(this)),
      imageBaseNameEdit_(new QLineEdit(this)),
      videoBaseNameEdit_(new QLineEdit(this)),
      browsePathButton_(new QPushButton(this)),
      saveImageButton_(new QPushButton(this)),
      videoRecordButton_(new QPushButton(this)),
      cameraHandle_(new HANDLE(INVALID_HANDLE_VALUE)),
      cameraTimer_(new QTimer())
{
    //Setup UI
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
        imageLabel_->setMinimumSize(500,500);
    mainLayout->addWidget(imageLabel_);

        QHBoxLayout* bottomLayout = new QHBoxLayout(this);
            QGridLayout* cameraPropertiesLayout = new QGridLayout(this);
                QSizePolicy cameraPropertySizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

                cameraPropertiesLayout->addWidget(new QLabel("Select Camera:"), 0,0);
                QStringList cameraList = enumerateCameras_();
                cameraSelector_->addItems(enumerateCameras_());
                cameraPropertiesLayout->addWidget(cameraSelector_,0,1);

                cameraPropertiesLayout->addWidget(new QLabel("Exposure(ms):"), 1, 0);
                exposureValueEdit_->setSizePolicy(cameraPropertySizePolicy);
                exposureValueEdit_->setValidator(new QIntValidator(0,5000));
                cameraPropertiesLayout->addWidget(exposureValueEdit_, 1, 1);

                cameraPropertiesLayout->addWidget(new QLabel("Gain:"), 2, 0);
                gainValueEdit_->setSizePolicy(cameraPropertySizePolicy);
                gainValueEdit_->setValidator(new QDoubleValidator(0,20,2));
                cameraPropertiesLayout->addWidget(gainValueEdit_, 2,1);

                cameraPropertiesLayout->addWidget(new QLabel("Binning:"),3,0);
                binningSelector_->addItems({"1","2"});
                cameraPropertiesLayout->addWidget(binningSelector_,3,1);

            QGridLayout* IOLayout = new QGridLayout(this);
                IOLayout->addWidget(new QLabel("Save Directory:"), 0, 0);
                savePathEdit_->setText(QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory));
                savePathEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
                IOLayout->addWidget(savePathEdit_, 0, 1);
                browsePathButton_->setText("Browse");
                IOLayout->addWidget(browsePathButton_, 0,2);

                IOLayout->addWidget(new QLabel("Image base name:"),1,0);
                imageBaseNameEdit_->setText("Image");
                IOLayout->addWidget(imageBaseNameEdit_, 1,1);
                saveImageButton_->setText("Save Image");
                IOLayout->addWidget(saveImageButton_,1,2);

                IOLayout->addWidget(new QLabel("Video base name:"),3,0);
                videoBaseNameEdit_->setText("Video");
                IOLayout->addWidget(videoBaseNameEdit_,3,1);
                videoRecordButton_->setText("Record");
                IOLayout->addWidget(videoRecordButton_,3,2);

        bottomLayout->addLayout(cameraPropertiesLayout);
        bottomLayout->addLayout(IOLayout);

    mainLayout->addLayout(bottomLayout);
    this->setLayout(mainLayout);

    //Connect widgets to actions
    QObject::connect(exposureValueEdit_,
                     &QLineEdit::textEdited,
                     this,
                     [this](QString value){int newExposure = value.toInt()*1000;
                                           xiSetParamInt(cameraHandle_, XI_PRM_EXPOSURE, newExposure);
                                             //TODO there should be a set Exposure method to avoid code duplication
                                           int ximeaExposure;
                                           xiGetParamInt(cameraHandle_, XI_PRM_EXPOSURE, &ximeaExposure);//ximea uses microseconds
                                           qDebug() << "Camera exposure in microseconds:" << ximeaExposure;
                                           exposure_ = newExposure;
                                           resetTimer_();
                                          });

    QObject::connect(gainValueEdit_,
                     &QLineEdit::textEdited,
                     this,
                     [this](QString value){float newGain = value.toFloat();
                                           xiSetParamFloat(cameraHandle_, XI_PRM_GAIN, newGain);
                                          gain_ = newGain;});

    QObject::connect(browsePathButton_,
                     &QPushButton::pressed,
                     this,
                     [this](){savePathEdit_->setText(QFileDialog::getExistingDirectory(this, "Choose save directory.", savePathEdit_->text()));});

    QObject::connect(saveImageButton_, &QPushButton::pressed, this, [this](){saveNextImage_ = true;});

    QObject::connect(videoRecordButton_, &QPushButton::pressed, this, &MainWindow::toggleVideoRecording);

    QObject::connect(cameraSelector_, &QComboBox::activated, this, [this](int index){disconnectCamera_(); connectCamera_(index);});

    QObject::connect(binningSelector_, &QComboBox::activated, this, [this](int level){setBinning_(level);});

    QObject::connect(cameraTimer_, &QTimer::timeout, this, &MainWindow::updateImage_);

    connectCamera_(0);

    updateImage_();
}

MainWindow::~MainWindow()
{
    disconnectCamera_();
}

void MainWindow::connectCamera_(int index){
    xiOpenDevice(index, &cameraHandle_);
    setParams_();
    XI_RETURN cameraStatus = xiStartAcquisition(cameraHandle_);
    if (cameraStatus != XI_OK) { handleXimeaError_(cameraStatus, "connectCamera");}

    resetTimer_();
}

void MainWindow::disconnectCamera_(){
    xiStopAcquisition(cameraHandle_);
    xiCloseDevice(cameraHandle_);
}


void MainWindow::setParams_(){
//    eventually, I will have this load from file.
    xiSetParamInt(cameraHandle_, XI_PRM_EXPOSURE, exposure_);
    xiSetParamFloat(cameraHandle_, XI_PRM_GAIN, gain_);
    xiSetParamInt(cameraHandle_, XI_PRM_AUTO_WB, autoWB_);
    xiSetParamInt(cameraHandle_, XI_PRM_IMAGE_DATA_FORMAT, XI_RGB32);
//    xiSetParamInt(cameraHandle_, XI_PRM_HORIZONTAL_FLIP, XI_OFF);
//    xiSetParamInt(cameraHandle_, XI_PRM_VERTICAL_FLIP, XI_OFF);
    xiSetParamInt(cameraHandle_, XI_PRM_DOWNSAMPLING_TYPE, XI_BINNING);
    xiSetParamInt(cameraHandle_, XI_PRM_DOWNSAMPLING, binningLevel_);

    exposureValueEdit_->setText(QString::number(exposure_/1000));
    gainValueEdit_->setText(QString::number(gain_));
}

void MainWindow::updateImage_(){
    int deviceConnected = 0;
    xiGetParamInt(cameraHandle_, XI_PRM_IS_DEVICE_EXIST, &deviceConnected);
    if(!deviceConnected){
        if (cameraTimer_->isActive()) cameraTimer_->stop();
        return;
    }

    XI_IMG ximeaImage;
    memset(&ximeaImage,0,sizeof(ximeaImage));
    ximeaImage.size = sizeof(XI_IMG);

    XI_RETURN cameraStatus = xiGetImage(cameraHandle_, 100, &ximeaImage);

    if(cameraStatus != XI_OK){handleXimeaError_(cameraStatus, "updateImage_"); return;}
    auto rawImageData = (unsigned int*)ximeaImage.bp;
    QImage outputImage{int(ximeaImage.width), int(ximeaImage.height), QImage::Format_RGB32};

    for (unsigned int inputLine = 0; inputLine < ximeaImage.height; ++inputLine){
        memcpy(outputImage.scanLine(inputLine),
               &rawImageData[inputLine*ximeaImage.width],
                ximeaImage.width * 4);
    }

    imageLabel_->setImage(outputImage);

    if(videoWriter_){
        cv::Mat outputMat = cv::Mat(cv::Size(ximeaImage.width,
                                             ximeaImage.height),
                                             CV_8UC4,
                                             ximeaImage.bp);
        cv::cvtColor(outputMat, outputMat, cv::COLOR_RGBA2RGB);

        videoWriter_->write(outputMat);
    }

    if(saveNextImage_){
        qDebug() << generateFilePath_(false);
        outputImage.save(generateFilePath_(false));
        saveNextImage_ = false;
    }
}

void MainWindow::handleXimeaError_(XI_RETURN cameraStatus, QString callingMethod){
    //I'll push this out to a gui element.
    qDebug() << "Error " << cameraStatus << " in " << callingMethod << "/n";
}


void MainWindow::toggleVideoRecording(){
    if(!videoWriter_){
        videoRecordButton_->setText("Stop");
        double framerate = 1.0/(double(double(cameraTimer_->interval())/1000.0));//returning 50, should be 5
        qDebug()<< "Framerate set to:" << framerate;
        videoWriter_ = new cv::VideoWriter();

        int width, height;
        xiGetParamInt(cameraHandle_, XI_PRM_WIDTH, &width);
        xiGetParamInt(cameraHandle_, XI_PRM_HEIGHT, &height);

        videoWriter_->open(generateFilePath_(true).toStdString(),
                                    cv::VideoWriter::fourcc('M', 'P', '4', 'V'),
                                    framerate,
                                    cv::Size(width, height));
    }
    else{
        videoRecordButton_->setText("Record");
        videoWriter_->release();
        delete videoWriter_;
        videoWriter_ = nullptr;
    }
}

QString MainWindow::generateFilePath_(bool isVideo){
    QDir directory(savePathEdit_->text());
    QString filename = isVideo ? videoBaseNameEdit_->text() : imageBaseNameEdit_->text();
    QFileInfo fileInfo(directory, filename);

    if (fileInfo.suffix() =="" && !isVideo) fileInfo.setFile(fileInfo.absoluteFilePath() + ".png");
    if (isVideo) fileInfo.setFile(fileInfo.absoluteFilePath() + ".mp4");

    if(std::filesystem::exists(fileInfo.absoluteFilePath().toStdString())){
        int fileIndex = 0;
        while(true){
            fileIndex++;
            QFileInfo incrementedFileInfo(directory, (fileInfo.baseName() + "_" + "%1" + "." + fileInfo.suffix()).arg(fileIndex));
            if(!std::filesystem::exists(incrementedFileInfo.absoluteFilePath().toStdString())){
                fileInfo = incrementedFileInfo;
                break;
            }
        }
    }
    return fileInfo.absoluteFilePath();
}

QStringList MainWindow::enumerateCameras_(){
    QStringList cameraList;
    DWORD numberDevices;
    xiGetNumberDevices(&numberDevices);
    char serialNumber[256];

    for (unsigned int deviceIndex = 0; deviceIndex < numberDevices; deviceIndex++){
        QString cameraString = QString::number(deviceIndex)+":";
        xiGetDeviceInfoString(deviceIndex, XI_PRM_DEVICE_SN, serialNumber, 256);
        cameraList.append(cameraString.append(serialNumber));
    }
    return cameraList;
}

void MainWindow::setBinning_(int binningLevel){
    binningLevel_ = binningLevel+1;//binningSelector is zero indexed, binning on camera is not
    disconnectCamera_();
    connectCamera_(cameraSelector_->currentIndex());
}

void MainWindow::resetTimer_(){
    if (cameraTimer_->isActive()) cameraTimer_->stop();
    cameraTimer_->setInterval(exposure_/1000);//microseconds to milliseconds
    cameraTimer_->start();
    qDebug() << "Camera timer interval in milliseconds:" << cameraTimer_->interval();
}

