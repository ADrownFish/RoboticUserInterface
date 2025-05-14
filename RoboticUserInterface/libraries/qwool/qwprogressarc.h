#ifndef QWPROGRESSARC_H
#define QWPROGRESSARC_H

#include <QWidget>
#include <QTimer>
#include <QPainter>

class QWProgressArc : public QWidget
{
    Q_OBJECT
        Q_PROPERTY(int MaxValue READ getMaxValue WRITE setMaxValue)
        Q_PROPERTY(int Value READ getValue WRITE setValue)
        Q_PROPERTY(int PenWidth READ getPenWidth WRITE setPenWidth)
        Q_PROPERTY(QColor ArcColor READ getArcColor WRITE setArcColor)
        Q_PROPERTY(QColor ArcValueColor READ getArcValueColor WRITE setArcValueColor)
        Q_PROPERTY(QColor TextColor READ getTextColor WRITE setTextColor)
        Q_PROPERTY(QColor PercentColor READ getPercentColor WRITE setPercentColor)

public:
    QWProgressArc(QWidget *parent = 0);

    void setMaxValue(int value);
    int getMaxValue();

    void setValue(int  value);
    int getValue();

    void setPenWidth(int  value);
    int getPenWidth();

    void setArcColor(const QColor & value);
    QColor getArcColor();

    void setArcValueColor(const QColor &value);
    QColor getArcValueColor();

    void setTextColor(const QColor &value);
    QColor getTextColor();

    void setPercentColor(const QColor &value);
    QColor getPercentColor();

protected:
    // 绘制摇杆
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    // 大小改变事件
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
signals:
    void valueChanged();

private:
    // 状态切换时，用于产生动画效果
    void onTimeout();

private:
    short m_MaxValue;
    short m_Value;
    short m_TargetValue;
    short m_PenWidth;

    QColor m_ArcBGColor;
    QColor m_ArcValueColor;
    QColor m_TextColor;
    QColor m_PercentColor;

    QTimer m_timer;          // 定时器
};


#endif
