#ifndef QWROCKER_H
#define QWROCKER_H

#include <QWidget>
#include <QPainter>
#include <math.h>

class QWRocker : public QWidget
{
    Q_OBJECT

public:
    QWRocker(QWidget *m_mainPage = 0);


protected:
    // 绘制开关
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    // 鼠标按下事件
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    // 鼠标释放事件 - 切换开关状态、发射toggled()信号
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

    void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    // 大小改变事件
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;


signals:
    // 状态改变时，发射信号
    void Moved( int x, int y);


public:
    //最大值 X
    void setMaxValueX(int v);
    int getMaxValueX();

    //最小值 X
    void setMinValueX(int v);
    int getMinValueX();

    //最大值 Y
    void setMaxValueY(int v);
    int getMaxValueY();

    //最小值 Y
    void setMinValueY(int v);
    int getMinValueY();

    //背景边界笔宽
    void setBackgroundBorderWidth(int v);
    int getBackgroundBorderWidth();

    //圆饼笔宽
    void setRoundCakeBorderWidth(int v);  
    int getRoundCakeBorderWidth();

    //背景边界半径
    void setBackgroundRadius(int v);
    int getBackgroundRadius();

    //圆饼半径
    void setRoundCakeRadius(int v);
    int getRoundCakeRadius();

    //边界颜色
    void setBackgroundColor(const QColor& v);
    QColor getBackgroundColor();

    //圆饼颜色
    void setRoundCakeColor(const QColor &v);
    QColor getRoundCakeColor();

    //设置触摸，坐标必须和本控件在同一坐标系，
    bool setTouchEvent(QPoint pos,Qt::TouchPointState state);

    //显示数值在界面
    void setShowNumericalDisplay(bool ok);
    bool getShowNumericalDisplay();

private:
    void PointPress(QPoint pos);
    void PointMoving(QPoint pos);
    void PointRelease(QPoint pos);

private:

    QString testStr;

    bool ShowNumericalDisplay = false;

    QFont font;

    QColor RoundCakeColor;
    QColor BackgroundColor;

    QPoint pressPoint;
    QPoint MovePoint;

    bool isPress = false;

    int OriginX = 300;
    int OriginY = 300;

    int CurrentDeviationX = 0;
    int CurrentDeviationY = 0;

    int MaxValueX = 100;
    int MinValueX = -100;
    int MaxValueY = 100;
    int MinValueY = -100;

    int BackgroundRadius = 200;
    int RoundCakeRadius = 100;

    int BackgroundBorderWidth = 4;
    int RoundCakeBorderWidth = 4;
};

#endif
