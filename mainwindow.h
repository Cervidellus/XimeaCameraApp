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

    void saveImage();
    void toggleVideoRecording();

private:
    ImageLabel* imageLabel_;
    QComboBox* cameraSelector_;
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
    void connectCamera_();
    void handleXimeaError_(XI_RETURN cameraStatus, QString callingMethod);
    void setDefaultParams_();
    void updateImage_();
    QString generateFilePath_(bool isVideo = false);
    QStringList enumerateCameras_();

    HANDLE cameraHandle_ = nullptr;
    QTimer* cameraTimer_;
    int cameraExposure_;

    //default parameters
    const int defaultExposure_ = 20000;//microseconds
    const float defaultGain_ = 0;
    const int defaultAutoWB_ = XI_ON;

};
#endif // MAINWINDOW_H
