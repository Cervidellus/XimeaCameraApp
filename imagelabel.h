#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>

class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    ImageLabel();

    void setImage(QImage image);

protected:
     void paintEvent(QPaintEvent* event) override;

private:
    QImage currentImage_;
};

#endif // IMAGELABEL_H
