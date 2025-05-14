#include "robotic_user_interface/form/NavigationView.h"

NavigationSwitcher::NavigationSwitcher(QWidget* parent)
  : NavigationObject(parent)
{
  layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  text.setText("Label");
  text.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

  toggle.setFixedSize(40, 20);
  toggle.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  toggle.setPenWidth(2);
  toggle.setBackgroundColor(QColor(0, 0, 0, 0));
  toggle.setToggle(true);
  toggle.setBackCheCkedgroundColor(QColor(0, 0, 0, 0));

  stretchWidgetHead.setFixedWidth(10);
  stretchWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  // 添加 text、弹簧、toggle 到布局
  layout->addWidget(&stretchWidgetHead);
  layout->addWidget(&text);
  layout->addWidget(&stretchWidget);
  layout->addWidget(&toggle);

  this->setFixedHeight(40);
}

void NavigationSwitcher::setText(const QString& t) {
  text.setText(t);
}

QWSwitcher* NavigationSwitcher::getSwitcher()
{
  return &toggle;
}

void NavigationSwitcher::setExpand(bool ok)
{
  text.setVisible(ok);
  stretchWidget.setVisible(ok);
  stretchWidgetHead.setVisible(ok);
}






NavigationItem::NavigationItem(QWidget* parent)
  : NavigationObject(parent)
{
  setFixedHeight(40);
  setMouseTracking(true); // 启用鼠标追踪以支持 hover

  QFont font;
  font.setPointSize(11);
  font.setWeight(QFont::Medium);

  // 左边选中态竖线
  lineWidget = new QWidget(this);
  lineWidget->setFixedSize(4, 16);
  setClicked(false);

  // 图标
  iconLabel = new QLabel(this);
  iconLabel->setFixedSize(20, 20);
  iconLabel->setScaledContents(true); // 保证图标填充

  // 文本
  textLabel = new QLabel(this);
  textLabel->setFont(font);
  textLabel->setFixedHeight(32);

  //弹簧
  stretchWidget = new QWidget();
  stretchWidget->setFixedWidth(10);  // 宽度固定为 10
  stretchWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

  // 布局
  layout = new QHBoxLayout(this);
  layout->setContentsMargins(5, 0, 5, 0);
  layout->setSpacing(4);
  layout->addWidget(lineWidget);
  layout->addWidget(iconLabel);
  layout->addWidget(stretchWidget);

  layout->addWidget(textLabel);
  layout->addStretch();

  setLayout(layout);
}


void NavigationItem::setIcon(const QIcon& icon) {
  currentIcon = icon;
  iconLabel->setPixmap(icon.pixmap(iconLabel->size()));
}

void NavigationItem::setText(const QString& text) {
  textLabel->setText(text);
}

void NavigationItem::setClicked(bool selected) {
  isSelected = selected;
  if (selected) {
  lineWidget->setStyleSheet("background-color: rgb(118, 185, 237); border-radius: 2px;");
  }  else {
  lineWidget->setStyleSheet("background-color: rgba(118, 185, 237, 0); border-radius: 2px;");
  }
  update(); // 重绘界面以刷新颜色状态
}

void NavigationItem::setAsButton(bool ok)
{
  isAsButton = ok;
}

bool NavigationItem::asButton()
{
  return isAsButton;
}

void NavigationItem::setExpand(bool ok)
{
  textLabel->setVisible(ok);
  stretchWidget->setVisible(ok);
}


void NavigationItem::enterEvent(QEnterEvent* event) {
  isHovered = true;
  update(); // 触发 repaint
  QWidget::enterEvent(event);
}

void NavigationItem::leaveEvent(QEvent* event) {
  isHovered = false;
  update();
  QWidget::leaveEvent(event);
}

void NavigationItem::mouseReleaseEvent(QMouseEvent* event) {
	emit clicked(this);
  QWidget::mousePressEvent(event);
}


void NavigationItem::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  if (isHovered) {
    QColor bg = QColor(50, 50, 50);
    painter.setBrush(bg);
    painter.setPen(Qt::NoPen);  // 不需要边框
    painter.drawRoundedRect(rect(), radius, radius);  // 圆角半径
  }

  QWidget::paintEvent(event);
}






NavigationView::NavigationView(QWidget* parent)
  : QWidget(parent)
{
  // 最外层垂直布局
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(5, 5, 5, 5);
  mainLayout->setSpacing(10);

  // 三个区域布局
  topLayout = new QVBoxLayout();
  topLayout->setSpacing(5);
  topLayout->setContentsMargins(0, 0, 0, 0);

  centerLayout = new QVBoxLayout();
  centerLayout->setSpacing(5);
  centerLayout->setContentsMargins(0, 0, 0, 0);

  bottomLayout = new QVBoxLayout();
  bottomLayout->setSpacing(5);
  bottomLayout->setContentsMargins(0, 0, 0, 0);

  menu = new NavigationItem();
  menu->setIcon(QIcon(":/svg/svg/menu.svg"));
  menu->setText(tr("Expand"));
  menu->setFixedHeight(40);

  // 添加一个弹性 spacer，保证中间内容弹性填充
  QWidget* topWidget = new QWidget;
  topWidget->setLayout(topLayout);

  QWidget* centerWidget = new QWidget;
  centerWidget->setLayout(centerLayout);

  QWidget* bottomWidget = new QWidget;
  bottomWidget->setLayout(bottomLayout);

  // 添加按钮
  mainLayout->addWidget(menu);
  mainLayout->addSpacerItem(new QSpacerItem(
    0, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));

  mainLayout->addWidget(topWidget);
  mainLayout->addSpacerItem(new QSpacerItem(
    0, 20,    QSizePolicy::Minimum,    QSizePolicy::Fixed  ));

  mainLayout->addWidget(centerWidget);  // stretch=1 占满中部空间
  mainLayout->addStretch();

  mainLayout->addWidget(bottomWidget);

  setLayout(mainLayout);

  // 初始化动画
  widthAnimation = new QVariantAnimation(this);
  widthAnimation->setDuration(500); // 动画持续时间500ms
  widthAnimation->setEasingCurve(QEasingCurve::InOutQuad); // 平滑过渡

  connect(widthAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
    this->setFixedWidth(value.toInt());
    });

  connect(menu, &NavigationItem::clicked, this, &NavigationView::toggleExpandRetract);

  // 初始状态
  collapsedWidth = 50;
  expandedWidth = 160;
  this->setFixedWidth(expandedWidth);
}

void NavigationView::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  // 可选：响应式调整内容
}


// 分别添加到三段的接口
void NavigationView::addItemToTop(NavigationObject* widget) {
  topLayout->addWidget(widget);
  objList.append(widget);
}

void NavigationView::addItemToCenter(NavigationItem* widget) {
  centerLayout->addWidget(widget);
  objList.append(widget);
  QObject::connect(widget, &NavigationItem::clicked, this, &NavigationView::setClicked);
}

void NavigationView::addItemToBottom(NavigationItem* widget) {
  bottomLayout->addWidget(widget);
  objList.append(widget);
  QObject::connect(widget, &NavigationItem::clicked, this, &NavigationView::setClicked);
}
void NavigationView::setClicked(NavigationItem* item)
{
  if (item->asButton()) {
    return;
  }

  if (selected) {
    selected->setClicked(false);
  }
  selected = item;
  selected->setClicked(true);
}


void NavigationView::toggleExpandRetract(bool ok)
{
  int currentWidth = this->width();
  int target = isExpand ? collapsedWidth : expandedWidth;

  if (widthAnimation->state() == QAbstractAnimation::Running) {
    widthAnimation->stop(); // 停止当前动画
  }

  widthAnimation->setStartValue(currentWidth);
  widthAnimation->setEndValue(target);
  widthAnimation->start();

  isExpand = !isExpand; // 切换状态

  for (auto& p : objList) {
    p->setExpand(isExpand);
  }

  menu->setExpand(isExpand);
}

