#include "qwswitcher.h"

QWSwitcher::QWSwitcher(QWidget* parent)
    : QWidget(parent),
    mChecked(true),

    m_BorderColor(120, 120, 120),
    m_Background(0, 0, 0, 0),
    m_CheckedBackground(0, 0, 0, 0),
    m_CheckedBall(4, 186, 189),
    m_CheckBall(90, 76, 160),
    m_Radius(32),
    m_FinalX(0)
{
    resize(150, 75);
    m_penWidth = 2;
  
    m_NowX = width() - m_Radius_pos - m_Radius / 2 ;
    setCursor(Qt::PointingHandCursor);

    // 连接信号槽
    connect(&m_timer, &QTimer::timeout, this, &QWSwitcher::onTimeout);
    m_timer.start(10);
}

// 绘制开关
void QWSwitcher::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event)

    QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QColor thumbColor;
  QColor bgColor;
  if (isEnabled()) { // 可用状态
    if (mChecked) { // 打开状态
      thumbColor = m_CheckedBall;
      bgColor = m_CheckedBackground;
    }
    else { //关闭状态
      thumbColor = m_CheckBall;
      bgColor = m_Background;
    }
  }
  else {  // 不可用状态
    thumbColor = QColor(255, 255, 255);
    bgColor.setAlpha((150));
  }

  // 绘制大椭圆
  painter.setBrush(bgColor);
  painter.setPen(QPen(m_BorderColor, m_penWidth, Qt::SolidLine));

  QPainterPath path;
  path.addRoundedRect(border_rect, m_Radius_pos, m_Radius_pos);
  painter.drawPath(path);

  // 绘制小椭圆
  painter.setBrush(thumbColor);
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(m_NowX, (height() - m_Radius) / 2, m_Radius, m_Radius);
}

// 鼠标按下事件
void QWSwitcher::mousePressEvent(QMouseEvent* event)
{
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
void QWSwitcher::mouseReleaseEvent(QMouseEvent* event)
{
    if (isEnabled()) {
        if ((event->type() == QMouseEvent::MouseButtonRelease) && (event->button() == Qt::LeftButton)) {
            event->accept();
            mChecked = !mChecked;
            emit toggled(mChecked);
            m_timer.start(10);
        }
        else {
            event->ignore();
        }
    }
}

// 大小改变事件
void QWSwitcher::resizeEvent(QResizeEvent* event)
{
    m_Radius_pos = qMin(width(), height()) / 2; // 使用最小边的一半作为半径
    border_rect = QRect(m_penWidth, m_penWidth, width() - 2 * m_penWidth, height() - 2 * m_penWidth);
    m_Radius = m_Radius_pos;  //更新球大小

    // updateCheckedBallPos();
    m_timer.start(10);

    update();
    QWidget::resizeEvent(event);
}

// 切换状态 - 滑动
void QWSwitcher::onTimeout(){
  updateCheckedBallPos();
  update();
}

void QWSwitcher::updateCheckedBallPos()
{
    int width = this->width();
    int height = this->height();

    if (mChecked)
    {
        m_FinalX = border_rect.right() - m_Radius_pos - m_Radius / 2;

        m_NowX += width / 20.f;
        if (m_NowX > m_FinalX)
        {
            m_NowX = m_FinalX;
            m_timer.stop();
        }
    }
    else
    {
        m_FinalX = border_rect.left() + m_Radius_pos - m_Radius / 2;

        m_NowX -= width / 20.f;
        if (m_NowX < m_FinalX)
        {
            m_NowX = m_FinalX;
            m_timer.stop();
        }
    }
}

// 返回开关状态 - 打开：true 关闭：false
bool QWSwitcher::isToggled() const
{
    return mChecked;
}

// 设置开关状态
void QWSwitcher::setToggle(bool checked)
{
    if(checked == mChecked)
        return;
        
    mChecked = checked;
    m_timer.start(10);
}

// 设置背景颜色
void QWSwitcher::setBackgroundColor(const QColor& color)
{
  m_Background = color;
  update();
}

QColor QWSwitcher::getBackgroundColor() const
{
    return m_Background;
}

void QWSwitcher::setCheckedBallColor(const QColor& color)
{
    m_CheckedBall = color;
    update();
}

QColor QWSwitcher::getCheckedBallColor() const
{
    return m_CheckedBall;
}

void QWSwitcher::setCheckBallColor(const QColor& color)
{
    m_CheckBall = color;
    update();
}

QColor QWSwitcher::getCheckBallColor() const
{
    return m_CheckBall;
}

void QWSwitcher::setPenWidth(int pen)
{
    if (pen < 0)
        pen = 0;

    m_penWidth = pen;
    update();
}

int QWSwitcher::getPenWidth() const
{
    return m_penWidth;
}

int QWSwitcher::getWidth() const
{
    return width();
}

void QWSwitcher::setWidth(int width)
{
    resize(width, width / 2);
    update();
}
/*
void QWSwitcher::setRadius(int Radius)
{
    m_Radius = Radius;
	updateImage();
}
int QWSwitcher::getRadius()
{
    return m_Radius;
}

*/

void QWSwitcher::setBorderColor(const QColor& color)
{
    m_BorderColor = color;
    update();
}
QColor QWSwitcher::getBorderColor()
{
    return m_BorderColor;
}
void QWSwitcher::setBackCheCkedgroundColor(const QColor& color)
{
    m_CheckedBackground = color;
    update();
}
QColor QWSwitcher::getBackCheCkedgroundColor()
{
    return m_CheckedBackground;
}



