#include "qwprogresswait.h"

#include "qdebug.h"

QWProgressWait::QWProgressWait(QWidget *m_mainPage) :
    QWidget(m_mainPage)
{
    connect(&timer,&QTimer::timeout,this,&QWProgressWait::onTimeOut);
}

std::pair<int,int> QWProgressWait::getArgTypeA(int time)
{
    int valueA = 0;
    int valueB = 0;

    float t = time / 1000.f;

	auto outQuad = [](double t) {
		return 1.0 - (1.0 - t) * (1.0 - t);
	};

	auto inQuad = [](double t)
	{
		return t * t;
	};

    //case EasingCurve::InOutQuad:
    valueB = ( 
		(t < 0.5) ? 2.0 * t * t : 1.0 - pow(-2.0 * t + 2.0, 2.0) / 2.0
		)  * 360.f;

    //case EasingCurve::OutInQuad:
    valueA = ((outQuad(2.0 * t) / 2.0) ) * 360.f;

    return std::pair<int,int>{valueB * 16, valueA * 16};
}
std::pair<int, int> QWProgressWait::getArgTypeB(int time)
{
	int valueA = 0;
	int valueB = 0;

	float t = time / 1000.f;

	auto outQuad = [](double t) {
		return 1.0 - (1.0 - t) * (1.0 - t);
	};

	auto inQuad = [](double t)
	{
		return t * t;
	};

	//case EasingCurve::InOutQuad:
	valueA = (	
		outQuad(2.0 * t) / 2.0
		) * 360.f;

	t = 1 - t;
	//case EasingCurve::OutInQuad:
	valueB = (
		2.0 * t * t
		) * 360.f;

	return std::pair<int, int>{valueB * 16, valueA * 16};
}

void QWProgressWait::updateImage()
{
	Pixmap = QPixmap(this->size());
	Pixmap.fill(QColor(0, 0, 0, 0));

	QPainter p(&Pixmap);

	p.setRenderHint(QPainter::Antialiasing);

	//计算应该画的直径
	int diameter = 0;
	int width = this->width();
	int height = this->height();
	if (width > height)
	{
		diameter = height;
	}
	else {
		diameter = width;
	}

	p.setPen(QPen(color, penWidth, Qt::PenStyle::SolidLine, Qt::RoundCap));

	std::pair<int, int> value;
	switch (waitingEffectType)
	{
	case WaitingEffectType::TypeA :
		value = getArgTypeA(CurrentProgress);
		break;
	case WaitingEffectType::TypeB:
		value = getArgTypeB(CurrentProgress);
		break;
	}

	p.drawArc(penWidth, penWidth, diameter - penWidth * 2, diameter - penWidth * 2, value.first, value.second);

	//p.setPen(QPen(QColor(90,90,90)));
	//p.drawText(diameter/2,diameter/2,QString::number(CurrentProgress));

	update();
}

void QWProgressWait::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.drawPixmap(0, 0, Pixmap);
}

void QWProgressWait::onTimeOut()
{
    CurrentProgress += speed;

    if(CurrentProgress >= FullProgress)
    {
        CurrentProgress = 0;
    }

	updateImage();
}

//开始与停止
void QWProgressWait::Launch(bool ok)
{
    isLaunch = ok;

    if(isLaunch)
    {
        timer.start(20);
    }
    else
    {
        timer.stop();

        CurrentProgress = 0;
		updateImage();
    }
}
bool QWProgressWait::LaunchState()
{
    return isLaunch;
}

//设置动画效果
void QWProgressWait::setWaitingEffectType(WaitingEffectType type)
{
    waitingEffectType = type;

	updateImage();
}
QWProgressWait::WaitingEffectType QWProgressWait::getWaitingEffectType()
{
    return waitingEffectType;
}

//设置颜色
void QWProgressWait::setColor(const QColor &c)
{
    color = c;

	updateImage();
}
QColor QWProgressWait::getColor()
{
    return color;
}

void QWProgressWait::setPenWidth(int value)
{
    penWidth = value;

	updateImage();
}
int QWProgressWait::getPenWidth()
{
    return penWidth;
}

void QWProgressWait::setSpeed(int value)
{
    if(value <= 0 || value > 10)
        return;

    speed = value;

	updateImage();
}

int QWProgressWait::getSpeed()
{
    return speed;
}
