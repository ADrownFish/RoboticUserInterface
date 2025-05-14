#include "qwbutton.h"

QWButton::QWButton(QWidget* m_mainPage) :
    QWidget(m_mainPage),
    CheckedColor(0, 120, 215),
    UnCheckedColor(100, 100, 100),
    nowColor(UnCheckedColor),
    Transformationtimes(0),
    mChecked(false),
    isEnableSvg (false),
    svgSize(50,50)
{
    connect(&m_timer,&QTimer::timeout,this,&QWButton::onTimeout);
}

void QWButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int tempNum = (8 - abs(Transformationtimes - 8)) * 4;
    tempNum = tempNum * width() * 0.003f;
    p.setPen((Qt::NoPen));
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(QBrush(nowColor));
    p.drawEllipse(tempNum, tempNum, width() - tempNum * 2, height() - tempNum * 2);

    if (isEnableSvg)    {
      svg.render(&p, QRect((width() - svgSize.width()) / 2, (height() - svgSize.height()) / 2, svgSize.width(), svgSize.height()));
    }
}

void QWButton::resizeEvent(QResizeEvent* event)
{
    setFixedSize(width(), width());
    update();
}
void QWButton::onTimeout()
{
    if (mChecked)
    {
        nowColor.setRed(nowColor.red() + (CheckedColor.red() - nowColor.red()) * 1/Transformationtimes);
        nowColor.setGreen(nowColor.green() + (CheckedColor.green() - nowColor.green()) * 1 / Transformationtimes);
        nowColor.setBlue(nowColor.blue() + (CheckedColor.blue() - nowColor.blue()) * 1 / Transformationtimes);
    }
    else
    {
        nowColor.setRed(nowColor.red() + (UnCheckedColor.red() - nowColor.red()) * 1 / Transformationtimes);
        nowColor.setGreen(nowColor.green() + (UnCheckedColor.green() - nowColor.green()) * 1 / Transformationtimes);
        nowColor.setBlue(nowColor.blue() + (UnCheckedColor.blue() - nowColor.blue()) * 1 / Transformationtimes);
    }
    Transformationtimes--;

    if (Transformationtimes == 0)
    {
        m_timer.stop();
    }

    update();
}
void QWButton::mousePressEvent(QMouseEvent* event)
{
    if (isEnabled()) {
        if (event->buttons() & Qt::LeftButton) {
            event->accept();
        }
        else {
            event->ignore();
        }
    }
}
void QWButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (isEnabled()) {
        if ((event->type() == QMouseEvent::MouseButtonRelease) && (event->button() == Qt::LeftButton)) {
            event->accept();
            Transformationtimes = 16;
            mChecked = !mChecked;
            emit toggled(mChecked);
            m_timer.start(15);
        }
        else {
            event->ignore();
        }
    }
}

void QWButton::setCheckedColor(const QColor& color)
{
    CheckedColor = color;
    update();
}

void QWButton::setUnCheckedColor(const QColor& color)
{
    UnCheckedColor = color;
    update();
}

QColor QWButton::getCheckedColor()
{
    return CheckedColor;
}

QColor QWButton::getUnCheckedColor()
{
    return UnCheckedColor;
}

void QWButton::setSvg(const QString& res)
{
    if (res.isEmpty())
    {
        isEnableSvg = false;
        return;
    }
    svg.load(res);
    isEnableSvg = true;
    update();
}

void QWButton::setSvgSize(const QSize& size)
{
    svgSize = size;
    update();
}

QSize QWButton::getSvgSize()
{
    return svgSize;
}

void QWButton::setChecked(bool ok)
{
    mChecked = ok;
    Transformationtimes = 16;
    m_timer.start(15);
}

bool QWButton::getChecked()
{
    return mChecked;
}

void QWButton::setUnchecked(bool ok)
{
    setChecked(!ok);
}

bool QWButton::getUnChecked()
{
    return !mChecked;
}
