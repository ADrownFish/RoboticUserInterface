#include "qwswitcherbutton.h"

QWSwitcherButton::QWSwitcherButton(QWidget* parent)
    : QWidget(parent)
{
	
}


QList<QWSwitcherButton::StateItem>& QWSwitcherButton::getStateItemList()
{
    return _stateItemList;
}

void QWSwitcherButton::appendState(const StateItem& item){
  _stateItemList.append(item);
  update();
}

int QWSwitcherButton::stateCount() {
    return _stateItemList.size();
}

void QWSwitcherButton::setSelectState(int value) {
    if (value < 0 || value >= _stateItemList.size()) {
        _currentState = 0;
    }

    _currentState = value;
    selectIndexChanged(_currentState);
    update();
}

int QWSwitcherButton::getSelectState(){
    return _currentState;
}

void QWSwitcherButton::setBorderRadius(int value){
    _borderRadius = value;
    update();
}

int QWSwitcherButton::getBorderRadius(){
    return _borderRadius;
}

void QWSwitcherButton::setPenWidth(int value){
    _penWidth = value;
    update();
}

int QWSwitcherButton::getPenWidth(){
    return _penWidth;
}

void QWSwitcherButton::setTextColor(QColor value){
    _textColor = value;
    update();
}

QColor QWSwitcherButton::getTextColor(){
    return _textColor;
}

void QWSwitcherButton::setBorderColor(QColor value){
    _borderColor = value;
    update();
}

QColor QWSwitcherButton::getBorderColor(){
    return _borderColor;
}


void QWSwitcherButton::setBorder(bool ok){
    _border = ok;
    update();
}

bool QWSwitcherButton::getBorder(){
    return _border;
}

QWSwitcherButton::StateItem& QWSwitcherButton::state(int index){
    if (index < 0)    {
        return _stateItemList[0];
    } else if(index + 1 > _stateItemList.size())    {
        return _stateItemList.last();
    }
    return _stateItemList[index];
}

// 绘制开关
void QWSwitcherButton::paintEvent(QPaintEvent* event){
  Q_UNUSED(event)
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QRect borderRect = rect().adjusted(_penWidth, _penWidth, -_penWidth, -_penWidth); // 调整边界以防止绘制在窗口外部
  if (_stateItemList.isEmpty()) {
    return;
  }
  StateItem& curState = _stateItemList[_currentState];
  //curState.pixmap.fill(QColor(200, 150, 120));

  if (_hover)
  {
    QColor color = curState.color;
    color.setAlpha(150);
    painter.setBrush(color);
  }
  else
  {
    painter.setBrush(QColor(0, 0, 0, 0));
  }
  if (_border)
  {
    painter.setPen(QPen(_borderColor, _penWidth));
  }
  else
  {
    painter.setPen(Qt::NoPen);
  }

  painter.drawRoundedRect(borderRect, _borderRadius, _borderRadius);

  // 计算正方形和长方形的矩形区域
  int widgetWidth = width();
  int widgetHeight = height();

  // 计算正方形的大小，不超过窗口宽度的50%

  int squareSize = qMin(widgetWidth / 2, widgetHeight) * 0.9; // 取宽度和高度的最小值作为正方形的边长
  int squareOffset = squareSize * 0.05; // 正方形右边的偏移量
  int rectangleWidth = widgetWidth - squareSize - squareOffset * 2; // 长方形的宽度等于 widget 宽度减去正方形的宽度和两个偏移量

  // 计算正方形和长方形的起始位置，加上额外的偏移量
  int squareX = squareOffset;
  int rectangleX = squareSize + squareOffset * 2; // 长方形的起始位置在正方形右边再加上一个偏移量

  QRect squareRect(squareX, (widgetHeight - squareSize) / 2, squareSize, squareSize); // 让正方形位于垂直方向中间
  QRect rectangleRect(rectangleX, (widgetHeight - squareSize) / 2, rectangleWidth - squareOffset, squareSize); // 让长方形位于垂直方向中间

  // 绘制正方形
  painter.setBrush(Qt::NoBrush);
  int adjustedWidth = squareSize * 0.2;
  squareRect = squareRect.adjusted(adjustedWidth, adjustedWidth, -adjustedWidth, -adjustedWidth);
  _stateItemList[_currentState].svg->render(&painter, squareRect);

  // 绘制长方形
  painter.setPen(QPen(QColor(255, 255, 255)));
  painter.setFont(font());
  painter.drawText(rectangleRect.adjusted(_penWidth, _penWidth, -_penWidth, -_penWidth), Qt::AlignCenter, curState.name);
}

// 鼠标按下事件
void QWSwitcherButton::mousePressEvent(QMouseEvent* event){
    if (isEnabled()) {
        if (event->buttons() & Qt::LeftButton) {
            event->accept();
        }
        else {
            event->ignore();
        }
    }
}
// 鼠标释放事件 - 切换开关状态、发射toggled()信号
void QWSwitcherButton::mouseReleaseEvent(QMouseEvent* event)
{
    if (isEnabled()) {
        if ((event->type() == QMouseEvent::MouseButtonRelease) && (event->button() == Qt::LeftButton)) {
            event->accept();

            _currentState++;
            if (_currentState >= _stateItemList.size())
            {
                _currentState = 0;
            }
            selectIndexChanged(_currentState);
            update();
        }
        else {
            event->ignore();
        }
    }
}

// 大小改变事件
void QWSwitcherButton::resizeEvent(QResizeEvent* event)
{
    update();
    QWidget::resizeEvent(event);
}

