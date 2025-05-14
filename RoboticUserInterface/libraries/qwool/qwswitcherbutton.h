#ifndef QWSWITCHERBUTTON_H
#define QWSWITCHERBUTTON_H

#include <memory>

#include <QWidget>
#include <QList>
#include <QPainter>
#include <QMouseEvent>
#include <QSvgRenderer>

class QWSwitcherButton : public QWidget{
  Q_OBJECT
public:
    struct StateItem {
        bool enable = false;
        QString name;
        QColor color;
        std::shared_ptr<QSvgRenderer> svg;
        StateItem() {
          svg = std::make_shared<QSvgRenderer>();
        }
        StateItem(bool enable, QString name, QColor color, QString res) {
          this->enable = enable;
          this->name = name;
          this->color = color;
          svg = std::make_shared<QSvgRenderer>();
          svg->load(res);
        }
    }; 

    //Q_PROPERTY(QList<StateSetStruct> stateSetList READ getStateSetList WRITE setStateSetList)
    //Q_PROPERTY(int stateSetQuantity READ getStateSetQuantity WRITE setStateSetQuantity)
    //Q_PROPERTY(int selectState READ getSelectState WRITE setSelectState)
    //Q_PROPERTY(int borderRadius READ getBorderRadius WRITE setBorderRadius)
    //Q_PROPERTY(int penWidth READ getPenWidth WRITE setPenWidth)
    //Q_PROPERTY(QColor textColor READ getTextColor WRITE setTextColor)


public:
    QWSwitcherButton(QWidget* parent = 0);

    QList<StateItem>& getStateItemList();

    void appendState(const StateItem& item);
    int stateCount();

    void setSelectState(int value);
    int getSelectState();

    void setTextColor(QColor value);
    QColor getTextColor();

    void setBorderRadius(int value);
    int getBorderRadius();

    void setBorderColor(QColor value);
    QColor getBorderColor();

    void setBorder(bool ok);
    bool getBorder();

    void setPenWidth(int value);
    int getPenWidth();

    StateItem& state(int index);

signals:
    void selectIndexChanged(int index);

protected:
    // 绘制开关
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    // 鼠标按下事件
    void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    // 鼠标释放事件 - 切换开关状态、发射toggled()信号
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
    // 大小改变事件
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    void enterEvent(QEnterEvent* event) override {
        QWidget::enterEvent(event); // 确保调用基类的事件处理函数
        // 在这里处理鼠标进入窗口时的操作
        _hover = true;
        update();
    }

    // 鼠标离开窗口事件处理函数
    void leaveEvent(QEvent* event) override {
        QWidget::leaveEvent(event); // 确保调用基类的事件处理函数
        // 在这里处理鼠标离开窗口时的操作
        _hover = false;
        update();
    }

signals:
    // 状态改变时，发射信号
    void toggled(bool checked);

private:
    QList<StateItem> _stateItemList;
    bool _hover = false;
    int _currentState = 0;
    int _borderRadius = 5;

    bool _border = true;
    int _penWidth = 0;

    QColor _borderColor = QColor(80, 85, 90, 0);
    QColor _textColor = QColor(240, 240, 240, 200);
};

#endif
