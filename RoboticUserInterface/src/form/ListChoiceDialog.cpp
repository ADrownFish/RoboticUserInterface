#include "robotic_user_interface/form/ListChoiceDialog.h"

#include "FluControls/FluRadioButton.h"
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
  if (dialog.exec() == QDialog::Accepted &&
      dialog.m_buttonGroup->checkedButton()) {
    int index = dialog.m_buttonGroup->checkedId();
    QString text = dialog.m_buttonGroup->checkedButton()->text();

    return {index, text};
  }
  return {-1, QString()};
}

ListChoiceDialog::ListChoiceDialog(QWidget *parent, const QString &title,
                                   const QString &message,
                                   const QStringList &items,
                                   const QString &defaultItem)
    : QDialog(parent), m_buttonGroup(new QButtonGroup(this)) {
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
      "background-color: rgb(150, 150, 150);"); // RGB(150, 150, 150) 灰色
  layout->addWidget(line);

  // 添加单选按钮
  for (int i = 0; i < items.size(); ++i) {
    FluRadioButton *radio = new FluRadioButton(items[i], this);
    m_buttonGroup->addButton(radio, i); // 将按钮与索引关联
    layout->addWidget(radio);

    // 设置默认选中项
    if (items[i] == defaultItem) {
      radio->setChecked(true);
    }
  }

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

  // 自动计算合适的大小
  adjustSize();
  setMinimumWidth(std::max(width(), 300));

  setStyleSheet(R"(
      ListChoiceDialog {
        background-color: rgb(80, 80, 80);
        border-radius: 10px;
        border: none;
      }
      QLabel {
        color: white;  /* 提示文字颜色 */
      }
    )");

  setWindowFlag(Qt::FramelessWindowHint);
}