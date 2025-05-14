#ifndef QWWINDOWBUTTON_H
#define QWWINDOWBUTTON_H

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QWidget>

class QWWindowButton : public QWidget {
  Q_OBJECT

public:
  enum class ShowMode { List, Single };
  enum class SelectionMode {
    SingleSelection,
    MultiSelection,
  };

  QWWindowButton(QWidget *m_mainPage = 0);
  ~QWWindowButton();

  void addUnit(QString UnitName);
  void delUnit(int index);

  void setSelectUnitIndex(int index);
  void setSelectUnit(const QString &name );

  int getCurrentUnitIndex();

  void setMultiSelectUnitIndex(unsigned int index);
  int getMultiCurrentUnitIndex();

  void setAllowMouseClicked(bool ok);
  bool getAllowMouseClicked();

  void setShowMode(ShowMode sm);
  ShowMode getShowMode();
  void setSelectionMode(SelectionMode sm);
  SelectionMode getSelectionMode();

  void setBackgroundColor(QColor c);
  QColor getBackgroundColor();

  void setSelectdColor(QColor c);
  QColor getSelectdColor();

  void setUnitName(int index, QString UnitName);
  QString getUnitName(int index);

  int getUnitSize() const;

  void setUnselectedVisible(bool ok);
  bool  getUnselectedVisible();

protected:
  // 绘制开关
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
  // 大小改变事件
  void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
  // 鼠标按下事件
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  // 鼠标释放事件 - 切换开关状态、发射toggled()信号
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

  void leaveEvent(QEvent *e) override;
  void enterEvent(QEnterEvent *e) override;

signals:
  void selectUnitIndexChanged(unsigned int index);

private:
  unsigned int getMousePosInUnit();

private:
  bool mouseClicked = false;
  bool allowMouseClicked = true;
  bool unselectedVisible = true;

  QStringList UnitList;

  int currentUnitIndex = -1;
  int hoverUnitIndex = -1;
  int hoverUnitIndexLast = 0;

  unsigned int currentMultiUnitIndex = 0;

  int backgroundBorderLineWidth = 2;
  int backgroundBorderRadius = 10;
  int unitBorderLineWidth = 2;
  int unitBorderRadius = 6;

  int intervalDistance = 5;

  bool drawBackground = true;
  bool drawBackgroundBorder = false;
  bool drawSelectdUnit = true;
  bool drawSelectdUnitBorder = false;
  bool drawNotSelectdUnit = false;
  bool drawNotSelectdUnitBorder = false;

  QColor TextColor;
  QColor backgroundColor;
  QColor backgroundBorderColor;
  QColor selectdColor;
  QColor selectdBorderColor;
  QColor selectdHighLightColor;
  QColor selectdBorderHighLightColor;
  QColor notSelectdColor;
  QColor notSelectdBorderColor;

  ShowMode showMode = ShowMode::List;
  SelectionMode selectionMode = SelectionMode::SingleSelection;
};

#endif
