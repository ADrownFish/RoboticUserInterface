#ifndef QWBUTTON_H
#define QWBUTTON_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QSvgRenderer>

class QWButton : public QWidget{
    Q_OBJECT
    //Q_PROPERTY(QColor CheckedColor READ getCheckedColor WRITE setCheckedColor)
    //Q_PROPERTY(QColor UnCheckedColor READ getUnCheckedColor WRITE setUnCheckedColor)

    //Q_PROPERTY(QString ICO READ getICO WRITE setICO)
    //Q_PROPERTY(QSize PixmapSize READ getPixmapSize WRITE setPixmapSize)
    //Q_PROPERTY(bool Checked READ getChecked WRITE setChecked)
    //Q_PROPERTY(bool Unchecked READ getUnChecked WRITE setUnchecked)

public:
    QWButton(QWidget *m_mainPage = 0);

public slots:
    void setCheckedColor(const QColor& color);
    void setUnCheckedColor(const QColor& color);
    QColor getCheckedColor();
    QColor getUnCheckedColor();
    void setSvg(const QString &res);
    void setSvgSize(const QSize &size);
    QSize getSvgSize(); 

    void setChecked(bool ok);
    bool getChecked();

    void setUnchecked(bool ok);
    bool getUnChecked();
protected:
    // 绘制开关
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    // 大小改变事件
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    // 鼠标按下事件
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    // 鼠标释放事件 - 切换开关状态、发射toggled()信号
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
signals:
    void toggled(bool ok);

private:
    // 状态切换时，用于产生滑动效果
    void onTimeout();

    unsigned char Transformationtimes;

    bool mChecked;         // 是否选中
    bool isEnableSvg;

    QColor CheckedColor;
    QColor UnCheckedColor;
    QColor nowColor;
    QTimer m_timer;
    QSvgRenderer svg;
    QSize svgSize;

    unsigned char tempNum = 0;
};

#endif
