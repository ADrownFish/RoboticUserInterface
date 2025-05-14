#ifndef QWPROGRESSVALUE_H
#define QWPROGRESSVALUE_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QPixmap>

class QWProgressValue : public QWidget
{
    Q_OBJECT
        Q_PROPERTY(int MaxValue READ getMaxValue WRITE setMaxValue)
        Q_PROPERTY(int Value READ getValue WRITE setValue)
        Q_PROPERTY(QColor backgroundColor READ getBackgroundColor WRITE setBackgroundColor)
        Q_PROPERTY(QColor ForegroundColor READ getForegroundColor WRITE setForegroundColor)

        Q_PROPERTY(QString text READ getText WRITE setText)
        Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)

        Q_PROPERTY(bool ShowProgress READ getShowProgress WRITE setShowProgress)

public:
    QWProgressValue(QWidget* parent = 0);

    void setMaxValue(float value);
    float getMaxValue();

    void setValue(float value);
    float getValue();

    void setText(const QString &str);
    QString getText();

    void setBackgroundColor(const QColor& color);
    QColor getBackgroundColor();

    void setForegroundColor(const QColor& color);
    QColor getForegroundColor();

    void setTextColor(const QColor& color);
    QColor getTextColor();

    void setShowProgress(bool ok);
    bool getShowProgress();

    //是否启用动画
    void setEnableAnimation(bool ok);
    bool getEnableAnimation();

private:

    void onTimerout();
    void makeImage();

protected:
    // 绘制开关
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    float m_MaxValue;
    float m_Value;
    float m_currentValue;

    bool showProgressText;
    bool EnableAnimation = false;
    
    QString text;
    QColor foregroundColor;
    QColor backgroundColor;
    QColor textColor;
    QTimer timer;
    QPixmap Pixmap;
};

#endif
