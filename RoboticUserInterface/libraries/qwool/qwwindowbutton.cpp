#include "qwwindowbutton.h"

QWWindowButton::QWWindowButton(QWidget *m_mainPage) : QWidget(m_mainPage) {
  TextColor = QColor(240, 240, 240), backgroundColor = QColor(150, 150, 150);
  backgroundBorderColor = QColor(150, 150, 150);
  selectdBorderColor = QColor(255, 255, 255);
  selectdBorderHighLightColor = QColor(255, 255, 255);
  notSelectdColor = QColor(0, 0, 0, 0);
  notSelectdBorderColor = QColor(0, 0, 0, 0);

  this->setSelectdColor(QColor(0, 120, 215));

  setMouseTracking(true);
  // setMouseTracking(true);
}

QWWindowButton::~QWWindowButton() {}

void QWWindowButton::paintEvent(QPaintEvent *event) {
  if (UnitList.size() == 0) {
    return;
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setFont(font());

  if (drawBackground) {
    if (drawBackgroundBorder) {
      painter.setPen(QPen(backgroundBorderColor, backgroundBorderLineWidth,
        Qt::SolidLine, Qt::PenCapStyle::RoundCap));
    }
    else {
      painter.setPen(Qt::NoPen);
    }

    painter.setBrush(backgroundColor);
    int interval = backgroundBorderLineWidth / 2.0;
    QPainterPath path;
    path.addRoundedRect(QRectF(interval, interval, width() - 2 * interval, height() - 2 * interval),
      backgroundBorderRadius, backgroundBorderRadius);

    painter.drawPath(path);
  }

  switch (showMode) {
  case ShowMode::List: {
    if (unselectedVisible) {

    }
    int totalSize = UnitList.size();
    int interval = (unitBorderLineWidth + backgroundBorderLineWidth) / 2.0 + intervalDistance;
    int unitWidth = (width() - interval) / totalSize;

    for (int i = 0; i < totalSize; i++) {
      if (selectionMode == SelectionMode::SingleSelection) {
        if (i == currentUnitIndex) {
          painter.setBrush(selectdColor);
        }
        else {
          painter.setBrush(notSelectdColor);
        }
      }
      else if (selectionMode == SelectionMode::MultiSelection) {
        if (currentMultiUnitIndex & (1 << i)) {
          painter.setBrush(selectdColor);
        }
        else {
          painter.setBrush(notSelectdColor);
        }
      }

      if (hoverUnitIndex == i) {
        painter.setBrush(selectdHighLightColor);
      }

      if (drawSelectdUnitBorder) {
        painter.setPen(QPen(selectdColor, unitBorderLineWidth));
      }
      else {
        painter.setPen(Qt::NoPen);
      }

      QPainterPath path;
      int startPosX = unitWidth * i;
      QRect rect(startPosX + interval, interval, unitWidth - interval,
        height() - 2 * interval);
      path.addRoundedRect(rect, unitBorderRadius, unitBorderRadius);

      painter.drawPath(path);
      painter.setPen(TextColor);
      painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, UnitList.at(i));
    }
  } break;

  case ShowMode::Single: {
    int interval = (unitBorderLineWidth + backgroundBorderLineWidth) / 2.0 +
      intervalDistance;

    painter.setBrush(selectdColor);
    QPainterPath path;
    QRect rect(interval, interval, width() - 2 * interval,
      height() - 2 * interval);
    path.addRoundedRect(rect, unitBorderRadius, unitBorderRadius);

    painter.drawPath(path);
    painter.setPen(TextColor);
    painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter,
      UnitList.at(currentUnitIndex));
  } break;
  default:
    break;
  }
}

void QWWindowButton::resizeEvent(QResizeEvent *event) {
  update();
}

void QWWindowButton::mousePressEvent(QMouseEvent *event) {
  if (isEnabled()) {
    if (event->buttons() & Qt::LeftButton) {
      event->accept();
      mouseClicked = true;
    } else {
      event->ignore();
    }
  }
}
void QWWindowButton::mouseMoveEvent(QMouseEvent *event) {
  hoverUnitIndex = getMousePosInUnit();

  if (hoverUnitIndexLast != hoverUnitIndex) {
    hoverUnitIndexLast = hoverUnitIndex;

    update();
  }
}
void QWWindowButton::mouseReleaseEvent(QMouseEvent *event) {
  if (isEnabled()) {
    if ((event->type() == QMouseEvent::MouseButtonRelease) &&
        (event->button() == Qt::LeftButton)) {
      event->accept();
      if (mouseClicked) {
        mouseClicked = false;

         if (!allowMouseClicked) {
          return;           
        }

        unsigned int unitIndexFromMouse = getMousePosInUnit();

        if (selectionMode == SelectionMode::SingleSelection) {
          if (unitIndexFromMouse != currentUnitIndex) {
            this->setSelectUnitIndex(unitIndexFromMouse);
            emit selectUnitIndexChanged(unitIndexFromMouse);
          }
        } else if (selectionMode == SelectionMode::MultiSelection) {
          unsigned int newIndex = currentMultiUnitIndex | (1 << unitIndexFromMouse);

          if (newIndex != currentMultiUnitIndex) {
            this->setMultiSelectUnitIndex(newIndex);
            emit selectUnitIndexChanged(newIndex);
          }
        }
      }

    } else {
      event->ignore();
    }
  }
}
unsigned QWWindowButton::getMousePosInUnit() {
  if(UnitList.size() == 0){
    return 0;
  }
  int unitWidth = width() / UnitList.size();
  int pointX = this->mapFromGlobal(QCursor().pos()).x();

  return pointX / unitWidth;
}

void QWWindowButton::leaveEvent(QEvent *e) {
  hoverUnitIndexLast = -1;
  hoverUnitIndex = -1;
  update();
}
void QWWindowButton::enterEvent(QEnterEvent *) {}

void QWWindowButton::addUnit(QString UnitName) {
  if (UnitList.size() == 0)
    currentUnitIndex = 0;

  UnitList << UnitName;

  update();
}
void QWWindowButton::delUnit(int index) {

  if (index > 0 && index < UnitList.size()) {
    if (currentUnitIndex == (UnitList.size() - 1)) {
      currentUnitIndex -= 1;
    }

    UnitList.removeAt(index);
  }

  update();
}
void QWWindowButton::setSelectUnit(const QString &name ){
  setSelectUnitIndex(UnitList.indexOf(name));
}
void QWWindowButton::setSelectUnitIndex(int index) {
  if (UnitList.size() == 0) {
    return;
  } else if (index < 0) {
    index = -1;
  } else if (index >= UnitList.size()) {
    index = UnitList.size() - 1;
  }

  currentUnitIndex = index;

  update();
}
void QWWindowButton::setMultiSelectUnitIndex(unsigned int index){
  if (UnitList.size() == 0) {
    return;
  }
  currentMultiUnitIndex = index & ((1 << UnitList.size()) - 1);;

  update();
}
int QWWindowButton::getMultiCurrentUnitIndex(){
  return currentMultiUnitIndex;
}

int QWWindowButton::getCurrentUnitIndex() { return currentUnitIndex; }

void QWWindowButton::setAllowMouseClicked(bool ok) { allowMouseClicked = ok; }
bool QWWindowButton::getAllowMouseClicked() { return allowMouseClicked; }

void QWWindowButton::setShowMode(QWWindowButton::ShowMode sm) { showMode = sm; }
QWWindowButton::ShowMode QWWindowButton::getShowMode() { return showMode; }

void QWWindowButton::setSelectionMode(QWWindowButton::SelectionMode sm) { selectionMode = sm; }
QWWindowButton::SelectionMode QWWindowButton::getSelectionMode() { return selectionMode; }

void QWWindowButton::setUnselectedVisible(bool ok) { unselectedVisible = ok; }
bool QWWindowButton::getUnselectedVisible() { return unselectedVisible; }

void QWWindowButton::setUnitName(int index, QString UnitName) {
  if (index > 0 && index < UnitList.size()) {
    UnitList[index] = UnitName;
  }
}
QString QWWindowButton::getUnitName(int index) {
  if (index >= 0 && index < UnitList.size()) {
    return UnitList.at(index);

  } else {
    return QString();
  }
}

void QWWindowButton::setSelectdColor(QColor c) {
  selectdColor = c;
  selectdHighLightColor = c;
  selectdHighLightColor.setAlpha(100);
}
QColor QWWindowButton::getSelectdColor() { return selectdColor; }

void QWWindowButton::setBackgroundColor(QColor c) { backgroundColor = c; }
QColor QWWindowButton::getBackgroundColor() { return backgroundColor; }

int QWWindowButton::getUnitSize() const { return UnitList.size(); }
