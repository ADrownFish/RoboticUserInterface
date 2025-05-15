#pragma once

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyleOption>

class FunctionCard : public QWidget
{
    Q_OBJECT
  public:
    FunctionCard(QWidget* parent = nullptr);

    FunctionCard(const QIcon& icon, const QString& titleText, const QString& contextText, QWidget* parent = nullptr);

    void setKey(QString key);

    QString getKey();

    void mouseReleaseEvent(QMouseEvent* event);

    void paintEvent(QPaintEvent* event);

  signals:
    void clicked(QString key);

  protected:
    QString m_key;
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_contextLabel;
};
