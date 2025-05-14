#include "qwwidget.h"

QWWidget::QWWidget(QWidget *m_mainPage) :
    QWidget(m_mainPage)
{
    BackgroundColor.setRgb(85, 85, 127);

    makeImage();
}
void QWWidget::resizeEvent(QResizeEvent* event)
{
    makeImage();

    QWidget::resizeEvent(event);
}

void QWWidget::setBorderRadius(int value)
{
    BorderRadius = value;

    makeImage();
}

int QWWidget::getBorderRadius()
{
    return BorderRadius;
}

void QWWidget::setBackgroundColor(const QColor &color)
{
    BackgroundColor = color;

    makeImage();
}

QColor QWWidget::getBackgroundColor()
{
    return BackgroundColor;
}

void QWWidget::makeImage()
{
    Pixmap = QPixmap(this->size());
    Pixmap.fill(QColor(0, 0, 0, 0));

    QPainter painter(&Pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(QRectF(0, 0, width(), height()), BorderRadius, BorderRadius);

    //画背景

    //painter.setPen(QPen(QColor(255,255,255,150),2));
    painter.setPen(Qt::NoPen);
    painter.setBrush(BackgroundColor);
    painter.drawPath(path);

    update();
}

void QWWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.drawPixmap(0, 0, Pixmap);


}
