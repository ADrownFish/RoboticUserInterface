#pragma once

#include <QDateTime>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QTimer>
#include <QWidget>

#include <algorithm>

// Generic custom widgets
namespace GCW {
enum NotifyType {
  Info = 0x00, // 提示信息，用于提供一般性的信息
  Success,     // 操作成功，表示操作成功完成
  Debug,       // 调试信息，用于调试目的，仅在调试模式下显示
  Warning,     // 警告信息，表示一般性的警告
  Error,       // 错误信息，表示操作失败或出现错误
};

class QSnackbar : public QWidget {
  Q_OBJECT
	enum class AnimationStage{
		Start,
		Stay,
		Hide,
	};
public:
  QSnackbar(QWidget *parent)
      : QWidget(parent), animation(this, "pos") {
    parentWidget = parent;

    // 连接显示动画完成信号
    connect(&animation, &QPropertyAnimation::finished, [this]() {
      if(animationStage == AnimationStage::Start){
				animationStage = AnimationStage::Stay;
      	stayTimer.start(stayDuration);
			} else if(animationStage == AnimationStage::Hide){
				emit finished(this);
				deleteLater();
			}
    });

    connect(&stayTimer, &QTimer::timeout, [this]() {
			animationStage = AnimationStage::Hide;
      startHideAnimation();
    });

  }
  ~QSnackbar() {}

  QColor getColorForNotifyType(NotifyType type) {
    switch (type) {
    case NotifyType::Info:
      return QColor(64, 156, 255); // 淡蓝色
    case NotifyType::Success:
      return QColor(0, 176, 80); // 绿色
    case NotifyType::Debug:
      return QColor(255, 192, 0); // 橙黄色
    case NotifyType::Warning:
      return QColor(255, 127, 39); // 橙色
    case NotifyType::Error:
      return QColor(220, 53, 69); // 深红色
    default:
      return QColor(0, 0, 0); // 黑色，未知类型
    }
  }

  QString notifyTypeToString(NotifyType type) {
    switch (type) {
    case NotifyType::Info:
      return QStringLiteral("[ INFO ]");
    case NotifyType::Success:
      return QStringLiteral("[ SUCCESSFUL ]");
    case NotifyType::Debug:
      return QStringLiteral("[ DEBUG ]");
    case NotifyType::Warning:
      return QStringLiteral("[ WARNING ]");
    case NotifyType::Error:
      return QStringLiteral("[ ERROR ]");
    default:
      return QStringLiteral("[ Unkonwn ]");
    }
  }

  void start(const QString &title, const QString &text, int totalTime,
             NotifyType type, int verticalOffset) {
    const int titleSpacing = 50; // 是点的间隙，用于隔开
    const int textSpacing = 50;

    _title = notifyTypeToString(type) + "  " + title;
    _text = text;

    // 计算停留时间 (总时间减去显示和隐藏动画时间)
    stayDuration = totalTime - 2 * animationDuration;
    if (stayDuration < 0) {
      stayDuration = 0;
    }

    // 测量字符串宽度
    QFontMetrics fontMetrics(font());
    int textWidth = fontMetrics.horizontalAdvance(_text) + textSpacing;
    textWidth = std::max(textWidth,
                         titleSpacing + fontMetrics.horizontalAdvance(_title));

    QSize UISize;
    UISize.setWidth(margin * 2 + textWidth);
    UISize.setHeight(60);
    if (UISize.width() > parentWidget->width() * 0.8)
      UISize.setWidth(parentWidget->width() * 0.8);
    else if (UISize.width() < 240)
      UISize.setWidth(240);

    QPoint showPos, hidePos;

    hidePos.rx() = parentWidget->width() / 2 - UISize.width() / 2;
    hidePos.ry() = -height();

    showPos.rx() = hidePos.x();
    showPos.ry() = verticalOffset; // 使用传入的垂直偏移量

    // 设置显示动画
    animation.setDuration(animationDuration);
    animation.setStartValue(hidePos);
    animation.setEndValue(showPos);
    animation.setEasingCurve(QEasingCurve::OutQuad);

    notifyTypeColor = getColorForNotifyType(type);

    setFixedSize(UISize);
    move(hidePos);
    show();
    raise();
    update();

    // 启动显示动画
    animation.start();
  }

  // 外部可以调用此方法来提前隐藏消息
  void startHideAnimation() {
    if (stayTimer.isActive()) {
      stayTimer.stop();
    }
		animation.stop();

		QPoint setPos = this->pos();
    animation.setStartValue(setPos);
		setPos.setY(-height());
		animation.setEndValue(setPos);
    animation.start();
  }

	void updatePosition(int verticalOffset) {
		animation.stop();
		
		QPoint setPos = this->pos();
    animation.setStartValue(setPos);
		setPos.setY(verticalOffset);
		animation.setEndValue(setPos);
    animation.start();

		
	}

signals:
  void finished(QSnackbar *snackbar);

protected:
  void paintEvent(QPaintEvent *event) override {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    painter.setPen(Qt::NoPen);
    painter.setBrush(backgroudColor);
    QRectF drawRect(rect());
    path.addRoundedRect(drawRect, BorderRadius, BorderRadius);
    painter.drawPath(path);

    painter.setPen(textColor);
    QFont Drawfont(font());
    Drawfont.setBold(true);

    painter.setFont(Drawfont);
    painter.drawText(QRect(margin, 2, width() - 2 * margin, 30),
                     Qt::AlignHCenter | Qt::AlignVCenter, _title);

    Drawfont.setBold(false);
    painter.setFont(Drawfont);
    painter.drawText(QRect(margin, 25, width() - 2 * margin, 25),
                     Qt::AlignHCenter | Qt::AlignVCenter, _text);

    // 画消息点
    painter.setBrush(notifyTypeColor);
    painter.drawEllipse(15, 13, 10, 10);
  }

	void mouseReleaseEvent(QMouseEvent *event)	{
		animation.stop();
		stayTimer.stop();
		animationStage = AnimationStage::Hide;
		startHideAnimation();
	}

public:
  QWidget *parentWidget;

	AnimationStage animationStage = AnimationStage::Start;

  QColor backgroudColor = QColor(250, 250, 250, 230);
  QColor textColor = QColor(50, 50, 50);
  QColor notifyTypeColor;

  int BorderRadius = 20;
  int margin = 10; // 边缘距离

  QString _title;
  QString _text;

  QPropertyAnimation animation;
  QTimer stayTimer;

  int animationDuration = 300; // 显示动画持续时间 (ms)
  int stayDuration = 4000;     // 停留时间 (ms)
};

class QSnackbarManager : public QObject {
  Q_OBJECT
public:
  QSnackbarManager(QWidget *parent = nullptr) : QObject(parent) {
    parentWidget = parent;
    spacing = 10; // 消息框之间的间距
  }
  ~QSnackbarManager() {}

  void addNotify(const QString &title, const QString &text, NotifyType type,
                 int totalTime = 5000 /* 5 s */) {
    QSnackbar *notifyObject = new QSnackbar(parentWidget);
    QFont _font(parentWidget->font());
    _font.setPointSize(12);
    notifyObject->setFont(_font);

    // 计算垂直偏移量
    int verticalOffset = calculateVerticalOffset();

    // 连接finished信号
    connect(notifyObject, &QSnackbar::finished, this, &QSnackbarManager::onSnackbarFinished);

    // 添加到活动列表
    activeSnackbars.append(notifyObject);

    notifyObject->start(title, text, totalTime, type, verticalOffset);
  }

  void onSnackbarFinished(QSnackbar *finishedSnackbar) {
    // 从活动列表中移除
    activeSnackbars.removeOne(finishedSnackbar);

    // 更新剩余消息框的位置
    updateSnackbarsPositions();
  }

  void updateSnackbarsPositions() {
    int currentOffset = spacing;

    for (QSnackbar *snackbar : activeSnackbars) {
      snackbar->updatePosition(currentOffset);
      currentOffset += snackbar->height() + spacing;
    }
  }

  int calculateVerticalOffset() {
    int offset = spacing;

    for (QSnackbar *snackbar : activeSnackbars) {
      offset += snackbar->height() + spacing;
    }

    return offset;
  }

  // 强制隐藏所有消息
  void hideAll() {
    for (QSnackbar *snackbar : activeSnackbars) {
      snackbar->startHideAnimation();
    }
    activeSnackbars.clear();
  }

private:
  QWidget *parentWidget;
  QList<QSnackbar *> activeSnackbars;
  int spacing; // 消息框之间的间距
};
} // namespace GCW