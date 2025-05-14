#include "qwrocker.h"

QWRocker::QWRocker(QWidget *m_mainPage) :
    QWidget(m_mainPage)
{

    RoundCakeColor.setRgb(150,200,150);
    BackgroundColor.setRgb(100, 150, 150);

    OriginX = width()/2;
    OriginY = height()/2;

    font.setFamily("\347\255\211\347\272\277");
    font.setPointSize(10);
}

// 绘制开关
void QWRocker::paintEvent(QPaintEvent* event) 
{
    QPainter painter(this);
    QColor bgColor;

    painter.setRenderHint(QPainter::Antialiasing);

    //画背景
    painter.setPen(QPen(BackgroundColor, BackgroundBorderWidth,Qt::SolidLine));
    bgColor = BackgroundColor;
    bgColor.setAlphaF(0.2f);
    painter.setBrush(QBrush(bgColor));
    painter.drawEllipse(OriginX - BackgroundRadius, OriginY - BackgroundRadius, BackgroundRadius*2, BackgroundRadius*2);

    //画线
    QColor color(BackgroundColor);
    color.setAlpha(100);
    painter.setPen(QPen(color, RoundCakeRadius/2, Qt::SolidLine,Qt::PenCapStyle::RoundCap));
    painter.drawLine(OriginX,OriginY,OriginX  - CurrentDeviationX,OriginY - CurrentDeviationY);

    //画中心点
    painter.setPen(QPen(BackgroundColor, RoundCakeBorderWidth, Qt::SolidLine));
    painter.setBrush(QBrush(BackgroundColor));
    painter.drawEllipse(OriginX - 40, OriginY - 40, 80, 80);


    //画饼
    painter.setPen(QPen(RoundCakeColor, RoundCakeBorderWidth, Qt::SolidLine));
    bgColor = RoundCakeColor;
    bgColor.setAlpha(220);
    painter.setBrush(QBrush(bgColor));
    painter.drawEllipse(OriginX - RoundCakeRadius - CurrentDeviationX, OriginY - RoundCakeRadius- CurrentDeviationY, RoundCakeRadius * 2, RoundCakeRadius * 2);

    //显示信息
    if(ShowNumericalDisplay)
    {
        painter.setFont(font);
        painter.setPen(QColor(255,255,255));
        //画X值
        painter.drawText(QRect(OriginX - 40 - 100,20,100,30),Qt::AlignHCenter | Qt::AlignVCenter,"Y: " + QString::number(CurrentDeviationY));

        //画Y值
        painter.drawText(QRect(OriginX + 40,      20,100,30),Qt::AlignHCenter | Qt::AlignVCenter,"X: " + QString::number(-CurrentDeviationX));
    }

    //测试
    painter.drawText(QPoint(50,50),testStr);
}

void QWRocker::PointPress(QPoint pos)
{
    isPress = true;

    pressPoint = pos;

}
void QWRocker::PointMoving(QPoint pos)
{
    if (isPress)
    {
        float angle = 0;
        float VectorLength = 0;

        MovePoint = pos;

        QPoint tempPoint = MovePoint - pressPoint;

        //得出当前角度
        angle = atan2(-tempPoint.y(), tempPoint.x());
        //得出当前向量长度
        VectorLength = sqrtf(tempPoint.x() * tempPoint.x() + tempPoint.y() * tempPoint.y());

        //限制范围
        if (VectorLength > BackgroundRadius)
        {
            VectorLength = BackgroundRadius;
        }

        //求出当前值
        CurrentDeviationX = -cos(angle) * VectorLength;
        CurrentDeviationY = sin(angle) * VectorLength;

        emit Moved(-CurrentDeviationX, CurrentDeviationY);

        update();
    }
}
void QWRocker::PointRelease(QPoint pos)
{
    isPress = false;

    CurrentDeviationX = 0;
    CurrentDeviationY = 0;

    update();

    emit Moved(-CurrentDeviationX, CurrentDeviationY);
}

// 鼠标按下事件
void QWRocker::mousePressEvent(QMouseEvent* event)
{
    PointPress(cursor().pos());
}

// 鼠标释放事件 - 切换开关状态、发射toggled()信号
void QWRocker::mouseMoveEvent(QMouseEvent* event)
{
    PointMoving(cursor().pos());
}

void QWRocker::mouseReleaseEvent(QMouseEvent* event)
{
    PointRelease(cursor().pos());
}
// 大小改变事件
void QWRocker::resizeEvent(QResizeEvent* event)
{
    OriginX = width()/2;
    OriginY = height()/2;

}

bool QWRocker::setTouchEvent(QPoint pos,Qt::TouchPointState state)
{
    QPoint currentTouchPos = mapFromParent(pos);

    int radius  = sqrt( (currentTouchPos.x() - OriginX) * (currentTouchPos.x() - OriginX) +
                        (currentTouchPos.y() - OriginY) * (currentTouchPos.y() -OriginY ) );

    switch (state)
    {
    case Qt::TouchPointState::TouchPointPressed:
        {
            //起始点必须在 饼上，才能被识别，圆得公式
            if(radius < RoundCakeRadius)
            {
                PointPress(pos);

                return true;
            }
        }
        break;

    case Qt::TouchPointState::TouchPointMoved:
        {
            PointMoving(pos);
            return false;
        }
        break;

    case Qt::TouchPointState::TouchPointReleased:
        {
            bool ok = false;
            if(isPress)
            {
                ok = true;
            }

            PointRelease(pos);

            return ok;
        }
        break;
    }

    return false;
}


void QWRocker::setMaxValueX(int v)
{
    MaxValueX = v;

    update();
}

void QWRocker::setMinValueX(int v)
{
    MinValueX = v;
    update();
}

int QWRocker::getMaxValueX()
{
    return MaxValueX;
}

int QWRocker::getMinValueX()
{
    return MinValueX;
}

void QWRocker::setMaxValueY(int v)
{
    MaxValueY = v;
    update();
}

void QWRocker::setMinValueY(int v)
{
    MinValueY = v;
    update();
}

int QWRocker::getMaxValueY()
{
    return MaxValueY;
}

int QWRocker::getMinValueY()
{
    return MinValueY;
}

void QWRocker::setBackgroundBorderWidth(int v)
{
    BackgroundBorderWidth = v;
}

int QWRocker::getBackgroundBorderWidth()
{
    return BackgroundBorderWidth;
}

void QWRocker::setRoundCakeBorderWidth(int v)
{
    RoundCakeBorderWidth = v;
}

int QWRocker::getRoundCakeBorderWidth()
{
    return RoundCakeBorderWidth;
}

void QWRocker::setBackgroundRadius(int v)
{
    BackgroundRadius = v;
}

int QWRocker::getBackgroundRadius()
{
    return BackgroundRadius;
}

void QWRocker::setRoundCakeRadius(int v)
{
    RoundCakeRadius = v;
}

int QWRocker::getRoundCakeRadius()
{
    return RoundCakeRadius;
}

void QWRocker::setBackgroundColor(const QColor& v)
{
    BackgroundColor = v;
}

QColor QWRocker::getBackgroundColor()
{
    return BackgroundColor;
}

void QWRocker::setRoundCakeColor(const QColor& v)
{
    RoundCakeColor = v;
}

QColor QWRocker::getRoundCakeColor()
{
    return RoundCakeColor;
}
void QWRocker::setShowNumericalDisplay(bool ok)
{
    ShowNumericalDisplay = ok;
}
bool QWRocker::getShowNumericalDisplay()
{
    return ShowNumericalDisplay;
}
