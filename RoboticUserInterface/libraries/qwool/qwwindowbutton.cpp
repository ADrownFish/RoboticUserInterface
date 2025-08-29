#include "qwwindowbutton.h"

QWWindowButton::QWWindowButton(QWidget* m_mainPage) : QWidget(m_mainPage) {
  TextColor = QColor(240, 240, 240);
  backgroundColor = QColor(51, 55, 61);
  backgroundBorderColor = QColor(80, 80, 90);
  selectdBorderColor = QColor(255, 255, 255);
  selectdBorderHighLightColor = QColor(255, 255, 255);
  notSelectdColor = QColor(0, 0, 0, 0);
  notSelectdBorderColor = QColor(0, 0, 0, 0);
  hoverColor = QColor(80, 100, 120);

  this->setSelectdColor(QColor(0, 120, 215));

  // 初始化动画
  hoverAnimation = new QVariantAnimation(this);
  hoverAnimation->setEasingCurve(QEasingCurve::OutQuad);
  hoverAnimation->setDuration(animationDuration);
  connect(hoverAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
    hoverProgress = value.toDouble();
    update();
    });

  selectionAnimation = new QVariantAnimation(this);
  selectionAnimation->setEasingCurve(QEasingCurve::OutQuad);
  selectionAnimation->setDuration(animationDuration);
  connect(selectionAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
    selectionProgress = value.toDouble();
    update();
    });

  setMouseTracking(true);
}

QWWindowButton::~QWWindowButton() {
  delete hoverAnimation;
  delete selectionAnimation;
}

void QWWindowButton::paintEvent(QPaintEvent* event) {
  if (UnitList.isEmpty()) {
    return;
  }

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  QFont originalFont = font();
  painter.setFont(originalFont);

  // 绘制背景
  if (drawBackground ) {
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

  // 预先计算所有单元位置
  calculateUnitRects();

  switch (viewMode) {
  case ViewMode::List: {
    // 1. 先绘制所有未选中的单元
    for (int i = 0; i < UnitList.size(); i++) {
      if (selectionMode == SelectionMode::Single && i == currentUnitIndex) {
        continue; // 跳过当前选中的单元
      }
      if (selectionMode == SelectionMode::Multiple && (currentMultiUnitIndex & (1 << i))) {
        continue; // 跳过选中的单元（多选模式）
      }

      QRectF rect = unitRects[i];
      painter.setBrush(notSelectdColor);
      painter.setPen(drawNotSelectdUnitBorder ? QPen(notSelectdBorderColor, unitBorderLineWidth) : Qt::NoPen);

      QPainterPath unitPath;
      unitPath.addRoundedRect(rect, unitBorderRadius, unitBorderRadius);
      painter.drawPath(unitPath);

      painter.setPen(TextColor);
      painter.drawText(rect, Qt::AlignCenter, UnitList.at(i));
    }

    // 2. 绘制选中单元（带动画效果）
    if (selectionMode == SelectionMode::Single && currentUnitIndex >= 0 && currentUnitIndex < UnitList.size()) {
      QRectF targetRect = unitRects[currentUnitIndex];

      if (animationStartIndex >= 0 && animationTargetIndex >= 0 && selectionProgress < 1.0) {
        QRectF startRect = unitRects[animationStartIndex];
        double x = startRect.x() + (targetRect.x() - startRect.x()) * selectionProgress;
        double y = startRect.y() + (targetRect.y() - startRect.y()) * selectionProgress;
        double w = startRect.width() + (targetRect.width() - startRect.width()) * selectionProgress;
        double h = startRect.height() + (targetRect.height() - startRect.height()) * selectionProgress;
        targetRect = QRectF(x, y, w, h);
      }

      painter.setBrush(selectdColor);
      painter.setPen(drawSelectdUnitBorder ? QPen(selectdBorderColor, unitBorderLineWidth) : Qt::NoPen);

      QPainterPath selectedPath;
      selectedPath.addRoundedRect(targetRect, unitBorderRadius, unitBorderRadius);
      painter.drawPath(selectedPath);

      // 只在动画完成或没有动画时绘制文字，避免文字重叠hoverAnimationIndex
      if ((selectionProgress >= 1.0 || animationStartIndex < 0) ) {

        painter.setPen(TextColor);
        painter.drawText(targetRect, Qt::AlignCenter, UnitList.at(currentUnitIndex));
      }
    }

    // 3. 绘制多选模式下的选中单元
    if (selectionMode == SelectionMode::Multiple) {
      for (int i = 0; i < UnitList.size(); i++) {
        if (currentMultiUnitIndex & (1 << i)) {
          QRectF rect = unitRects[i];
          painter.setBrush(selectdColor);
          painter.setPen(drawSelectdUnitBorder ? QPen(selectdBorderColor, unitBorderLineWidth) : Qt::NoPen);

          QPainterPath path;
          path.addRoundedRect(rect, unitBorderRadius, unitBorderRadius);
          painter.drawPath(path);

          painter.setPen(TextColor);
          painter.drawText(rect, Qt::AlignCenter, UnitList.at(i));
        }
      }
    }

    // 4. 最后绘制悬停效果（确保在最上层）
    if (hoverUnitIndex >= 0 && hoverUnitIndex < UnitList.size() && hoverProgress > 0) {
      QRectF hoverRect = unitRects[hoverUnitIndex];

      // 应用悬停动画效果
      double scale = 1.0 + 0.1 * hoverProgress;
      double offsetX = hoverRect.width() * (1 - scale) / 2;
      double offsetY = hoverRect.height() * (1 - scale) / 2;

      QRectF scaledRect(hoverRect.x() + offsetX,
        hoverRect.y() + offsetY,
        hoverRect.width() * scale,
        hoverRect.height() * scale);

      // 绘制悬停背景
      painter.setBrush(hoverColor);
      painter.setPen(Qt::NoPen);
      QPainterPath hoverPath;
      hoverPath.addRoundedRect(scaledRect, unitBorderRadius, unitBorderRadius);
      painter.drawPath(hoverPath);

      // 绘制悬停文字（确保不会与其他文字重叠）
      QFont hoverFont = originalFont;
      hoverFont.setPointSizeF(originalFont.pointSizeF() * (1.0 + 0.2 * hoverProgress));
      painter.setFont(hoverFont);
      painter.setPen(TextColor);
      painter.drawText(scaledRect, Qt::AlignCenter, UnitList.at(hoverUnitIndex));

      // 恢复原始字体
      painter.setFont(originalFont);
    }
  } break;

  case ViewMode::Single: {
    if (currentUnitIndex < 0 || currentUnitIndex >= UnitList.size())
      break;

    int interval = (unitBorderLineWidth + backgroundBorderLineWidth) / 2.0 + intervalDistance;

    painter.setBrush(selectdColor);
    QPainterPath path;
    QRect rect(interval, interval, width() - 2 * interval, height() - 2 * interval);
    path.addRoundedRect(rect, unitBorderRadius, unitBorderRadius);
    painter.drawPath(path);

    painter.setPen(TextColor);
    painter.drawText(rect, Qt::AlignCenter, UnitList.at(currentUnitIndex));
  } break;

  default:
    break;
  }
}

void QWWindowButton::resizeEvent(QResizeEvent* event) {
  calculateUnitRects(); // 重新计算单元位置
  update();
}

void QWWindowButton::mousePressEvent(QMouseEvent* event) {
  if (isEnabled()) {
    if (event->buttons() & Qt::LeftButton) {
      event->accept();
      mouseClicked = true;
    }
    else {
      event->ignore();
    }
  }
}

void QWWindowButton::mouseMoveEvent(QMouseEvent* event) {

  int oldHoverIndex = hoverUnitIndex;
  hoverUnitIndex = getMousePosInUnit();

  if (oldHoverIndex != hoverUnitIndex) {
    updateHoverAnimation(hoverUnitIndex);
  }
}

void QWWindowButton::mouseReleaseEvent(QMouseEvent* event) {

  if (isEnabled()) {
    if ((event->type() == QMouseEvent::MouseButtonRelease) &&
      (event->button() == Qt::LeftButton)) {
      event->accept();
      if (mouseClicked) {
        mouseClicked = false;

        if (!allowMouseClicked) {
          return;
        }

        int unitIndexFromMouse = getMousePosInUnit();
        //qDebug() << "unitIndexFromMouse:" << (int)unitIndexFromMouse;

        if (selectionMode == SelectionMode::Single) {
          if (unitIndexFromMouse != currentUnitIndex && unitIndexFromMouse >= 0) {
            updateSelectionAnimation(unitIndexFromMouse);
            emit selectUnitIndexChanged(unitIndexFromMouse);
          }
        }
        else if (selectionMode == SelectionMode::Multiple) {
          unsigned int newIndex = currentMultiUnitIndex | (1 << unitIndexFromMouse);

          if (newIndex != currentMultiUnitIndex) {
            this->setMultiSelectUnitIndex(newIndex);
            emit selectUnitIndexChanged(newIndex);
          }
        }
      }
    }
    else {
      event->ignore();
    }
  }
}

void QWWindowButton::leaveEvent(QEvent* e) {
  startHoverAnimation(-1, hoverProgress, 0.0);
  update();

  QWidget::leaveEvent(e);
}

void QWWindowButton::enterEvent(QEnterEvent*) {
  // 不需要额外处理
}

void QWWindowButton::calculateUnitRects() {
  unitRects.clear();
  if (UnitList.isEmpty())
    return;

  int totalSize = UnitList.size();
  int interval = (unitBorderLineWidth + backgroundBorderLineWidth) / 2.0 + intervalDistance;

  if (direction == Direction::Horizontal) {
    double unitHeight = height() - 2 * interval;
    double unitWidth = 0;

    if (unitSizeMode == UnitSizeMode::Stretch) {
      // 自适应宽度均分
      unitWidth = (width() - (totalSize + 1) * interval) / static_cast<double>(totalSize);
    }
    else {
      // 固定宽度
      unitWidth = fixedUnitSize;
    }

    for (int i = 0; i < totalSize; i++) {
      double x = interval + i * (unitWidth + interval);
      double y = interval;
      unitRects.append(QRectF(x, y, unitWidth, unitHeight));
    }
  }
  else { // Vertical
    double unitWidth = width() - 2 * interval;
    double unitHeight = 0;

    if (unitSizeMode == UnitSizeMode::Stretch) {
      // 自适应高度均分
      unitHeight = (height() - (totalSize + 1) * interval) / static_cast<double>(totalSize);
    }
    else {
      // 固定高度
      unitHeight = fixedUnitSize;
    }

    for (int i = 0; i < totalSize; i++) {
      double x = interval;
      double y = interval + i * (unitHeight + interval);
      unitRects.append(QRectF(x, y, unitWidth, unitHeight));
    }
  }
}

int QWWindowButton::getMousePosInUnit() {
  if (UnitList.isEmpty()) {
    return 0;
  }

  // 如果没有计算过单元位置，先计算
  if (unitRects.size() != UnitList.size()) {
    calculateUnitRects();
  }

  QPoint pos = this->mapFromGlobal(QCursor::pos());

  for (int i = 0; i < unitRects.size(); i++) {
    if (unitRects[i].contains(pos)) {
      return i;
    }
  }

  return -1;
}

void QWWindowButton::updateHoverAnimation(int index) {
  if (hoverAnimation->state() == QAbstractAnimation::Running) {
    hoverAnimation->stop();
  }

  if (index >= 0) {
    // 鼠标进入新单元
    hoverAnimation->setStartValue(0.0);
    hoverAnimation->setEndValue(1.0);
    hoverAnimation->start();
    hoverAnimationIndex = index;
  }
  else {
    // 鼠标离开当前单元
    hoverAnimation->setStartValue(hoverProgress);
    hoverAnimation->setEndValue(0.0);
    hoverAnimation->start();
    hoverAnimationIndex = -1;
  }
}

void QWWindowButton::updateSelectionAnimation(int targetIndex) {
  if (targetIndex < 0 || targetIndex >= UnitList.size() || targetIndex == currentUnitIndex) {
    return;
  }

  // 保存当前索引作为动画起点
  animationStartIndex = currentUnitIndex;
  animationTargetIndex = targetIndex;

  // 更新当前索引（实际值立即改变，但显示有动画）
  currentUnitIndex = targetIndex;

  // 启动动画
  if (selectionAnimation->state() == QAbstractAnimation::Running) {
    selectionAnimation->stop();
  }

  selectionAnimation->setStartValue(0.0);
  selectionAnimation->setEndValue(1.0);
  selectionAnimation->start();

  hoverAnimationIndex = targetIndex;
}

void QWWindowButton::addUnit(QString UnitName) {
  if (UnitList.isEmpty()) {
    currentUnitIndex = 0;
    animationStartIndex = -1;
    animationTargetIndex = 0;
    selectionProgress = 1.0;
  }

  UnitList.append(UnitName);
  adjustMinimumSize();
  calculateUnitRects();
  update();
}

void QWWindowButton::delUnit(int index) {
  if (index >= 0 && index < UnitList.size()) {
    if (currentUnitIndex == index) {
      currentUnitIndex = qMax(0, UnitList.size() - 2);
    }

    UnitList.removeAt(index);
    adjustMinimumSize();
    calculateUnitRects();
    update();
  }
}

void QWWindowButton::clearUnit() {
  currentUnitIndex = -1;

  UnitList.clear();
  adjustMinimumSize();
  calculateUnitRects();
  update();
}

void QWWindowButton::setSelectUnit(const QString& name) {
  setSelectUnitIndex(UnitList.indexOf(name));
}

void QWWindowButton::setSelectUnitIndex(int index) {
  if (UnitList.isEmpty()) {
    currentUnitIndex = -1;
    update();
    return;
  }
  if (index < 0) {
    // 清除选中状态
    if (currentUnitIndex != -1) {
      currentUnitIndex = -1;
      update();
    }
    return;
  }
  else if (index >= UnitList.size()) {
    index = UnitList.size() - 1;
  }

  if (index != currentUnitIndex) {
    updateSelectionAnimation(index);
  }
  else {
    currentUnitIndex = index;
    update();
  }
}

int QWWindowButton::getCurrentUnitIndex() { return currentUnitIndex; }

void QWWindowButton::setMultiSelectUnitIndex(unsigned int index) {
  if (UnitList.size() == 0) {
    return;
  }
  currentMultiUnitIndex = index & ((1 << UnitList.size()) - 1);;

  update();
}
int QWWindowButton::getMultiCurrentUnitIndex() {
  return currentMultiUnitIndex;
}

void QWWindowButton::setAllowMouseClicked(bool ok) {
	allowMouseClicked = ok;
}

bool QWWindowButton::getAllowMouseClicked() {
	return allowMouseClicked;
}

void QWWindowButton::setAutoAdjustMinimumSize(bool ok) {
  autoAdjustMinimumSize = ok;
}

bool QWWindowButton::getAutoAdjustMinimumSize() {
  return autoAdjustMinimumSize;
}

void QWWindowButton::setIntervalDistance(int distance) {
  intervalDistance = distance;
}
int QWWindowButton::getIntervalDistance() {
  return intervalDistance;
}

void QWWindowButton::setFixedUnitSize(int size) {
  fixedUnitSize = size;
}
int QWWindowButton::getFixedUnitSize() {
  return fixedUnitSize;
}

void QWWindowButton::setViewMode(QWWindowButton::ViewMode d) {
	viewMode = d;
	update();
}

QWWindowButton::ViewMode QWWindowButton::getViewMode() {
	return viewMode;
}

void QWWindowButton::setSelectionMode(QWWindowButton::SelectionMode d) {
	selectionMode = d;
	update();
}

QWWindowButton::SelectionMode QWWindowButton::getSelectionMode() {
	return selectionMode;
}

void QWWindowButton::setDirection(QWWindowButton::Direction d) {
	direction = d;
	update();
}

QWWindowButton::Direction QWWindowButton::getDirection() {
	return direction;
}

void QWWindowButton::setUnitSizeMode(QWWindowButton::UnitSizeMode d) {
  unitSizeMode = d;
  update();
}

QWWindowButton::UnitSizeMode QWWindowButton::getUnitSizeMode() {
  return unitSizeMode;
}

void QWWindowButton::setSelectdColor(QColor c) {
  selectdColor = c;
  selectdHighLightColor = c;
  selectdHighLightColor.setAlpha(100);
}
QColor QWWindowButton::getSelectdColor() { return selectdColor; }

void QWWindowButton::setBackgroundColor(QColor c) { backgroundColor = c; }

QColor QWWindowButton::getBackgroundColor() { return backgroundColor; }

void QWWindowButton::setUnitName(int index, QString UnitName) {
  if (index > 0 && index < UnitList.size()) {
    UnitList[index] = UnitName;
  }
}
QString QWWindowButton::getUnitName(int index) {
  if (index >= 0 && index < UnitList.size()) {
    return UnitList.at(index);

  }
  else {
    return QString();
  }
}

int QWWindowButton::getUnitSize() const { return UnitList.size(); }

void QWWindowButton::setAnimationDuration(int duration) {
  if (duration > 0) {
    animationDuration = duration;
    hoverAnimation->setDuration(duration);
    selectionAnimation->setDuration(duration);
  }
}

int QWWindowButton::getAnimationDuration() const {
  return animationDuration;
}

void QWWindowButton::startHoverAnimation(int index, double startValue, double endValue) {
  if (hoverAnimation->state() == QAbstractAnimation::Running) {
    hoverAnimation->stop();
  }

  hoverAnimation->setStartValue(startValue);
  hoverAnimation->setEndValue(endValue);
  hoverAnimation->start();

  hoverUnitIndex = index;
}

void QWWindowButton::startSelectionAnimation(double startValue, double endValue) {
  if (selectionAnimation->state() == QAbstractAnimation::Running) {
    selectionAnimation->stop();
  }

  selectionAnimation->setStartValue(startValue);
  selectionAnimation->setEndValue(endValue);
  selectionAnimation->start();
}

void QWWindowButton::setHoverColor(QColor c) {
  hoverColor = c;
  update();
}

QColor QWWindowButton::getHoverColor() const {
  return hoverColor;
}

void QWWindowButton::adjustMinimumSize() {
  if (autoAdjustMinimumSize) {
    int interval = (unitBorderLineWidth + backgroundBorderLineWidth) / 2.0 + intervalDistance;
    int minSize = interval;
    minSize += UnitList.size() * (fixedUnitSize + interval);
    if (direction == Direction::Horizontal) {
      setMinimumWidth(minSize);
    }else {
      setMinimumHeight(minSize);
    }
  }
}

// 其他成员函数保持不变（略）