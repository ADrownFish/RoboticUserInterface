#include "qwcharts.h"

QWCharts::QWCharts(QWidget *parent)
    : QWidget(parent)
{

    resize(300, 200);

    m_BorderColor.setRgb(0,120,255);
    m_Background.setRgb(17, 125, 187,50);
    m_PointColor.setRgb(17, 125, 187);

    m_PenWidth = 2;

    m_MaxValue = 300;
    m_MinValue = -300;

    ValueInterval = 5;
    CellWidth = 80;
    CellHeight = 20;

    maxPointSize = 2000;

    m_ShowBG = false;
    m_ShowBorder = false;
    m_FillCurve = false;
    m_showLabel = false;

    updateImage();

}

QWCharts::~QWCharts()
{

}

// 绘制开关
void QWCharts::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, Pixmap);

}

// 大小改变事件
void QWCharts::resizeEvent(QResizeEvent *event)
{
    updateImage();

    QWidget::resizeEvent(event);
}

void QWCharts::appendValue(int value)
{
    int Xvalue = 0;

    while (PointList.size() > maxPointSize)
    {
        PointList.removeFirst();
        ++Xvalue;
    }

    for (int i = 0; i < PointList.size(); ++i)
    {
        PointList[i].setX(PointList[i].rx() - ValueInterval * Xvalue);
    }

    PointList.append(QPoint(PointList.size() * ValueInterval, -value));

    updateImage();
}

int QWCharts::getValue()
{
    return 0;
}

void QWCharts::appendPoint(QPoint point)
{
    PointList.append( point);


    updateImage();
}

void QWCharts::setBorderColor(const QColor & value)
{
    m_BorderColor = value;
    updateImage();
}

void QWCharts::setBackground(const QColor & value)
{
    m_Background = value;
    updateImage();
}

void QWCharts::setPointColor(const QColor & value)
{
    m_PointColor = value;
    updateImage();
}

void QWCharts::setPenWidth(float value)
{
    m_PenWidth = value;
    updateImage();
}

void QWCharts::setMaxValue(float value)
{
    m_MaxValue = value;
    updateImage();
}

void QWCharts::setMinValue(float value)
{
    m_MinValue = value;
    updateImage();
}

void QWCharts::setAxisXValueInterval(int value)
{
    ValueInterval = value;
    PointList.clear();

    updateImage();
}

void QWCharts::setShowBorder(bool value)
{
    m_ShowBorder = value;
    updateImage();
}

void QWCharts::setShowBackground(bool value)
{
    m_ShowBG = value;
    updateImage();
}
QPoint QWCharts::getNewPoint()
{
    return QPoint(0,0);
}

QColor QWCharts::getBorderColor() const
{
    return m_BorderColor;
}

QColor QWCharts::getBackground() const
{
    return m_Background;
}

QColor QWCharts::getPointColor() const
{
    return m_PointColor;
}

float QWCharts::getPenWidth()
{
    return m_PenWidth;
}

float QWCharts::getMaxValue()
{
    return m_MaxValue;
}

float QWCharts::getMinValue()
{
    return m_MinValue;
}

int QWCharts::getAxisXValueInterval()
{
    return ValueInterval;
}

void QWCharts::setCellWidth(int value)
{
    CellWidth = value;

    updateImage();
}

int QWCharts::getCellWidth()
{
    return CellWidth;
}

void QWCharts::setCellHeight(int value)
{
    CellHeight = value;

    updateImage();
}

int QWCharts::getCellHeight()
{
    return CellHeight;
}

void QWCharts::setFillCurve(bool value)
{
    m_FillCurve = value;

    updateImage();
}

bool QWCharts::getFillCurve()
{
    return m_FillCurve;
}

void QWCharts::setShowLabel(bool value)
{
    m_showLabel = value;

    updateImage();
}

bool QWCharts::getShowLabel()
{
    return m_showLabel;
}

void QWCharts::setMaxBufferPointCount(int value)
{
    maxPointSize = value;
}

int QWCharts::getMaxBufferPointCount()
{
    return maxPointSize;
}

void QWCharts::updateImage()
{

    if (!isEnabled())
        return;

    makeImage();

    update();
}

void QWCharts::makeImage()
{
    //计算画布
    float mapHeight = m_MaxValue - m_MinValue;
    //计算零点
    float zeroLine = 0;
    zeroLine = ((mapHeight + m_MinValue) / mapHeight)*height();

    float YAxisScale = mapHeight / height();

    //画图
    Pixmap = QPixmap(this->size());
    Pixmap.fill(QColor(0, 0, 0, 0));

    QPainter painter(&Pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    //绘制边框、背景
    if (m_ShowBG)
    {
        painter.setPen(QPen(QColor(160, 160, 100),1,Qt::SolidLine,Qt::PenCapStyle::RoundCap));
        painter.setBrush(Qt::NoBrush);

        int tempX = width();
        int tempY = zeroLine;
        while (true)
        {
            painter.drawLine(tempX, tempY, 0, tempY);
            tempY -= CellHeight;

            if (tempY < 0)
                break;
        }

        tempY = zeroLine;
        while (true)
        {
            painter.drawLine(tempX, tempY, 0, tempY);
            tempY += CellHeight;

            if (tempY > height())
                break;
        }

        tempY = height();
        while (true)
        {
            painter.drawLine(tempX, 0, tempX, tempY);
            tempX -= CellWidth;

            if (tempX < 0)
                break;
        }
    }

    //是否显示标签
    if (m_showLabel)
    {
        QFont thisFont(font());

        int fontHeightError = thisFont.pixelSize() /3.2 ;
        painter.setFont(thisFont);
        painter.setPen(QPen(QColor(255, 255, 255), 1));

        for (int i = 0; i < zeroLine; i += CellHeight)
        {
            int y = zeroLine - i;
            painter.drawText(QPoint(8,y+ fontHeightError), QString::number((i)* YAxisScale, 'f', 1));

            painter.drawLine(0,y,5,y);
        }

        for (int i = zeroLine + CellHeight; i < height(); i += CellHeight)
        {
            painter.drawText(QPoint(8, i+ fontHeightError), QString::number((zeroLine - i )* YAxisScale, 'f', 1));
            painter.drawLine(0, i, 5, i);
        }
    }

    //画零线
    painter.setPen(QPen(m_BorderColor,1));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(0, zeroLine, width(), zeroLine);

    //绘制曲线起始点
    QPainterPath  path(QPointF(width(), zeroLine));
    if (PointList.isEmpty())
    {
        path.lineTo(
             width() - (ValueInterval * PointList.size() + 1),
            zeroLine
        );
    }
    else
    {
        path.lineTo(
            PointList.first().x() + width() - (ValueInterval * PointList.size() + 1),
            zeroLine
        );
    }

    //绘制路径
    for (int i = 0; i < PointList.size(); ++i)
    {
        int tempX = PointList.at(i).x() + width() - (ValueInterval * (PointList.size() - 1));
        int tempY = PointList.at(i).y() / YAxisScale + zeroLine;

        QPoint point;
        point.setY(tempY);
        point.setX(tempX);

        //大于-间隔值才可以被显示
        if (tempX >= -(ValueInterval))
        {
            path.lineTo(point);
        }
    }

    //是否填充曲线
    if (m_FillCurve)
        painter.setBrush(m_Background);
    else
        painter.setBrush(Qt::NoBrush);

    painter.setPen(QPen(m_PointColor, m_PenWidth / 2, Qt::SolidLine, Qt::RoundCap));
    painter.drawPath(path);

    if (m_ShowBorder)
    {
        painter.setPen(QPen(m_BorderColor, m_PenWidth, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(0, 0, width(), height());
    }
}

bool QWCharts::getShowBorder()
{
    return m_ShowBorder;
}

bool QWCharts::getShowBackground()
{
    return m_ShowBG;
}
