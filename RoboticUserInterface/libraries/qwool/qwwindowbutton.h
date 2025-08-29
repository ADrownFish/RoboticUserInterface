#ifndef QWWINDOWBUTTON_H
#define QWWINDOWBUTTON_H

#include <QColor>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QWidget>
#include <QVariantAnimation>
#include <QEasingCurve>

class QWWindowButton : public QWidget {
  Q_OBJECT

public:
  enum class ViewMode {
    List, 
    Single,
  };
  enum class SelectionMode {
    Single,
    Multiple,
  };
  enum class Direction {
    Horizontal,
    Vertical,
  };
  enum class UnitSizeMode {
    Fixed, 
    Stretch,
  };

  QWWindowButton(QWidget* m_mainPage = nullptr);
  ~QWWindowButton();

  void addUnit(QString UnitName);
  void delUnit(int index);
  void clearUnit();

  void setSelectUnitIndex(int index);
  void setSelectUnit(const QString& name);

  int getCurrentUnitIndex();

  void setMultiSelectUnitIndex(unsigned int index);
  int getMultiCurrentUnitIndex();

  void setAllowMouseClicked(bool ok);
  bool getAllowMouseClicked();

  void setAutoAdjustMinimumSize(bool ok);
  bool getAutoAdjustMinimumSize();

  void setIntervalDistance(int distance);
  int getIntervalDistance();

  void setFixedUnitSize(int size);
  int getFixedUnitSize();

  void setViewMode(ViewMode d);
  ViewMode getViewMode();

  void setSelectionMode(SelectionMode d);
  SelectionMode getSelectionMode();

  void setDirection(Direction d);
  Direction getDirection();

  void setUnitSizeMode(UnitSizeMode d);
  UnitSizeMode getUnitSizeMode();

  void setBackgroundColor(QColor c);
  QColor getBackgroundColor();

  void setSelectdColor(QColor c);
  QColor getSelectdColor();

  void setUnitName(int index, QString UnitName);
  QString getUnitName(int index);

  int getUnitSize() const;

  void setAnimationDuration(int duration);
  int getAnimationDuration() const;

  void setHoverColor(QColor c);
  QColor getHoverColor() const;

protected:
  void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void leaveEvent(QEvent* e) override;
  void enterEvent(QEnterEvent* e) override;

signals:
  void selectUnitIndexChanged(unsigned int index);

private:
  void updateHoverAnimation(int index);
  void updateSelectionAnimation(int targetIndex);
  int getMousePosInUnit();
  void calculateUnitRects();
  void startHoverAnimation(int index, double startValue, double endValue);
  void startSelectionAnimation(double startValue, double endValue);
  void adjustMinimumSize();

private:
  bool mouseClicked = false;
  bool allowMouseClicked = true;
  bool autoAdjustMinimumSize = true;

  QStringList UnitList;
  QList<QRectF> unitRects; // 存储每个单元的位置和大小

  int currentUnitIndex = -1;
  int hoverUnitIndex = -1;
  int hoverUnitIndexLast = -1;

  unsigned int currentMultiUnitIndex = 0;

  int backgroundBorderLineWidth = 2;
  int backgroundBorderRadius = 10;
  int unitBorderLineWidth = 2;
  int unitBorderRadius = 6;

  int intervalDistance = 5;
  int animationDuration = 200; // 动画持续时间(ms)
  int fixedUnitSize = 50;

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
  QColor hoverColor;

  ViewMode viewMode = ViewMode::List;
  SelectionMode selectionMode = SelectionMode::Single;
  Direction direction = Direction::Horizontal;
  UnitSizeMode unitSizeMode = UnitSizeMode::Stretch;

  // 动画相关
  QVariantAnimation* hoverAnimation;
  QVariantAnimation* selectionAnimation;
  double hoverProgress = 0.0;      // 0.0-1.0
  double selectionProgress = 0.0;  // 0.0-1.0
  int animationTargetIndex = -1;   // 动画目标索引
  int animationStartIndex = -1;    // 动画起始索引

  int hoverAnimationIndex = -1;
};

#endif