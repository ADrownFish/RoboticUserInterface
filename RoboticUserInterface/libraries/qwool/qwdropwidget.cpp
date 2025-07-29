#include "qwdropwidget.h"
#include "qwdropwidget.h"
#include "qwdropwidget.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QPropertyAnimation>

QWDropWidget::QWDropWidget(QWidget* parent)
  : QWidget(parent)
{
  auto layout = new QHBoxLayout(this);
  layout->setSpacing(1);
  layout->setContentsMargins(0,0,0,0);

  m_arrowButton = new QPushButton(this);
  m_arrowButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  m_arrowButton->setFixedWidth(24);

  // 使用QPropertyAnimation实现平滑过渡
  animHover = new QPropertyAnimation(m_arrowButton, "iconSize", this);
  animHover->setDuration(200);
  animHover->setStartValue(QSize(12, 12));
  animHover->setEndValue(QSize(20, 20));

  animRelease = new QPropertyAnimation(m_arrowButton, "iconSize", this);
  animRelease->setDuration(100);
  animRelease->setStartValue(QSize(20, 20));
  animRelease->setEndValue(QSize(12, 12));

  animPress = new QPropertyAnimation(m_arrowButton, "iconSize", this);
  animPress->setDuration(50);  // 更快的动画
  animPress->setStartValue(QSize(20, 20));  // 从悬停大小开始
  animPress->setEndValue(QSize(18, 18));    // 按下时稍微缩小一点

  m_arrowButton->installEventFilter(this);

  m_arrowButton->setStyleSheet(R"(
    QPushButton{
      background - color: rgb(31, 31, 31);
      border : 0;
      outline: none;
    }
    QPushButton:focus {
      outline: none;
    }
  )");

  layout->addWidget(m_arrowButton);
  connect(m_arrowButton, &QPushButton::clicked, this, &QWDropWidget::onArrowClicked);
}

bool QWDropWidget::eventFilter(QObject* obj, QEvent* event)
{
  if (obj == m_arrowButton) {
    if (event->type() == QEvent::Enter) {
      animHover->start();
    }
    else if (event->type() == QEvent::Leave) {
      if (!m_arrowButton->isDown()) {
        animRelease->start();
      }
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
      if (m_arrowButton->underMouse()) {
        animHover->start();
      }
      else {
        animRelease->start();
      }
    }
  }
  return QObject::eventFilter(obj, event);
}

void QWDropWidget::setWidget(QWidget* w)
{
  if (m_widget) {
    layout()->removeWidget(m_widget);
    m_widget->deleteLater(); // optional
  }

  m_widget = w;
  if (m_widget) {
    layout()->removeWidget(m_arrowButton);
    layout()->addWidget(m_widget);
    layout()->addWidget(m_arrowButton);
  }
}

void QWDropWidget::setMenu(QMenu* menu, std::function<void()> beforeMenuCallback)
{
  m_menu = menu;
  m_beforeMenu = beforeMenuCallback;
}

void QWDropWidget::setDropDirection(DropDirection dir)
{
  m_direction = dir;
}

void QWDropWidget::setDropIcon(const QIcon& icon)
{
  m_arrowButton->setIcon(icon);
  m_arrowButton->setIconSize(QSize(12, 12));
}

void QWDropWidget::onArrowClicked()
{
  if (!m_menu) return;

  constexpr int offset = 1;
  QPoint pos;

  switch (m_direction) {
  case DropDirection::Down:
    pos = mapToGlobal(QPoint(0, height() + offset));
    break;
  case DropDirection::Up:
    pos = mapToGlobal(QPoint(0, -m_menu->sizeHint().height() - offset));
    break;
  case DropDirection::Left:
    pos = mapToGlobal(QPoint(-m_menu->sizeHint().width() - offset, 0));
    break;
  case DropDirection::Right:
    pos = mapToGlobal(QPoint(width() + offset, 0));
    break;
  }

  m_beforeMenu();
  m_menu->exec(pos);
}

void QWDropWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
}

void QWDropWidget::paintEvent(QPaintEvent* event)
{

  QWidget::paintEvent(event);
}
