#pragma once

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QVariantAnimation>

#include "qwool/qwswitcher.h"
#include "qt_material_widgets/qtmaterialraisedbutton.h"


class NavigationObject : public QWidget {
public:

  NavigationObject(QWidget* parent) : QWidget(parent){}

  virtual void setExpand(bool ok) {}
};


class NavigationSwitcher : public NavigationObject {
  Q_OBJECT

public:
  explicit NavigationSwitcher(QWidget* parent = nullptr);
  ~NavigationSwitcher() override = default;

  void setText(const QString& text);
  QWSwitcher* getSwitcher();

  void setExpand(bool ok) override;

private:
  QLabel text;
  QWSwitcher toggle;

  QHBoxLayout* layout;
  QWidget stretchWidget;
  QWidget stretchWidgetHead;
};

class NavigationItem : public NavigationObject {
  Q_OBJECT
public:
  explicit NavigationItem(QWidget* parent = nullptr);
  void setIcon(const QIcon& icon);
  void setText(const QString& text);
  void setClicked(bool b);

  void setAsButton(bool ok);
  bool asButton();

  void setExpand(bool ok) override;

protected:
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

signals:
  void clicked(NavigationItem *);

private:
  
  bool isHovered = false;
  bool isSelected = false;
  bool isAsButton = false;

  int radius = 5;

  QLabel* iconLabel;
  QLabel* textLabel;
  QWidget* lineWidget;
  QWidget* stretchWidget;

  QIcon currentIcon;

  QHBoxLayout* layout;
};

class NavigationView : public QWidget {
  Q_OBJECT

public:
  explicit NavigationView(QWidget* parent = nullptr);
  ~NavigationView() override = default;

  void addItemToTop(NavigationObject* widget);
  void addItemToCenter(NavigationItem* widget);
  void addItemToBottom(NavigationItem* widget);

  void setClicked(NavigationItem *item);

  void toggleExpandRetract(bool ok = true);

private:


protected:
  void resizeEvent(QResizeEvent* event) override;

private:
  bool isExpand = true;
  int expandedWidth;
  int collapsedWidth;
  QVariantAnimation* widthAnimation;

  QVBoxLayout* topLayout;
  QVBoxLayout* centerLayout;
  QVBoxLayout* bottomLayout;

  NavigationItem* menu;
  NavigationItem* selected = nullptr;

  QList<NavigationObject*> objList;
};


/*

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    setLayout(layout);
    layout->addWidget(&toggle);
    layout->addWidget(&text);
    this->layout()->setContentsMargins(padding, padding, padding, padding);

    toggle.setPenWidth(2);
    toggle.setBackgroundColor(QColor(0, 0, 0, 0));
    toggle.setToggle(true);
    toggle.setBackCheCkedgroundColor(QColor(0, 0, 0, 0));

    QFont font("Roboto", fontSize, QFont::Medium);
    text.setFont(font);

    resize(100, 60);
    setBorderRadius(5);

*/