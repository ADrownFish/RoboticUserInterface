#pragma once
#include <QDialog>
#include <QButtonGroup>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QStringList>

class QWWindowButton;

class ListChoiceDialog : public QDialog {
  Q_OBJECT

public:
  // 静态方法：返回用户选择的索引和文本（取消时返回 -1 和空字符串）
  static std::pair<int, QString> getChoice(
    QWidget* parent,
    const QString& title,
    const QString& message,
    const QStringList& items,
    const QString& defaultItem = QString()
  ); 

private:
  explicit ListChoiceDialog(
    QWidget* parent,
    const QString& title,
    const QString& message,
    const QStringList& items,
    const QString& defaultItem
  );

  QWWindowButton* btn;
};