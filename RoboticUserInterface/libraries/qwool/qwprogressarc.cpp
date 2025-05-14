#include "qwprogressarc.h"

QWProgressArc::QWProgressArc(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(10, 10);
    resize(100, 100);

    m_MaxValue = 300;
    m_TargetValue = 150;
    m_Value = 150;
    m_PenWidth = 8;
    m_ArcBGColor.setRgb(0,120,200);
    m_ArcValueColor.setRgb(4, 186, 189);

    // 连接信号槽
    connect(&m_timer, &QTimer::timeout, this, &QWProgressArc::onTimeout);
    update();
}

// 绘制开关
void QWProgressArc::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (!isEnabled())
        // 可用状态
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制大圆弧
    short startstep = (90 - m_MaxValue / 2);
    short endstep = m_MaxValue;

    painter.setPen(QPen(m_ArcBGColor,m_PenWidth,Qt::SolidLine,Qt::RoundCap));
    painter.drawArc(m_PenWidth, m_PenWidth,
        width() - m_PenWidth * 2,height()- m_PenWidth *2, startstep * 16, endstep * 16); //绘画角度

    //绘制实际值弧
    painter.setPen(QPen(m_ArcValueColor, m_PenWidth, Qt::SolidLine, Qt::RoundCap));

    startstep = startstep + endstep - m_Value;
    endstep = m_Value;

    painter.drawArc(m_PenWidth, m_PenWidth,
        width() - m_PenWidth * 2, height() - m_PenWidth * 2, startstep * 16, endstep  * 16); //绘画角度

    QFont f = QFont(this->font());
    painter.setFont(f);
    painter.setPen(m_TextColor);
    int vvalue = ((float)m_Value / (float)m_MaxValue) * 100;
    painter.drawText(0, height()/15, width(), height(), Qt::AlignCenter, QString::number(vvalue)+"%");
}

// 大小改变事件
void QWProgressArc::resizeEvent(QResizeEvent *event)
{

    QWidget::resizeEvent(event);
}

// 切换状态 - 滑动
void QWProgressArc::onTimeout()
{
    if (m_Value < m_TargetValue)
    {
        m_Value += 6;
        if(m_Value > m_TargetValue)
            m_Value = m_TargetValue;
    }
    else if(m_Value > m_TargetValue)
    {
        m_Value -= 6;
        if(m_Value < m_TargetValue)
            m_Value = m_TargetValue;
    }
    else
    {
        m_timer.stop();
    }
    update();
}

void QWProgressArc::setMaxValue(int value)
{
    m_MaxValue = value;
    update();
}

int QWProgressArc::getMaxValue()
{
    return m_MaxValue;
}

void QWProgressArc::setValue(int value)
{
    if (value > m_MaxValue)
        m_TargetValue = m_MaxValue;
    else if(value < 0)
        value = 0;
    else
        m_TargetValue = value;

    m_timer.start(4);
}

int QWProgressArc::getValue()
{
    return m_TargetValue;
}

void QWProgressArc::setPenWidth(int value)
{
    m_PenWidth = value;
    update();
}

int QWProgressArc::getPenWidth()
{
    return m_PenWidth;
}

void QWProgressArc::setArcColor(const QColor & value)
{
    m_ArcBGColor = value;
    update();
}

QColor QWProgressArc::getArcColor()
{
    return m_ArcBGColor;
}

void QWProgressArc::setArcValueColor(const QColor & value)
{
    m_ArcValueColor = value;
    update();
}

QColor QWProgressArc::getArcValueColor()
{
    return m_ArcValueColor;
}

void QWProgressArc::setTextColor(const QColor & value)
{
    m_TextColor = value;
    update();
}

QColor QWProgressArc::getTextColor()
{
    return m_TextColor;
}

void QWProgressArc::setPercentColor(const QColor & value)
{
    m_PercentColor = value;
    update();
}

QColor QWProgressArc::getPercentColor()
{
    return m_PercentColor;
}
