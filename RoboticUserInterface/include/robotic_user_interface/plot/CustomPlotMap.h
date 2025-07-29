#pragma once

#include "qcustomplot/qcustomplot.h"
#include "qt_gcw/QSnackbarManager.h"
#include "FluControls/FluMenu.h"

#include "robotic_user_interface/core/DataSource.h"

#include <QList>
#include <QVector>
#include <QMap>

#define QCUSTOMPLOT_USE_OPENGL

struct CustomPlotMapBind {
  QCPGraph* graph;
  QColor color;
  ObjectData* data;
  scalar_t lastTime = 0.;
  int lastIndex = 0; // 新增：记录上次更新的索引位置

  void optimizedUpdate(bool trimData = true) {
    if (!graph || !data || data->time.isEmpty()) return;

    // 1. 裁剪旧数据（可选）
    if (trimData) {
      const scalar_t minTime = data->time.last() - data->timeWindow;
      auto it = std::lower_bound(data->time.begin(), data->time.end(), minTime);
      int keepStart = std::distance(data->time.begin(), it);

      if (keepStart > 0) {
        data->time.remove(0, keepStart);
        data->data.remove(0, keepStart);
        lastIndex = qMax(0, lastIndex - keepStart);
      }
    }

    // 2. 准备新增数据
    int newPoints = data->time.size() - lastIndex;
    if (newPoints <= 0) return;

    // 3. 批量添加数据（使用QCustomPlot优化过的接口）
    QVector<scalar_t> newTimes(data->time.constBegin() + lastIndex,
      data->time.constBegin() + lastIndex + newPoints);
    QVector<scalar_t> newValues(data->data.constBegin() + lastIndex,
      data->data.constBegin() + lastIndex + newPoints);

    graph->addData(newTimes, newValues);
    lastIndex += newPoints;
    lastTime = data->time.last();
  }

  // 全量更新（重置时使用）
  void fullUpdate() {
    if (!graph || !data) return;
    graph->data()->clear();
    lastIndex = 0;
    if (!data->time.isEmpty()) {
      graph->setData(data->time, data->data);
      lastIndex = data->time.size();
      lastTime = data->time.last();
    }
  }

  bool operator==(const CustomPlotMapBind& other) const {
    bool ok = true;
    ok &= this->graph == other.graph;
    ok &= this->data == other.data;
    return ok;
  }
};

class CustomPlotLayer : public QCustomPlot {

  Q_OBJECT
  friend class CustomPlotMap;
public:
  struct QCPItemTextEllipse{
    QCPItemText *itemText = nullptr;
    QCPItemTracer *itemTracer = nullptr;
  };
public:
  CustomPlotLayer(QWidget *parent = nullptr);
  ~CustomPlotLayer();

  void initAxisRect(QCPAxisRect *axisRect);

  void initItemText(QCPItemText *item);

  void initItemTracer(QCPItemTracer *item);

  void initCustomPlot(QCustomPlot *customPlot);

  void appedObjectData(ObjectData* data, const QString &name);

  void init();

  void setConfiguration(std::shared_ptr<Configuration> config);

  void updatePlot();

  void updateAxisX(bool replot = false);

  void updateAxisY(bool replot = false);

  void updateAxis(bool replot = false);

  void updateLineAndText(bool replot = false);

  QList<CustomPlotMapBind>& bindList();

  QPoint & mousePoint(){return mousePos;}

signals:
  void publishNotify(GCW::NotifyType type, const QString& title,   const QString& text);

  void rightClicked();  // 自定义右键点击信号

private:
  void setupSignalConnection();

  void setupWidgetsControls();

protected:
  // 重写拖放事件处理函数
  void dropEvent(QDropEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *a_event);
  void contextMenuEvent(QContextMenuEvent* event);

private:
  QList<CustomPlotMapBind> bindList_;
  std::shared_ptr<Configuration> config_;

  // operation
  QCPItemStraightLine* verticalLine;
  QCPItemStraightLine* horizontalLine;
  QMap<QCPGraph*, QCPItemTextEllipse> coordLabels;
  QCPItemText* coordLabels_X;
  QCPItemText* coordLabels_Y;

  QPoint mousePos;
  bool markLineVertical = false;
  bool markLineHorizontal = false;
  bool isMidMousePressed = false;

  QColor markColor = QColor(255, 80, 80);
};


class CustomPlotMap : public QWidget {

  Q_OBJECT

public:
  CustomPlotMap(QWidget* parent = nullptr);
  ~CustomPlotMap();

  void init();

  void setConfiguration(std::shared_ptr<Configuration> config);

  void updatePlot();

  void addCustomPlotLayer();

  void delCustomPlotLayer();

  void resetSplitterLayout();

signals:
  void publishNotify(GCW::NotifyType type, const QString& title, const QString& text);

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  CustomPlotLayer* getCustomPlotLayer(const QPoint &point);

  void layerAxisScalingChanged(CustomPlotLayer *p, const QCPRange &range);

private:
  std::shared_ptr<Configuration> config_;

  FluMenu menu;
  QPoint menuActivatePos;

  QList<CustomPlotLayer*> layerList;
  QSplitter* splitter = nullptr;
};