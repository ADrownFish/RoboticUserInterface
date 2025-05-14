#ifndef QWSWITCHER_H
#define QWSWITCHER_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>

class QWSwitcher : public QWidget
{
    Q_OBJECT
        Q_PROPERTY(bool Checked  READ isToggled WRITE setToggle)
        Q_PROPERTY(QColor BackgroundColor READ getBackgroundColor WRITE setBackgroundColor)
        Q_PROPERTY(QColor CheckedBallColor READ getCheckedBallColor WRITE setCheckedBallColor)
        Q_PROPERTY(QColor CheckBallColor READ getCheckBallColor WRITE setCheckBallColor)
        Q_PROPERTY(int Pen READ getPenWidth WRITE setPenWidth)
        Q_PROPERTY(int Width READ getWidth WRITE setWidth)
        Q_PROPERTY(QColor BorderColor READ getBorderColor WRITE setBorderColor)
        Q_PROPERTY(QColor CheckedBackgroundColor READ getBackCheCkedgroundColor WRITE setBackCheCkedgroundColor)

public:
    QWSwitcher(QWidget* parent = 0);

    // 开关状态
    bool isToggled() const;
    void setToggle(bool checked);
    // 背景颜色
    void setBackgroundColor(const QColor& color);
    QColor getBackgroundColor()const;
    // 设置选中颜色
    void setCheckedBallColor(const QColor& color);
    QColor getCheckedBallColor()const;
    // 设置未选中颜色
    void setCheckBallColor(const QColor& color);
    QColor getCheckBallColor()const;
    //设置笔宽
    void setPenWidth(int pen);
    int getPenWidth()const;
    //设置大小
    void setWidth(int width);
    int getWidth()const;
    //设置边框颜色
    void setBorderColor(const QColor& color);
    QColor getBorderColor();
    //设置选中背景颜色
    void setBackCheCkedgroundColor(const QColor& color);
    QColor getBackCheCkedgroundColor();

protected:
    // 绘制开关
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    // 鼠标按下事件
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    // 鼠标释放事件 - 切换开关状态、发射toggled()信号
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    // 大小改变事件
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

signals:
    // 状态改变时，发射信号
    void toggled(bool checked);

private:
    // 状态切换时，用于产生滑动效果
    void onTimeout();

    void updateCheckedBallPos();

private:
    bool mChecked;         // 是否选中

    QColor m_BorderColor;                   //边框颜色
    QColor m_Background;                     // 背景颜色
    QColor m_CheckedBackground;                     // 选中背景颜色
    QColor m_CheckedBall;                   // 选中颜色
    QColor m_CheckBall;                       // 未选中颜色
    int m_Radius;          // 圆半径
    int m_FinalX;              // 最终要移动到的点x坐标
    float m_NowX;             //现在的x坐标
    int m_penWidth;
    QTimer m_timer;          // 定时器

    int m_Radius_pos = 0;
    QRect border_rect;
};

#endif
