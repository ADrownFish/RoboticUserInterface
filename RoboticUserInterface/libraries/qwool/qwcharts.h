#ifndef QWCHARTS_H
#define QWCHARTS_H

#include <QWidget>
#include <QTimer>
#include <QList>
#include <QPainter>
#include <QQueue>
#include <QThread>
#include <QMutex>
#include <qmath.h>
#include <QPainterPath>


class QWCharts : public QWidget
{
    Q_OBJECT

        Q_PROPERTY(int appendToValue READ getValue WRITE appendValue)

        Q_PROPERTY(QColor BorderColor READ getBorderColor WRITE setBorderColor)
        Q_PROPERTY(QColor Background READ getBackground WRITE setBackground)
        Q_PROPERTY(QColor PointColor READ getPointColor WRITE setPointColor)

        Q_PROPERTY(int PenWidth READ getPenWidth WRITE setPenWidth)
        Q_PROPERTY(int MaxValue READ getMaxValue WRITE setMaxValue)
        Q_PROPERTY(int MinValue READ getMinValue WRITE setMinValue)
        Q_PROPERTY(int AxisXValueInterval READ getAxisXValueInterval WRITE setAxisXValueInterval)
        Q_PROPERTY(int CellWidth READ getCellWidth WRITE setCellWidth)
        Q_PROPERTY(int CellHeight READ getCellHeight WRITE setCellHeight)

        Q_PROPERTY(bool ShowBorder READ getShowBorder WRITE setShowBorder)
        Q_PROPERTY(bool ShowBackground READ getShowBackground WRITE setShowBackground)
        Q_PROPERTY(bool FillCurve READ getFillCurve WRITE setFillCurve)
        Q_PROPERTY(bool ShowLabel READ getShowLabel WRITE setShowLabel)

        Q_PROPERTY(int MaxBufferPointCount READ getMaxBufferPointCount WRITE setMaxBufferPointCount)



public:
    QWCharts(QWidget *parent = 0);
    ~QWCharts();

    //添加一个新点
    void appendPoint(QPoint);
    QPoint getNewPoint();

    //添加一个值，这个值是y轴
    void appendValue(int value);
    int getValue();

    //设置边框颜色
    void setBorderColor(const QColor& value);
    QColor getBorderColor()const ;

    //设置背景颜色
    void setBackground(const QColor& value);
    QColor getBackground() const ;

    //设置点颜色
    void setPointColor(const QColor& value);
    QColor getPointColor() const;

    //设置画笔的宽度
    void setPenWidth(float value);
    float getPenWidth();

    //设置最大值
    void setMaxValue(float value);
    float getMaxValue();

    //设置最小值
    void setMinValue(float value);
    float getMinValue();

    //X轴 值的默认间隔
    void setAxisXValueInterval(int value);
    int getAxisXValueInterval();

    //设置格子宽度
    void setCellWidth(int value);
    int getCellWidth();

    //设置格子高度
    void setCellHeight(int value);
    int getCellHeight();


    //设置是否显示边框
    void setShowBorder(bool value);
    bool getShowBorder();

    //设置是否显示背景
    void setShowBackground(bool value);
    bool getShowBackground();

    //设置填充曲线
    void setFillCurve(bool value);
    bool getFillCurve();

    //设置显示标签
    void setShowLabel(bool value);
    bool getShowLabel();

    //允许存在最大数量的点
    void setMaxBufferPointCount(int value);
    int getMaxBufferPointCount();

protected:
    // 绘制开关
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    // 大小改变事件
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
signals:

private:
    // 状态切换时，用于产生滑动效果
    void onTimeout();

    void updateImage();
    void makeImage();

private:
    QColor m_BorderColor;         //边框颜色
    QColor m_Background;          //背景颜色
    QColor m_PointColor;

    float m_PenWidth;             //连接线线宽
    float m_MaxValue;             //最大值
    float m_MinValue;             //最小值


    int ValueInterval;               //X轴 值的默认间隔
    int CellWidth;
    int CellHeight;

    int maxPointSize;

    bool m_ShowBorder;
    bool m_ShowBG;
    bool m_FillCurve;
    bool m_showLabel;

    QList <QPoint > PointList;

    QPixmap Pixmap;
};

#endif
