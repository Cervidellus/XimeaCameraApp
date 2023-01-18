#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QString>
#include <xiApi.h>


class QFileDialog;
class ImageLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QTimer;
namespace cv{
class VideoWriter;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void toggleVideoRecording();

private:
    ImageLabel* imageLabel_;
    QComboBox* cameraSelector_;
    QComboBox* binningSelector_;
    QLineEdit* exposureValueEdit_;
    QLineEdit* gainValueEdit_;
    QLineEdit* savePathEdit_;
    QLineEdit* imageBaseNameEdit_;
    QLineEdit* videoBaseNameEdit_;
    QPushButton* browsePathButton_;
    QPushButton* saveImageButton_;
    QPushButton* videoRecordButton_;

    cv::VideoWriter* videoWriter_ = nullptr;

    //Connect the first available camera.
    //GUI will allow you to select another, and will restrict you to only those that are available.
    void connectCamera_(int index);
    void disconnectCamera_();
    void handleXimeaError_(XI_RETURN cameraStatus, QString callingMethod);
    void setParams_();
    void setBinning_(int binningLevel);
    void updateImage_();
    QString generateFilePath_(bool isVideo = false);
    QStringList enumerateCameras_();

    HANDLE cameraHandle_ = nullptr;
    QTimer* cameraTimer_;
    int cameraExposure_;
    bool saveNextImage_ = false;

    //default parameters
    int exposure_ = 20000;//microseconds
    float gain_ = 0;
    int autoWB_ = XI_ON;
    int binningLevel_ = 1;

};
#endif // MAINWINDOW_H
