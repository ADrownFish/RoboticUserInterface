#pragma once

#include "qcustomplot/qcustomplot.h"
#include "qt_gcw/QSnackbarManager.h"
#include "FluControls/FluMenu.h"

#include "filter/LiteKalmanFilter.h"
#include "filter/MovingAverageFilter.h"
#include "filter/IIRFilter.h"

#include "robotic_user_interface/core/DataSource.h"

#include <QList>
#include <QVector>
#include <QMap>

#define QCUSTOMPLOT_USE_OPENGL

class FilterData{

public:
  enum class FilterType{
    LowPass,
    HighPass,
    BandPass,
    BandStop,
    LowShelf,
    HighShelf,

    MovingAverage,
    Kalman,
  };

  union FilterParm{
    FilterParm(){}
    FilterParm(robot::IIRFilterParm _value){
      IIR = _value;
    }
    FilterParm(robot::LiteKalmanFilterParm _lowHighPass){
      Kalman = _lowHighPass;
    }
    FilterParm(robot::MovingAverageFilterParm _bandStopPass){
      MovingAverage = _bandStopPass;
    }

    robot::IIRFilterParm IIR;
    robot::LiteKalmanFilterParm Kalman;
    robot::MovingAverageFilterParm MovingAverage;
  };

  void setType(FilterType d, const FilterParm& p){
    type = d;
    parm = p;
  }

  void setEnable(bool e){
    enable = e;
    if(!enable){
      filter.reset();
      data.reset();
      return;
    }

    switch(type){
    case FilterType::LowPass:
    case FilterType::HighPass:
    case FilterType::BandPass:
    case FilterType::BandStop:
    case FilterType::LowShelf:
    case FilterType::HighShelf:
      filter = std::make_shared<robot::IIRFilter>(static_cast<robot::IIRFilterType>(type), parm.IIR);
      break;
    case FilterType::Kalman:
      filter = std::make_shared<robot::LiteKalmanFilter>(parm.Kalman);
      break;
    case FilterType::MovingAverage:
      filter = std::make_shared<robot::MovingAverageFilter>(parm.MovingAverage);
      break;
    default:
      filter = std::make_shared<robot::BaseFilter>();
      std::cout << "[FilterData] Wrong filter type, reset to default" << std::endl;
      break;
    }
  }

  ObjectData::Ptr &update(const ObjectData::Ptr& d){
    if(!enable){
      return data;
    }

    
  }

  ObjectData::Ptr &result()  {
    return data;
  }

private:

  bool enable = false;
  FilterType type = FilterType::MovingAverage;
  FilterParm parm;

  robot::BaseFilter::Ptr filter;
  ObjectData::Ptr data;
};

class CustomPlotMapBind {
public:
  QCPGraph* graph;
  QCPItemText *itemText = nullptr;
  QCPItemTracer *itemTracer = nullptr;
  FilterData filterData;

  ObjectData* data;
  QColor color;


public:
  // 全量更新（重置时使用）
  void fullUpdate() {
    // debug 0.7~1ms
    if (!graph || !data) return;
    graph->setData(data->time, data->data, true);
  }

  void asynchronousUpdates() {
    if (!graph || !data) return;
    graph->data();
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

public:
  CustomPlotLayer(QWidget *parent = nullptr);
  ~CustomPlotLayer();

  void initAxisRect(QCPAxisRect *axisRect);

  void initItemText(QCPItemText *item);

  void initItemTracer(QCPItemTracer *item);

  void initCustomPlot(QCustomPlot *customPlot);

  void appedObjectData(ObjectData* data, const QString &name, const QString colorString);

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

  void dragAccepted();

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  void DoubleClickDataPoint(const QPointF &pos);

protected:
  // 重写拖放事件处理函数
  void dropEvent(QDropEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *a_event) override;
  void contextMenuEvent(QContextMenuEvent* event) override;

private:
  QList<CustomPlotMapBind> bindList_;
  std::shared_ptr<Configuration> config_;

  // 数据追踪器
  QCPItemStraightLine* verticalLine;
  QCPItemStraightLine* horizontalLine;
  QCPItemText* coordLabels_X;
  QCPItemText* coordLabels_Y;

  // 数据对比
  bool displayDelta = false;
  QCPItemLine* markLineA;
  QCPItemLine* markLineB;
  QCPItemTracer* markTracerA;
  QCPItemTracer* markTracerB;
  QList<QPointF> markPoints;
  QCPItemText *deltaText;

  // 鼠标单击判断
  QTimer clickTimer;
  bool clickedFlag = false;

  QPoint mousePos;
  bool markLineVertical = false;
  bool markLineHorizontal = false;
  bool isMidMousePressed = false;

  QColor markColor = QColor(255, 220, 200);
};


class CustomPlotMap : public QWidget {

  Q_OBJECT

public:
  CustomPlotMap(QWidget* parent = nullptr);
  ~CustomPlotMap();

  void init();

  void setConfiguration(std::shared_ptr<Configuration> config);

  void setDataSource(const std::shared_ptr<DataSource>& ds);

  void updatePlot();

  CustomPlotLayer* addCustomPlotLayer();

  void delCustomPlotLayer(bool keepOne = true);

  void resetSplitterLayout();

  void checkObjectData();

  QSplitter* getSplitter();

  QList<CustomPlotLayer*>& getLayerList();

signals:
  void publishNotify(GCW::NotifyType type, const QString& title, const QString& text);

  void dragAccepted();

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  CustomPlotLayer* getCustomPlotLayer(const QPoint &point);

  void layerAxisScalingChanged(CustomPlotLayer *p, const QCPRange &range);

private:
  std::shared_ptr<Configuration> config_;
  std::shared_ptr<DataSource>    dataSource_;

  FluMenu menu;
  QPoint menuActivatePos;

  QList<CustomPlotLayer*> layerList;

  QSplitter* verticalSplitter = nullptr;
  QVector<QSplitter*> horizontalLayouts;
};
