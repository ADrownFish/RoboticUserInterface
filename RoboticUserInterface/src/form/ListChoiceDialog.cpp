#include "robotic_user_interface/form/ListChoiceDialog.h"

#include "qwool/qwwindowbutton.h"
#include "qt_material_widgets/qtmaterialraisedbutton.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QStyle>

std::pair<int, QString>
ListChoiceDialog::getChoice(QWidget *parent, const QString &title,
                            const QString &message, const QStringList &items,
                            const QString &defaultItem) {
  ListChoiceDialog dialog(parent, title, message, items, defaultItem);
  auto result = dialog.exec();
  int index = dialog.btn->getCurrentUnitIndex();
  if (result == QDialog::Accepted && index >= 0) {
    QString text = dialog.btn->getUnitName(index);
    // qDebug() << index << text;
    return {index, text};
  }
  return {-1, QString()};
}

ListChoiceDialog::ListChoiceDialog(QWidget *parent, const QString &title,
                                   const QString &message,
                                   const QStringList &items,
                                   const QString &defaultItem)
    : QDialog(parent) {
  setWindowTitle(title);
  setModal(true);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSizeConstraint(QLayout::SetMinimumSize); // 保持最小大小
  layout->setContentsMargins(10, 10, 10, 10);

  // 提示信息
  if (!message.isEmpty()) {
    QLabel *label = new QLabel(message, this);
    layout->addWidget(label);
  }

  QFrame *line = new QFrame;
  line->setFrameShape(QFrame::HLine);  // 设置为水平线
  line->setFrameShadow(QFrame::Sunken); // 可选：设置阴影效果
  line->setLineWidth(3);               // 设置线宽为 2px
  line->setFixedHeight(3);             // 固定高度（避免占用额外空间）
  line->setStyleSheet(
      "background-color: rgb(150, 150, 150);");
  layout->addWidget(line);

  // 添加单选按钮
  btn = new QWWindowButton(this);
  btn->setBackgroundColor(QColor(0,0,0,0));
  btn->setAllowMouseClicked(true);
  btn->setDirection(QWWindowButton::Direction::Vertical);
  btn->setFixedUnitSize(30);
  btn->setIntervalDistance(4);
  layout->addWidget(btn);
  // qDebug() << items;
  QString longestText;
  for (int i = 0; i < items.size(); ++i) {
    
    auto &item = items[i];

    btn->addUnit(item);
    
    if (item == defaultItem) {
      btn->setSelectUnitIndex(i);
    }

    if (item.length() > longestText.length()) {
      longestText = item;
    }
  }

  QFontMetrics fm(font());
  QRect textRect = fm.boundingRect(QRect(0, 0, parent->width(), 1000), 
                                    Qt::TextWordWrap, 
                                    longestText);
  int minWidth = textRect.width() + 80;                                  

  line = new QFrame;
  line->setFrameShape(QFrame::HLine);  // 设置为水平线
  line->setFrameShadow(QFrame::Sunken); // 可选：设置阴影效果
  line->setLineWidth(3);               // 设置线宽为 2px
  line->setFixedHeight(3);             // 固定高度（避免占用额外空间）
  line->setStyleSheet(
      "background-color: rgb(150, 150, 150);"); // RGB(150, 150, 150) 灰色
  layout->addWidget(line);

  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->setContentsMargins(0, 10, 0, 0); // 设置按钮区域边距

  auto btn_ok = new QtMaterialRaisedButton();
  btn_ok->setText(tr("Confirm"));
  btn_ok->setFixedSize(80, 30);

  auto btn_cancel = new QtMaterialRaisedButton();
  btn_cancel->setText(tr("Cancel"));
  btn_cancel->setFixedSize(80, 30);

  buttonLayout->addStretch(); // 左侧弹簧（使按钮靠右）
  buttonLayout->addWidget(btn_ok);
  buttonLayout->addWidget(btn_cancel);
  layout->addLayout(buttonLayout);

  connect(btn_ok, &QPushButton::clicked, this, &QDialog::accept);
  connect(btn_cancel, &QPushButton::clicked, this, &QDialog::reject);

  // // 自动计算合适的大小
  // adjustSize();

  setStyleSheet(R"(
      ListChoiceDialog {
        background-color: rgb(60, 60, 60);
        border-radius: 10px;
        border: none;
      }
      QLabel {
        color: white;
      }
    )");

  setWindowFlag(Qt::FramelessWindowHint);
  adjustSize();
  setMinimumWidth(minWidth);
}