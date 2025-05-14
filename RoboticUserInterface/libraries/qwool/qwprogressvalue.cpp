#include "qwprogressvalue.h"

QWProgressValue::QWProgressValue(QWidget* parent) :
    QWidget(parent),
    m_MaxValue(100),
    m_Value(0),
    m_currentValue(m_Value),

    backgroundColor(80,80,80),
    foregroundColor(4, 139, 143),
    textColor(255,255,255),

    showProgressText(false)
{
    connect(&timer,&QTimer::timeout,this,&QWProgressValue::onTimerout);
}
void QWProgressValue::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.drawPixmap(0, 0, Pixmap);
}

void QWProgressValue::onTimerout()
{
    if(m_currentValue < m_Value)
    {      
        m_currentValue += m_MaxValue / 30.f ;
        if(m_currentValue > m_Value)
        {
            m_currentValue = m_Value;
            timer.stop();
        }
    }
    else if(m_currentValue > m_Value)
    {
        m_currentValue -= m_MaxValue / 30 ;
        if(m_currentValue < m_Value)
        {
            m_currentValue = m_Value;
            timer.stop();
        }
    }
    else {
        timer.stop();
    }

    makeImage();
}

void QWProgressValue::makeImage()
{
    float percentage_make = (m_currentValue / (float)m_MaxValue);

    Pixmap = QPixmap(this->size());
    Pixmap.fill(QColor(0, 0, 0, 0));

    QPainter p(&Pixmap);

    p.setPen((Qt::NoPen));
    p.setRenderHint(QPainter::Antialiasing);

    p.setBrush(QBrush(backgroundColor));
    p.drawRoundedRect(0, 0, width(), height(), 0, 0);

    p.setBrush(QBrush(foregroundColor));
    p.drawRoundedRect(0, 0, percentage_make * width(), height(), 0, 0);

    QRect textRect(0, 0, width(), height());
    p.setPen(textColor);
    if (showProgressText)
    {
        p.drawText(textRect, Qt::AlignCenter, text + QString(" %1 %").arg(static_cast<int>(percentage_make * 100)));
    }
    else {
        p.drawText(textRect, Qt::AlignCenter, text);
    }

    update();
}

float QWProgressValue::getMaxValue()
{
    return m_MaxValue;
}
void QWProgressValue::setMaxValue(float value)
{
    if(value <= 0)
        value = 0;

    m_MaxValue = value;

    makeImage();
}
void QWProgressValue::setValue(float value)
{
   if(EnableAnimation)
    {
        if (value > m_MaxValue)
            m_Value = m_MaxValue;

        if(m_Value == value)
            return;

        m_Value = value;
        timer.start(20);

    }
    else
   {
       if (value > m_MaxValue)
           m_currentValue = m_MaxValue;

        m_currentValue = value;

        makeImage();
    }
}
float QWProgressValue::getValue()
{
    return m_Value;
}

void QWProgressValue::setForegroundColor(const QColor &color)
{
    foregroundColor = color;

    makeImage();
}

QColor QWProgressValue::getForegroundColor()
{
    return foregroundColor;
}

void QWProgressValue::setBackgroundColor(const QColor& color)
{
    backgroundColor = color;

    makeImage();
}

QColor QWProgressValue::getBackgroundColor()
{
    return backgroundColor;
}

void QWProgressValue::setText(const QString &str)
{
    text = str;

    makeImage();
}

QString QWProgressValue::getText()
{
    return text;
}

void QWProgressValue::setTextColor(const QColor &color)
{
    textColor = color;

    makeImage();
}

QColor QWProgressValue::getTextColor()
{
    return textColor;
}

void QWProgressValue::setShowProgress(bool ok)
{
    showProgressText = ok;

    makeImage();
}

bool QWProgressValue::getShowProgress()
{
    return showProgressText;
}

void QWProgressValue::setEnableAnimation(bool ok)
{
    EnableAnimation = ok;
}

bool QWProgressValue::getEnableAnimation()
{
    return EnableAnimation;
}

void QWProgressValue::resizeEvent(QResizeEvent* event)
{
    makeImage();
    QWidget::resizeEvent(event);
}
