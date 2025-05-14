#include "qwwindowwidget.h"

QWWindowWidget::QWWindowWidget(QWidget *m_mainPage) :
    QWidget(m_mainPage)
{
  BackgroundColor.setRgb(85, 85, 127);
  update();
}
void QWWindowWidget::resizeEvent(QResizeEvent* event){
  QWidget::resizeEvent(event);
  update();
}

void QWWindowWidget::setBorderRadius(int value){
  BorderRadius = value;
  update();
}

int QWWindowWidget::getBorderRadius(){
  return BorderRadius;
}

void QWWindowWidget::setBackgroundColor(const QColor &color){
  BackgroundColor = color;
  update();
}

QColor QWWindowWidget::getBackgroundColor()
{
    return BackgroundColor;
}

void QWWindowWidget::paintEvent(QPaintEvent* event){
  Q_UNUSED(event)

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QPainterPath path;
  path.addRoundedRect(QRectF(0, 0, width(), height()), BorderRadius, BorderRadius);

  painter.setPen(Qt::NoPen);
  painter.setBrush(BackgroundColor);
  painter.drawPath(path);
}
