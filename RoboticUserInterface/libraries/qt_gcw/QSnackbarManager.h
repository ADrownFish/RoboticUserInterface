#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QDateTime>
#include <QPropertyAnimation>
#include <QDebug>
#include <QPainterPath>

#include <algorithm>

// Generic custom widgets
namespace GCW
{
	enum NotifyType
	{
		Info = 0x00,          // 提示信息，用于提供一般性的信息
		Success,                // 操作成功，表示操作成功完成
		Debug,                  // 调试信息，用于调试目的，仅在调试模式下显示
		Warning,               // 警告信息，表示一般性的警告
		Error,                     // 错误信息，表示操作失败或出现错误
	};

	class QSnackbar :public QWidget
	{
		Q_OBJECT
	public:
		QSnackbar(QWidget* parent)
			: QWidget(parent),
			_animation(this, "pos")
		{
			parentWidget = parent;
		}
		~QSnackbar()
		{

		}

		QColor getColorForNotifyType(NotifyType type) {
			switch (type) {
			case NotifyType::Info:
				return QColor(64, 156, 255);  // 淡蓝色
			case NotifyType::Success:
				return QColor(0, 176, 80);    // 绿色
			case NotifyType::Debug:
				return QColor(255, 192, 0);   // 橙黄色
			case NotifyType::Warning:
				return QColor(255, 127, 39);  // 橙色
			case NotifyType::Error:
				return QColor(220, 53, 69);   // 深红色
			default:
				return QColor(0, 0, 0);       // 黑色，未知类型
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
				return QStringLiteral("[ 错误 ]");
			default:
				return QStringLiteral("[ 未知 ]");
			}
		}


		void start(const QString& title, const QString& text, int totalTime, NotifyType type)
		{
			//qDebug() << "[ QSnackbar ][ INFO ] add , from" << parentWidget->objectName();

			const int titleSpacing = 50;  //是点的间隙，用于隔开
			const int textSpacing = 50;

			_title = notifyTypeToString(type) + "  " + title;
			_text = text;

			// 测量字符串宽度
			QFontMetrics fontMetrics(font());
			int textWidth = fontMetrics.horizontalAdvance(_text) + textSpacing;
			textWidth = std::max(textWidth, titleSpacing + fontMetrics.horizontalAdvance(_title));  

			QSize UISize;
			UISize.setWidth(margin * 2 + textWidth);
			UISize.setHeight(60);
			if(UISize.width() > parentWidget->width() * 0.8)
				UISize.setWidth(parentWidget->width() * 0.8);
			else if(UISize.width() < 240)
				UISize.setWidth(240);
			

			QPoint showPos, hidePos;

			hidePos.rx() = parentWidget->width() / 2 - UISize.width()/2;
			hidePos.ry() = -height();

			showPos.rx() = hidePos.x();
			showPos.ry() = height();

			_animation.setDuration(totalTime); // 动画持续时间为1秒
			_animation.setStartValue(hidePos); // 初始位置
			_animation.setKeyValueAt(0.05, showPos);
			_animation.setKeyValueAt(0.95, showPos);
			_animation.setEndValue(hidePos); // 终点位置
			_animation.setEasingCurve(QEasingCurve::OutInQuad);
			_animation.start(QAbstractAnimation::KeepWhenStopped);

			connect(&_animation, &QPropertyAnimation::finished, [this]()
				{
					deleteLater();
				});

			notifyTypeColor = getColorForNotifyType(type);

			setFixedSize(UISize);
			move(hidePos);
			show();
			raise();
			update();
		}

	protected:

		void paintEvent(QPaintEvent* event) override
		{
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
			painter.drawText(QRect(margin, 2, width() - 2 * margin, 30), Qt::AlignHCenter | Qt::AlignVCenter, _title);

			Drawfont.setBold(false);
			painter.setFont(Drawfont);
			painter.drawText(QRect(margin, 25, width() - 2 * margin, 25), Qt::AlignHCenter | Qt::AlignVCenter, _text);

			//画消息点
			painter.setBrush(notifyTypeColor);
			painter.drawEllipse(15, 13, 10, 10);

		}

	public:
		QWidget* parentWidget;

		QColor backgroudColor = QColor(250, 250, 250, 230);
		QColor textColor = QColor(50, 50, 50);
		QColor notifyTypeColor;

		int BorderRadius = 20;
		int margin = 10;                    //边缘距离

		QString _title;
		QString _text;

		QPropertyAnimation _animation;
	};


	class QSnackbarManager : public QObject{
		Q_OBJECT
	public:
		QSnackbarManager(QWidget* parent = nullptr)
		: QObject(parent)
		{
			parentWidget = parent;

			setConnection();
		}
		~QSnackbarManager() {

		}

		void addNotify(const QString& title, const QString& text, NotifyType type,int totalTime = 5000 /* 5 s */ )
		{
			QSnackbar *notifyObject = new QSnackbar(parentWidget);
			QFont _font(parentWidget->font());
			_font.setPointSize(12);
			notifyObject->setFont(_font);
			notifyObject->start(title, text, totalTime, type);
		}

		void setConnection()
		{

		}

	private:
		QWidget* parentWidget;
	};
}


