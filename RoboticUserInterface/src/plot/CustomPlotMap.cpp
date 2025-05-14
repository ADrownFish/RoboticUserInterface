#include "robotic_user_interface/plot/CustomPlotMap.h"

#include <QTreeWidgetItem>

CustomPlotLayer::CustomPlotLayer(QWidget *parent) : QCustomPlot(parent) {
  // 设置控件可接受拖放
  setAcceptDrops(true);
}
CustomPlotLayer::~CustomPlotLayer() {}

void CustomPlotLayer::contextMenuEvent(QContextMenuEvent* event) {
  emit rightClicked();
}

void CustomPlotLayer::dragEnterEvent(QDragEnterEvent * event) {
  // 如果拖动的事件中有Text，则允许drop。如果没有Text则不允许放下
  if (event->mimeData()->hasFormat("application-robotuserinterface-objectdata")) {
    event->acceptProposedAction();
  }
}
void CustomPlotLayer::dropEvent(QDropEvent *event) {
  if (!event->mimeData()->hasFormat("application-robotuserinterface-objectdata")) return;
  QByteArray data = event->mimeData()->data("application-robotuserinterface-objectdata");
  QString name = event->mimeData()->data("application-robotuserinterface-objectname");
 
  quintptr ptrValue;
  if (data.size() == sizeof(ptrValue)) {
    memcpy(&ptrValue, data.data(), sizeof(ptrValue));
  }
  else {
    return;
  }

  ObjectData* ptr = reinterpret_cast<ObjectData*>(ptrValue);
  appedObjectData(ptr, name);

  event->acceptProposedAction();
  QCustomPlot::dropEvent(event);
}

void CustomPlotLayer::setupSignalConnection() {

}

void CustomPlotLayer::updatePlot() {
  // 获取轴
  QCPAxis* xAxis = this->axisRect()->axis(QCPAxis::atBottom);
  QCPAxis* yAxis = this->axisRect()->axis(QCPAxis::atLeft);

  // 格子线
  xAxis->grid()->setVisible(config_->plot.gridLine);
  yAxis->grid()->setVisible(config_->plot.gridLine);

  if (!config_->plot.isPaused) {
    // 给每个数据源更新
    int dataSourceCount = bindList_.count();

    bool allowScale = false;
    for (size_t i = 0; i < dataSourceCount; i++) {
      auto& bind = bindList_[i];

      // 如果有一个有数据变化，都允许刷新
      allowScale |= !bind.data->isZeroData();

      // 不启用
      bind.graph->setVisible(bind.data->enable);
      if (!bind.data->enable) {
        continue;
      }

      bind.graph->setData(bind.data->time, bind.data->data); // 原本是这样的

      QCPGraph::LineStyle lineStyle;
      QCPScatterStyle scatterStyle;
      if (config_->plot.plotLine == PlotLineType::Line) {
        lineStyle = QCPGraph::LineStyle::lsLine;
        scatterStyle = QCPScatterStyle(QCPScatterStyle::ssNone);
        
      } else if (config_->plot.plotLine == PlotLineType::Point) {
        lineStyle = QCPGraph::LineStyle::lsNone;
        scatterStyle = QCPScatterStyle(QCPScatterStyle::ssDiamond, 4);
       
      } else {
        lineStyle = QCPGraph::LineStyle::lsLine;
        scatterStyle = QCPScatterStyle(QCPScatterStyle::ssSquare, 4);
      }
      bind.graph->setLineStyle(lineStyle);
      bind.graph->setScatterStyle(scatterStyle);

      // 性能优化 TODO
      //QSharedPointer<QCPGraphDataContainer> container = bind.graph->data(); // 想拿出来修改
      //container->
    }

    // 自动缩放 x
    if (config_->plot.xAutoScale) {
      xAxis->rescale(true);
    }

    // 自动缩放 y
    if (config_->plot.yAutoScale && allowScale) {
      yAxis->rescale(true);

      QCPRange range = yAxis->range();
      double margin = 0.05 * (range.upper - range.lower); // 10% 的边距
      yAxis->setRange(range.lower - margin, range.upper + margin);
    }
  }

  this->replot(QCustomPlot::rpQueuedReplot);
}

void CustomPlotLayer::initAxisRect(QCPAxisRect *axisRect) {
  axisRect->setBackground(QColor(30, 30, 30)); // 设置背景色

  // 设置x/y轴文本色、轴线色、字体等
  QPen basePen(QColor(132, 178, 170));
  QPen tickPen(QColor(128, 100, 255));
  QPen subTickPen(QColor(255, 165, 0));

  axisRect->axis(QCPAxis::atBottom)->setTickLabelColor(Qt::white);
  axisRect->axis(QCPAxis::atBottom)->setLabelColor(QColor(180, 200, 200));
  axisRect->axis(QCPAxis::atBottom)->setBasePen(basePen);
  axisRect->axis(QCPAxis::atBottom)->setTickPen(tickPen);
  axisRect->axis(QCPAxis::atBottom)->setSubTickPen(subTickPen);

  QFont xFont = axisRect->axis(QCPAxis::atBottom)->labelFont();
  xFont.setPointSize(12);
  xFont.setFamily(QStringLiteral("微软雅黑"));
  axisRect->axis(QCPAxis::atBottom)->setLabelFont(xFont);

  axisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::white);
  axisRect->axis(QCPAxis::atLeft)->setLabelColor(QColor(180, 200, 200));
  axisRect->axis(QCPAxis::atLeft)->setBasePen(basePen);
  axisRect->axis(QCPAxis::atLeft)->setTickPen(tickPen);
  axisRect->axis(QCPAxis::atLeft)->setSubTickPen(subTickPen);
  QFont yFont(xFont);
  yFont.setPointSize(12);
  axisRect->axis(QCPAxis::atLeft)->setLabelFont(yFont);
}

void CustomPlotLayer::initCustomPlot(QCustomPlot* customPlot) {
  customPlot->setBackground(QColor(30, 30, 30));
  customPlot->setNoAntialiasingOnDrag(true);
  customPlot->setPlottingHints(QCP::phFastPolylines);

  // 设置基本坐标轴（左侧Y轴和下方X轴）可拖动、可缩放、曲线可选、legend可选、设置伸缩比例，使所有图例可见
  customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectItems);
  customPlot->legend->setVisible(true);
  customPlot->legend->setFont(QFont("Arial", 9));
  customPlot->legend->setSelectedFont(QFont("Arial", 10));
  customPlot->legend->setBrush(QColor(255, 255, 255, 100));
  customPlot->legend->setBorderPen(QColor(255, 255, 255, 255));
  customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft); // 设置图例的位置为右上角

  initAxisRect(customPlot->axisRect());
}

void CustomPlotLayer::appedObjectData(ObjectData* data, const QString& name) {
  // 确保没有添加
  for (int i = 0; i < bindList_.size(); i++) {
    if (bindList_[i].data == data) {
      return;
    }
  }

  CustomPlotMapBind bind;
  bind.data = data;
  bind.graph = this->addGraph();
  bind.graph->setName(name);
  bind.graph->setAdaptiveSampling(true);
  bind.color = ColorScheme::getColor(bindList_.size());
  bind.graph->setPen(QPen(bind.color));
  bindList_.append(bind);
}
void CustomPlotLayer::init() {
  setupWidgetsControls();
  setupSignalConnection();
}
void CustomPlotLayer::setConfiguration(std::shared_ptr<Configuration> config) {
  config_ = config;
}

void CustomPlotLayer::setupWidgetsControls() {
  initCustomPlot(this);
}


CustomPlotMap::CustomPlotMap::CustomPlotMap(QWidget* parent) 
: QWidget(parent){


}

CustomPlotMap::~CustomPlotMap() {


}

void CustomPlotMap::init() {
  setupWidgetsControls();
  setupSignalConnection();

  addCustomPlotLayer();
}

void CustomPlotMap::setConfiguration(std::shared_ptr<Configuration> config) {
  config_ = config;
}

void CustomPlotMap::updatePlot() {
  for (const auto & obj : layerList) {
    obj->updatePlot();
  }

}

void CustomPlotMap::setupSignalConnection() {

  connect(Action_Plot_ObtPlot_Add, &QAction::triggered, [this](bool ok) {
    addCustomPlotLayer();
  });

  connect(Action_Plot_ObtPlot_Del, &QAction::triggered, [this](bool ok) {
    delCustomPlotLayer();
  });

  connect(this, &QCustomPlot::customContextMenuRequested, [this](QPoint pos) {
    Action_Plot_ObtPlot_Del->setEnabled(false);
    menu.exec(this->mapToGlobal(pos));
  });
}

void CustomPlotMap::setupWidgetsControls() {

  QVBoxLayout* layout = new QVBoxLayout(this);
  splitter = new QSplitter(Qt::Vertical, this);
  layout->addWidget(splitter);
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);

  this->setContextMenuPolicy(Qt::CustomContextMenu);

  QString css = "QMenu{"
    "background:rgba(255,255,255,1);"
    "border:none;"
    "}"
    "QMenu::item{"
    "padding:11px 10px;"
    "color:rgba(51,51,51,1);"
    "font-size:12px;"
    "}"
    "QMenu::item:hover{"
    "background-color:#409CE1;"
    "}"
    "QMenu::item:selected{"
    "background-color:#409CE1;"
    "}";

  menu.setStyleSheet(css);

  Action_Plot_ObtPlot_Add = menu.addAction(QStringLiteral("添加子图"));
  Action_Plot_ObtPlot_Del = menu.addAction(QStringLiteral("删除该子图"));
}

void CustomPlotMap::addCustomPlotLayer() {
  CustomPlotLayer* layer = new CustomPlotLayer();
  layer->setConfiguration(config_);
  layer->init();

connect(layer, &CustomPlotLayer::rightClicked, [this]() {
    Action_Plot_ObtPlot_Del->setEnabled(true);
    menu.exec(QCursor::pos());
  });

  splitter->addWidget(layer);
  splitter->setContentsMargins(0, 0, 0, 0);
  layerList.append(layer);

  resetSplitterLayout();
}

void CustomPlotMap::delCustomPlotLayer() {
  QPoint localPos = splitter->mapFromGlobal(QCursor::pos());
  CustomPlotLayer* clickedLayer = nullptr;

  // 遍历 QSplitter 中的所有控件
  int customPlotLayerCount = splitter->count();
  for (int i = 0; i < customPlotLayerCount; ++i) {
    QWidget* widget = splitter->widget(i);
    if (widget->geometry().contains(localPos)) {
      clickedLayer = qobject_cast<CustomPlotLayer*>(widget);
      layerList.removeOne(clickedLayer);
      clickedLayer->deleteLater();

      // 最低存留一个
      if (customPlotLayerCount == 1) {
        addCustomPlotLayer();
        return;
      }

      break;
    }
  }

  resetSplitterLayout();
}

void CustomPlotMap::resetSplitterLayout() {
  // 重新设置大小
  QList<int> sizes;
  for (int i = 0; i < splitter->count(); i++) {
    sizes.append(1);  // 每个控件分配相同的比例
  }
  splitter->setSizes(sizes);
}