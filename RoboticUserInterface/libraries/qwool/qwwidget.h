#ifndef QWWIDGET_H
#define QWWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPainterPath>


class QWWidget : public QWidget
{
    Q_OBJECT

        Q_PROPERTY(int BorderRadius READ getBorderRadius WRITE setBorderRadius)
        Q_PROPERTY(QColor BackgroundColor READ getBackgroundColor WRITE setBackgroundColor)

public:
    QWWidget(QWidget *m_mainPage = 0);


    void setBorderRadius(int value);
    int getBorderRadius();

    void setBackgroundColor(const QColor &color);
    QColor getBackgroundColor();

private:
    void makeImage();



protected:
    // 绘制
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    // 大小改变事件
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;


private:
    QPixmap Pixmap;

    int BorderRadius = 20;
    QColor BackgroundColor;
};

#endif
