#include "imagelabel.h"

ImageLabel::ImageLabel()
{

}

void ImageLabel::setImage(QImage image){
    currentImage_ = image;
}

void ImageLabel::paintEvent(QPaintEvent* event){

    if (currentImage_.isNull()) return;

    QSize labelSize = this->size();

    QPixmap newPixmap =QPixmap::fromImage(currentImage_);

    QPixmap newPixmapScaled = newPixmap.scaled(labelSize, Qt::KeepAspectRatio);

    this->setPixmap(newPixmapScaled);

    QLabel::paintEvent(event);
};


