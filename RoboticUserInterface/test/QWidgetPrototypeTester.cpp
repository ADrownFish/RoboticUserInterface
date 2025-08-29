#include "QWidgetPrototypeTester.h"



#include <QGridLayout>

QWRangeSelector::QWRangeSelector(QWidget* parent)
  : QWidget(parent)
{
  initUi();
  initConnections();
  setMode(Percentage); // 默认模式为百分比
  setRange(0.0, 100.0);
}

QWRangeSelector::~QWRangeSelector()
{
}

// 初始化 UI 元素
void QWRangeSelector::initUi()
{
  // 创建输入框
  m_startEdit = new QLineEdit(this);
  m_endEdit = new QLineEdit(this);

  // 设置输入框样式：扁平暗色背景
  QString editStyle = "QLineEdit { background: #333; color: white; border: none; padding: 4px; }";
  m_startEdit->setStyleSheet(editStyle);
  m_endEdit->setStyleSheet(editStyle);

  // 占位提示（使用 tr() 以支持翻译）
  m_startEdit->setPlaceholderText(tr("Start"));
  m_endEdit->setPlaceholderText(tr("End"));

  // 创建轨道绘制部件
  m_trackWidget = new RangeTrack(this);
  m_trackWidget->setFixedHeight(50); // 初始高度，可根据需要调整
  m_trackWidget->setCursor(Qt::PointingHandCursor);

  // 布局：左输入框、轨道、右输入框
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(8, 8, 8, 8);
  layout->setSpacing(8);
  layout->addWidget(m_startEdit);
  layout->addWidget(m_trackWidget, 1); // 轨道区域扩展拉伸
  layout->addWidget(m_endEdit);

  // 设置背景色为暗色
  this->setStyleSheet("background-color: #222;");
  this->setLayout(layout);
}

// 初始化信号与槽连接
void QWRangeSelector::initConnections()
{
  // 输入框编辑完成后更新轨道游标
  connect(m_startEdit, &QLineEdit::editingFinished, this, &QWRangeSelector::onStartEditingFinished);
  connect(m_endEdit, &QLineEdit::editingFinished, this, &QWRangeSelector::onEndEditingFinished);

  // 轨道游标位置改变后更新输入框
  connect(m_trackWidget, &RangeTrack::valuesChanged, this, [this](double left, double right) {
    // 根据模式格式化值
    double v1 = (m_mode == Percentage ? left * 100.0 : m_minVal + left * (m_maxVal - m_minVal));
    double v2 = (m_mode == Percentage ? right * 100.0 : m_minVal + right * (m_maxVal - m_minVal));
    m_startEdit->setText(valueToString(v1));
    m_endEdit->setText(valueToString(v2));
    emit rangeChanged(v1, v2);
    });
}

// 设置模式（百分比或数值）
void QWRangeSelector::setMode(Mode m)
{
  if (m_mode == m) return;
  m_mode = m;
  // 更新输入框内容格式，保持当前比例对应的值
  double left = m_trackWidget->leftValue();
  double right = m_trackWidget->rightValue();
  double v1 = (m_mode == Percentage ? left * 100.0 : m_minVal + left * (m_maxVal - m_minVal));
  double v2 = (m_mode == Percentage ? right * 100.0 : m_minVal + right * (m_maxVal - m_minVal));
  m_startEdit->setText(valueToString(v1));
  m_endEdit->setText(valueToString(v2));
}

// 设置轨道高度
void QWRangeSelector::setTrackHeight(int h)
{
  m_trackWidget->setTrackHeight(h);
}

// 设置游标宽高
void QWRangeSelector::setCursorSize(int w, int h)
{
  m_trackWidget->setCursorSize(w, h);
}

// 设置数值范围
void QWRangeSelector::setRange(double minVal, double maxVal)
{
  if (maxVal <= minVal) return;
  m_minVal = minVal;
  m_maxVal = maxVal;
  m_trackWidget->setValueRange(minVal, maxVal);
}

// 获取起始值
double QWRangeSelector::startValue() const
{
  double left = m_trackWidget->leftValue();
  return (m_mode == Percentage ? left * 100.0 : m_minVal + left * (m_maxVal - m_minVal));
}

// 获取结束值
double QWRangeSelector::endValue() const
{
  double right = m_trackWidget->rightValue();
  return (m_mode == Percentage ? right * 100.0 : m_minVal + right * (m_maxVal - m_minVal));
}

// 设置起始值
void QWRangeSelector::setStartValue(double v)
{
  // 将值映射到 0-1 范围
  double ratio = (m_mode == Percentage ? v / 100.0 : (v - m_minVal) / (m_maxVal - m_minVal));
  m_trackWidget->setLeftValue(ratio);
  m_startEdit->setText(valueToString(v));
}

// 设置结束值
void QWRangeSelector::setEndValue(double v)
{
  double ratio = (m_mode == Percentage ? v / 100.0 : (v - m_minVal) / (m_maxVal - m_minVal));
  m_trackWidget->setRightValue(ratio);
  m_endEdit->setText(valueToString(v));
}

// 输入框编辑完成：更新左游标位置
void QWRangeSelector::onStartEditingFinished()
{
  double v = stringToValue(m_startEdit->text());
  setStartValue(v);
}

// 输入框编辑完成：更新右游标位置
void QWRangeSelector::onEndEditingFinished()
{
  double v = stringToValue(m_endEdit->text());
  setEndValue(v);
}

// 将数字值转换为字符串，保留两位小数
QString QWRangeSelector::valueToString(double v) const
{
  return QString::number(v, 'f', 2);
}

// 将字符串转换为数字
double QWRangeSelector::stringToValue(const QString& s) const
{
  bool ok;
  double v = s.toDouble(&ok);
  if (!ok) return 0.0;
  // 在百分比模式下限制 0-100
  if (m_mode == Percentage) {
    if (v < 0) v = 0;
    if (v > 100) v = 100;
  }
  else {
    if (v < m_minVal) v = m_minVal;
    if (v > m_maxVal) v = m_maxVal;
  }
  return v;
}

// ===================== RangeTrack 内部类实现 =====================

RangeTrack::RangeTrack(QWidget* parent)
  : QWidget(parent)
{
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
  // 初始化游标缩放比例
  m_cursorScale[0] = m_cursorScale[1] = 1.0;
  // 创建动画对象
  m_anim[0] = new QPropertyAnimation(this, "cursorScale0", this);
  m_anim[1] = new QPropertyAnimation(this, "cursorScale1", this);
  for (int i = 0; i < 2; ++i) {
    m_anim[i]->setDuration(200);
  }
}

// 绘制轨道和游标
void RangeTrack::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  // 绘制背景轨道
  QRectF trackRect(0, (height() - m_trackHeight) / 2, width(), m_trackHeight);
  p.setPen(Qt::NoPen);
  p.setBrush(QColor("#555")); // 深灰色轨道
  p.drawRect(trackRect);

  // 绘制游标
  for (int i = 0; i < 2; ++i) {
    double ratio = (i == 0 ? m_leftPos : m_rightPos);
    double x = ratio * width();
    // 根据动画缩放比例调整尺寸
    int cw = int(m_cursorWidth * m_cursorScale[i]);
    int ch = int(m_cursorHeight * m_cursorScale[i]);
    // 竖直居中位置
    int topY = (height() - ch) / 2;
    QRectF vertLine(x - cw / 2, topY, cw, ch);
    // 水平短线长度
    int hl = cw * 3;
    // 上下小横线
    QPointF topLeft(x - hl / 2, topY);
    QPointF topRight(x + hl / 2, topY);
    QPointF bottomLeft(x - hl / 2, topY + ch);
    QPointF bottomRight(x + hl / 2, topY + ch);
    // 半透明效果：选中时降低透明度
    if (i == m_selectedCursor) {
      p.setOpacity(0.5);
    }
    else {
      p.setOpacity(1.0);
    }
    p.setPen(QPen(QColor("#DDD"), cw)); // 浅灰色竖线
    p.drawLine(QPointF(x, topY), QPointF(x, topY + ch));
    p.setPen(QPen(QColor("#DDD"), 1));  // 浅灰色横线
    p.drawLine(topLeft, topRight);
    p.drawLine(bottomLeft, bottomRight);
    p.setOpacity(1.0);
  }
}

// 更新游标位置辅助
void RangeTrack::updateCursorPos(int idx, double ratio)
{
  // 限制在0-1范围内
  if (ratio < 0.0) ratio = 0.0;
  if (ratio > 1.0) ratio = 1.0;
  // 确保左游标不超过右游标
  if (idx == 0 && ratio > m_rightPos) ratio = m_rightPos;
  if (idx == 1 && ratio < m_leftPos) ratio = m_leftPos;
  if (idx == 0) m_leftPos = ratio;
  else m_rightPos = ratio;
  update();
  emit valuesChanged(m_leftPos, m_rightPos);
}

// 鼠标按下事件：检测是否选中游标
void RangeTrack::mousePressEvent(QMouseEvent* event)
{
  double x = event->position().x();
  // 计算两个游标的横坐标
  double pos0 = m_leftPos * width();
  double pos1 = m_rightPos * width();
  // 选中范围： ± 游标宽度附近
  int clickRange = m_cursorWidth * 4;
  if (qAbs(x - pos0) < clickRange) {
    m_selectedCursor = 0;
  }
  else if (qAbs(x - pos1) < clickRange) {
    m_selectedCursor = 1;
  }
  else {
    m_selectedCursor = -1;
  }
  if (m_selectedCursor != -1) {
    m_dragging = true;
    update();
  }
  QWidget::mousePressEvent(event);
}

// 鼠标移动事件：处理拖动与悬停
void RangeTrack::mouseMoveEvent(QMouseEvent* event)
{
  double x = event->position().x();
  // 如果正在拖动某个游标，更新位置
  if (m_dragging && m_selectedCursor != -1) {
    double ratio = x / width();
    updateCursorPos(m_selectedCursor, ratio);
  }

  // 悬停检测：判断是否移到某个游标附近
  int oldHover = m_hoveredCursor;
  int hoverRange = m_cursorWidth * 4;
  if (qAbs(x - m_leftPos * width()) < hoverRange) {
    m_hoveredCursor = 0;
  }
  else if (qAbs(x - m_rightPos * width()) < hoverRange) {
    m_hoveredCursor = 1;
  }
  else {
    m_hoveredCursor = -1;
  }
  // 游标悬停状态改变时启动动画
  if (oldHover != m_hoveredCursor) {
    // 对之前悬停的游标缩小
    if (oldHover != -1) {
      m_anim[oldHover]->stop();
      m_anim[oldHover]->setStartValue(m_cursorScale[oldHover]);
      m_anim[oldHover]->setEndValue(1.0);
      m_anim[oldHover]->start();
    }
    // 对当前悬停的游标放大
    if (m_hoveredCursor != -1) {
      m_anim[m_hoveredCursor]->stop();
      m_anim[m_hoveredCursor]->setStartValue(m_cursorScale[m_hoveredCursor]);
      m_anim[m_hoveredCursor]->setEndValue(1.3);
      m_anim[m_hoveredCursor]->start();
    }
  }
  QWidget::mouseMoveEvent(event);
}

// 鼠标释放事件：停止拖动
void RangeTrack::mouseReleaseEvent(QMouseEvent* event)
{
  Q_UNUSED(event);
  m_dragging = false;
  // 保持游标选中状态（可继续用键盘调整）
  update();
}

// 键盘事件：使用左右键微调选中游标
void RangeTrack::keyPressEvent(QKeyEvent* event)
{
  if (m_selectedCursor != -1) {
    if (event->key() == Qt::Key_Left) {
      double step = 0.01; // 微调步长
      double newPos = (m_selectedCursor == 0 ? m_leftPos - step : m_rightPos - step);
      updateCursorPos(m_selectedCursor, newPos);
    }
    else if (event->key() == Qt::Key_Right) {
      double step = 0.01;
      double newPos = (m_selectedCursor == 0 ? m_leftPos + step : m_rightPos + step);
      updateCursorPos(m_selectedCursor, newPos);
    }
  }
  QWidget::keyPressEvent(event);
}

// 事件总线：监听鼠标进入/离开轨道部件
bool RangeTrack::event(QEvent* event)
{
  if (event->type() == QEvent::Enter) {
    // 鼠标进入轨道区域
    setCursor(Qt::PointingHandCursor);
  }
  else if (event->type() == QEvent::Leave) {
    // 鼠标离开轨道区域：如果曾悬停，缩小回来
    if (m_hoveredCursor != -1) {
      int idx = m_hoveredCursor;
      m_anim[idx]->stop();
      m_anim[idx]->setStartValue(m_cursorScale[idx]);
      m_anim[idx]->setEndValue(1.0);
      m_anim[idx]->start();
      m_hoveredCursor = -1;
    }
    setCursor(Qt::ArrowCursor);
  }
  return QWidget::event(event);
}

// 设置左游标的归一化位置
void RangeTrack::setLeftValue(double v)
{
  updateCursorPos(0, v);
}

// 设置右游标的归一化位置
void RangeTrack::setRightValue(double v)
{
  updateCursorPos(1, v);
}


QWidgetPrototypeTester::QWidgetPrototypeTester(QWidget * parent)
	: QWidget(parent) {
	combobox = new QtMaterialComboBox(this);
	
	QGridLayout* grid = new QGridLayout(this);
	this->setLayout(grid);

	grid->addWidget(combobox);
  grid->addWidget(new QWRangeSelector);
}
