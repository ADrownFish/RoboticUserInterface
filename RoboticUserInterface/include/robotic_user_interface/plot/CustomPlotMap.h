#pragma once

#include "qcustomplot/qcustomplot.h"
#include "qt_gcw/QSnackbarManager.h"

#include "robotic_user_interface/core/DataSource.h"

#include <QList>
#include <QVector>

#define QCUSTOMPLOT_USE_OPENGL

struct CustomPlotMapBind {
  QCPGraph* graph;
  QColor color;
  ObjectData* data;
};

class CustomPlotLayer : public QCustomPlot {

  Q_OBJECT

public:
  CustomPlotLayer(QWidget *parent = nullptr);
  ~CustomPlotLayer();

  void initAxisRect(QCPAxisRect *axisRect);

  void initCustomPlot(QCustomPlot *customPlot);

  void appedObjectData(ObjectData* data, const QString &name);

  void init();

  void setConfiguration(std::shared_ptr<Configuration> config);

  void updatePlot();

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

private:
  std::shared_ptr<Configuration> config_;

  QMenu menu;
  QAction* Action_Plot_ObtPlot_Add = nullptr;
  QAction* Action_Plot_ObtPlot_Del = nullptr;

  QList<CustomPlotLayer*> layerList;
  QSplitter* splitter = nullptr;
};