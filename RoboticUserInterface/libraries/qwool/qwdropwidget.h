#pragma once

#include <QWidget>
#include <QPushButton>
#include <QMenu>
#include <QPointer>
#include <functional>

class QPropertyAnimation;

class QWDropWidget : public QWidget
{
  Q_OBJECT
public:
  enum class DropDirection {
    Up,
    Down,
    Left,
    Right
  };

  explicit QWDropWidget(QWidget* parent = nullptr);

  // 设置外部提供的按钮和菜单
  void setWidget(QWidget* button);
  void setMenu(QMenu* menu, std::function<void()> beforeMenuCallback = []{ });
  void setDropDirection(DropDirection dir);
  void setDropIcon(const QIcon& icon);

protected:
  void resizeEvent(QResizeEvent* event) override;

  void paintEvent(QPaintEvent* event) override;

  bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
  void onArrowClicked();

private:
  QPointer<QWidget> m_widget;
  QPushButton* m_arrowButton = nullptr;
  QPointer<QMenu> m_menu;
  DropDirection m_direction = DropDirection::Down;

  std::function<void()> m_beforeMenu;

  QPropertyAnimation* animHover;
  QPropertyAnimation* animPress;
  QPropertyAnimation* animRelease;
};
