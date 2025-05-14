#ifndef QWPROGRESSWAIT_H
#define QWPROGRESSWAIT_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <math.h>

class QWProgressWait : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool Launch READ LaunchState WRITE Launch)
    Q_PROPERTY(WaitingEffectType Value READ getWaitingEffectType WRITE setWaitingEffectType)
    Q_PROPERTY(QColor Color READ getColor WRITE setColor)
    Q_PROPERTY(int PenWidth READ getPenWidth WRITE setPenWidth)
    Q_PROPERTY(int Speed READ getSpeed WRITE setSpeed)

public:
    enum WaitingEffectType
    {
        TypeA,
        TypeB
    };

public:
    QWProgressWait(QWidget *m_mainPage = 0);

    //开始与停止
    void Launch(bool ok);
    bool LaunchState();

    //设置动画效果
    void setWaitingEffectType(WaitingEffectType type);
    WaitingEffectType getWaitingEffectType();

    //设置颜色
    void setColor(const QColor &color);
    QColor getColor();

    void setPenWidth(int value);
    int getPenWidth();

    void setSpeed(int value);
    int getSpeed();

protected:

    void paintEvent(QPaintEvent* event) override;
   // void resizeEvent(QResizeEvent* event) override;

private:

    //槽函数
    void onTimeOut();

    //获取当前的圆弧参数
    std::pair<int,int> getArgTypeA(int time);
    std::pair<int, int> getArgTypeB(int time);

	void updateImage();

private:

    QColor color;
    WaitingEffectType waitingEffectType = WaitingEffectType::TypeA;

    QTimer timer;

    int penWidth = 20;
    int CurrentProgress = 0;
    int speed = 1;


    const int FullProgress = 1000;
    bool isLaunch = false;

	QPixmap Pixmap;
};

#endif
