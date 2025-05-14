#pragma once

#include <QLayout>
#include <QRect>
#include <QStyle>

class QFlowLayout : public QLayout {
public:
  explicit QFlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1,
                       int vSpacing = -1);
  explicit QFlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
  ~QFlowLayout() override;

  void addItem(QLayoutItem *item) override;
  int horizontalSpacing() const;
  int verticalSpacing() const;
  Qt::Orientations expandingDirections() const override;
  bool hasHeightForWidth() const override;
  int heightForWidth(int) const override;
  int count() const override;
  QLayoutItem *itemAt(int index) const override;
  QSize minimumSize() const override;
  void setGeometry(const QRect &rect) override;
  QSize sizeHint() const override;
  QLayoutItem *takeAt(int index) override;

  // 重新调整水平/垂直方向的间距
  void setHorizontalSpacing(int hSpacing);
  void setVerticalSpacing(int vSpacing);
  // 立即刷新布局（重新设置水平/垂直方向的间距后，如果布局没有变化，可调用此接口显式刷新布局）
  void refreshLayout();

private:
  int doLayout(const QRect &rect, bool testOnly) const;
  int smartSpacing(QStyle::PixelMetric pm) const;

  QList<QLayoutItem *> itemList;
  int m_hSpace;
  int m_vSpace;
};
