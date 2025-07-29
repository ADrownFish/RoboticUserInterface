#include "robotic_user_interface/plot/CustomPlotMap.h"

#include <QTreeWidgetItem>

CustomPlotLayer::CustomPlotLayer(QWidget *parent) : QCustomPlot(parent) {
  // 设置控件可接受拖放
  setAcceptDrops(true);
}
CustomPlotLayer::~CustomPlotLayer() {

}

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
  int dataSourceCount = bindList_.count();

  // 获取轴
  QCPAxis* xAxis = this->axisRect()->axis(QCPAxis::atBottom);
  QCPAxis* yAxis = this->axisRect()->axis(QCPAxis::atLeft);

  // 格子线
  xAxis->grid()->setVisible(config_->plot.gridLine);
  yAxis->grid()->setVisible(config_->plot.gridLine);

  if (!config_->plot.isPaused) {
    // 给每个数据源更新
    bool allowScale = false;
    for (size_t i = 0; i < dataSourceCount; i++) {
      auto& bind = bindList_[i];

      // 如果有一个有数据变化，都允许刷新
      allowScale |= !bind.data->isZeroData();

      // 不启用
      bind.graph->setVisible(bind.data->enable);
      
      // 如果是静态数据，之后要考虑是否要刷新
      if (!bind.data->enable  ) {
        continue;
      }

      bind.fullUpdate();
    }

    // 如果是静态数据，则不进行缩放
    bool notUpdateAxis = true;
    for (auto &bind : bindList_) {
      notUpdateAxis &= bind.data->type == ObjectData::DataType::Static;

      // 如果需要刷新一次数据的时候
      if (bind.data->refreshOnceFlag) {
        notUpdateAxis = false;
        bind.data->refreshOnceFlag = false;
      }

      if (!notUpdateAxis) {
        break;
      }
    }
    // 自动缩放 x
    if (!notUpdateAxis && config_->plot.xAutoScale) {
      updateAxisX();
    }

    // 自动缩放 y
    if (!notUpdateAxis && config_->plot.yAutoScale && allowScale) {
      updateAxisY();
    }
  }

  for (size_t i = 0; i < dataSourceCount; i++) {
    // 线形
    auto& bind = bindList_[i];
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
  }

  updateLineAndText();

  this->replot(QCustomPlot::rpQueuedReplot);
}

void CustomPlotLayer::updateAxisX(bool replot)
{
  xAxis->rescale(true);

  if (replot) {
    this->replot(QCustomPlot::rpQueuedReplot);
  }
}

void CustomPlotLayer::updateAxisY(bool replot)
{
  yAxis->rescale(true);
  yAxis->scaleRange(1.1);

  if (replot) {
    this->replot(QCustomPlot::rpQueuedReplot);
  }
}

void CustomPlotLayer::updateAxis(bool replot)
{
  xAxis->rescale(true);
  yAxis->rescale(true);

  QCPRange range = yAxis->range();
  double margin = 0.025 * (range.upper - range.lower); // 5% 的边距
  yAxis->setRange(range.lower - margin, range.upper + margin);

  if (replot) {
    this->replot(QCustomPlot::rpQueuedReplot);
  }
}


void CustomPlotLayer::updateLineAndText(bool replot)
{
  // 划线判定逻辑
  if (isMidMousePressed) {
    markLineVertical = true;
    markLineHorizontal = true;
  }
  else if (config_->plot.tracker) {
    markLineVertical = true;
    markLineHorizontal = false;
  }
  else {
    markLineVertical = false;
    markLineHorizontal = false;
  }


  // 划线显示逻辑
  scalar_t x = this->xAxis->pixelToCoord(mousePos.x());
  scalar_t y = 0;
  scalar_t xRatio = (mousePos.x() - this->axisRect()->left()) /
    (scalar_t)this->axisRect()->width();

  // 竖直线
  if (markLineVertical) {
    verticalLine->setVisible(true);

    // 更新垂直线（X固定，Y跨整个轴范围）
    auto& range = this->yAxis->range();
    verticalLine->point1->setCoords(x, range.lower);
    verticalLine->point2->setCoords(x, range.upper);

    if(isMidMousePressed){
      coordLabels_X->position->setCoords(x, range.lower);
    }
  }
  else {
    verticalLine->setVisible(false);
  }

  if (markLineHorizontal) {
    horizontalLine->setVisible(true);
    y = this->yAxis->pixelToCoord(mousePos.y());

    // 更新水平线（Y固定，X跨整个轴范围）
    auto &range = this->xAxis->range();
    horizontalLine->point1->setCoords(range.lower, y);
    horizontalLine->point2->setCoords(range.upper, y);

    if (isMidMousePressed) {
      coordLabels_Y->position->setCoords(range.lower, y);
    }
  }
  else {
    horizontalLine->setVisible(false);
  }
  if (markLineVertical) {
    for (auto bind : bindList_) {
      auto label = coordLabels[bind.graph].itemText;
      auto tracer = coordLabels[bind.graph].itemTracer;

      label->setVisible(true);
      label->setColor(bind.color);
      tracer->setVisible(true);
      tracer->setBrush(bind.color);

      auto data = bind.graph->data();
      auto it = data->findBegin(x);
      if (it != data->constEnd()) {
        double value = it->value;

        label->setText(QString::number(value, 'g', 6));
        Qt::Alignment xAlign = ((xRatio > 0.8) ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter;
        label->setPositionAlignment(xAlign);
        label->position->setCoords(x, value);
        tracer->position->setCoords(x, value);
      }
    }
  }
  else {
    for (auto bind : bindList_) {
      auto label = coordLabels[bind.graph].itemText;
      auto tracer = coordLabels[bind.graph].itemTracer;

      label->setVisible(false);
      tracer->setVisible(false);
    }
  }

  // 鼠标中键 显示axis数值
  if (isMidMousePressed) {
    coordLabels_X->setVisible(true);
    coordLabels_Y->setVisible(true);

    coordLabels_X->setText(QString::number(x, 'g', 6));
    coordLabels_Y->setText(QString::number(y, 'g', 6));
  } else {
    coordLabels_X->setVisible(false);
    coordLabels_Y->setVisible(false);
  }

  if (replot) {
    this->layer("overlay")->replot();
  }
}

QList<CustomPlotMapBind>& CustomPlotLayer::bindList()
{
  return bindList_;
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

void CustomPlotLayer::initItemText(QCPItemText* itemText)
{
  itemText->setPositionAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
  itemText->setColor(QColor(200,250,255));
  itemText->position->setType(QCPItemPosition::ptPlotCoords);
  //itemText->position->setCoords(0.05, 0.95);  // 右上角显示
  itemText->setText("-");
  itemText->setVisible(false);
  itemText->setSelectable(false);
  itemText->setLayer("overlay");
  itemText->setPositionAlignment(Qt::AlignCenter);  // 文本右对齐
  itemText->setBrush(QBrush(QColor(60, 60, 60, 220)));  // 背景画刷
  itemText->setPadding(QMargins(5, 2, 5, 2));          // 文字边距（增加背景范围）
  //itemText->setPen(QPen(QColor(255, 255, 255), 1));  // 边框画笔
}

void CustomPlotLayer::initItemTracer(QCPItemTracer *itemTracer){
  itemTracer->setVisible(false);
  itemTracer->setSelectable(false);
  itemTracer->setLayer("overlay");
  itemTracer->setStyle(QCPItemTracer::tsCircle);
  itemTracer->setSize(12);
}

void CustomPlotLayer::initCustomPlot(QCustomPlot* customPlot) {
  // 启用鼠标跟踪
  customPlot->setMouseTracking(true);

  customPlot->setBackground(QColor(30, 30, 30));
  customPlot->setNoAntialiasingOnDrag(true);
  customPlot->setPlottingHints(QCP::phFastPolylines);

  // 设置基本坐标轴（左侧Y轴和下方X轴）可拖动、可缩放、曲线可选、legend可选、设置伸缩比例，使所有图例可见
  customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | /*QCP::iSelectLegend |*/ QCP::iSelectItems);
  customPlot->legend->setVisible(true);
  QFont font = customPlot->font();
  font.setPointSize(9);
  customPlot->legend->setFont(font);
  customPlot->legend->setSelectedFont(font);
  customPlot->legend->setBrush(QColor(40, 40, 40, 200));
  customPlot->legend->setBorderPen(QColor(40, 100, 150));
  customPlot->legend->setTextColor(QColor(200, 200, 200));
  customPlot->legend->setSelectedTextColor(QColor(255, 255, 255));
  customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft); // 设置图例的位置为右上角

  customPlot->setNoAntialiasingOnDrag(true); // 禁用抗锯齿（拖拽时优化）

  // 垂直线（跟随X坐标）
  verticalLine = new QCPItemStraightLine(customPlot);
  verticalLine->setPen(QPen(markColor, 1, Qt::DashLine));  // 红色虚线
  verticalLine->point1->setCoords(0, 0);  // 起点 (x, y轴下限)
  verticalLine->point2->setCoords(0, 1);  // 终点 (x, y轴上限)
  verticalLine->setVisible(false);        // 初始隐藏

  // 水平线（跟随Y坐标）
  horizontalLine = new QCPItemStraightLine(customPlot);
  horizontalLine->setPen(QPen(markColor, 1, Qt::DashLine));
  horizontalLine->point1->setCoords(0, 0);  // 起点 (x轴下限, y)
  horizontalLine->point2->setCoords(1, 0);  // 终点 (x轴上限, y)
  horizontalLine->setVisible(false);

  // 坐标标签（显示X,Y值）
  coordLabels_X = new QCPItemText(customPlot);
  coordLabels_X->setClipToAxisRect(false);  // 允许绘制到轴区域
  coordLabels_X->setBrush(QBrush(QColor(60, 80, 80)));  // 背景画刷
  coordLabels_Y = new QCPItemText(customPlot);
  coordLabels_Y->setClipToAxisRect(false);  // 允许绘制到轴区域
  coordLabels_Y->setBrush(QBrush(QColor(60, 80, 80)));  // 背景画刷
  initItemText(coordLabels_X);
  initItemText(coordLabels_Y);

  // 禁止选中
  verticalLine->setSelectable(false);  
  verticalLine->setLayer("overlay");

  horizontalLine->setSelectable(false);
  horizontalLine->setLayer("overlay");

  customPlot->xAxis->setSelectableParts(QCPAxis::spNone);  // 不可点击任何部分
  customPlot->yAxis->setSelectableParts(QCPAxis::spNone);


  // 鼠标按下事件（中键触发）
  connect(customPlot, &QCustomPlot::mousePress, this, [=](QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
      isMidMousePressed = true;

      QMouseEvent fakeMove(event->type(), event->pos(), Qt::NoButton, Qt::NoButton, event->modifiers());
      mouseMoveEvent(&fakeMove);  // 手动触发移动事件
    }
    });

  // 鼠标释放事件（可选：中键释放时隐藏十字线）
  connect(customPlot, &QCustomPlot::mouseRelease, this, [=](QMouseEvent* event) {
    if (event->button() == Qt::MiddleButton) {
      isMidMousePressed = false;
    }
    });

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

  if(!coordLabels.contains(bind.graph)){
    auto& coordLabel = coordLabels[bind.graph];
    coordLabel.itemText = new QCPItemText(this);
    coordLabel.itemTracer = new QCPItemTracer(this);

    initItemText(coordLabel.itemText);
    initItemTracer(coordLabel.itemTracer);
  }
  
  bind.graph->setData(data->time, data->data);
  updateAxis(true);
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
    obj->legend->setVisible(config_->plot.legend);
  }

}

void CustomPlotMap::setupSignalConnection() {

  // 主菜单
  FluAction* ObtPlot_add = new FluAction(tr("Add Subplot"));                                 // 添加子图
  FluAction* ObtPlot_rm = new FluAction(tr("Remove Subplot"));                            // 删除子图
  FluAction* ObtPlot_edit = new FluAction(tr("Edit Subplot"));                                 // 编辑子图
  FluAction* ObtPlot_rmAllCurves = new FluAction(tr("Remove All Curves"));          // 删除所有曲线
  FluAction* ObtPlot_zoomSubplot = new FluAction(tr("Zoom Subplot"));               // 缩放子图
  FluAction* ObtPlot_zoomAllSubplot = new FluAction(tr("Zoom All Subplot"));      // 缩放所有子图

  // 线形子菜单
  FluMenu* menu_lineShape = new FluMenu();
  menu_lineShape->setTitle(tr("Line Shape"));
  FluAction* lineShape_line = new FluAction(tr("Line"));                      
  FluAction* lineShape_point = new FluAction(tr("Point"));              
  FluAction* lineShape_pointLine = new FluAction(tr("Point & Line"));              
  menu_lineShape->addAction(lineShape_line);
  menu_lineShape->addAction(lineShape_point);
  menu_lineShape->addAction(lineShape_pointLine);
  QObject::connect(lineShape_line, &QAction::triggered, [this](bool ok) {
    config_->plot.plotLine = PlotLineType::Line;
    });
  QObject::connect(lineShape_point, &QAction::triggered, [this](bool ok) {
    config_->plot.plotLine = PlotLineType::Point;
    });
  QObject::connect(lineShape_pointLine, &QAction::triggered, [this](bool ok) {
    config_->plot.plotLine = PlotLineType::LinePoint;
    });

  FluMenu* menu_rm_it = new FluMenu();
  menu_rm_it->setTitle(tr("Remove It"));

  menu.addAction(ObtPlot_add);
  menu.addAction(ObtPlot_rm);
  menu.addAction(ObtPlot_edit);
  menu.addSeparator();
  menu.addAction(ObtPlot_rmAllCurves);
  menu.addMenu(menu_rm_it);
  menu.addSeparator();
  menu.addAction(ObtPlot_zoomSubplot);
  menu.addAction(ObtPlot_zoomAllSubplot);
  menu.addSeparator();
  menu.addMenu(menu_lineShape);

  connect(ObtPlot_add, &QAction::triggered, [this](bool ok) {
    addCustomPlotLayer();
  });

  connect(ObtPlot_rm, &QAction::triggered, [this](bool ok) {
    delCustomPlotLayer();
  });

  connect(ObtPlot_zoomSubplot, &QAction::triggered, [this](bool ok) {
    QPoint pos_toggle = splitter->mapFromGlobal(menuActivatePos);
    CustomPlotLayer* clickedLayer = getCustomPlotLayer(pos_toggle);
    if (clickedLayer) {
      clickedLayer->updateAxis(true);
    }
    });

  connect(ObtPlot_zoomAllSubplot, &QAction::triggered, [this](bool ok) {
    for (auto layer : layerList) {
      layer->updateAxis(true);
    }
    });

  connect(ObtPlot_rmAllCurves, &QAction::triggered, [this](bool ok) {
    QPoint pos_toggle = splitter->mapFromGlobal(menuActivatePos);
    CustomPlotLayer* clickedLayer = getCustomPlotLayer(pos_toggle);
    if (clickedLayer) {

      auto& binds = clickedLayer->bindList();
      for (auto& bind : binds) {
        clickedLayer->removeGraph(bind.graph);

        // 删除对应坐标标签
        auto label_p = clickedLayer->coordLabels[bind.graph].itemText;
        auto tracer_p = clickedLayer->coordLabels[bind.graph].itemTracer;
        clickedLayer->removeItem(label_p);
        clickedLayer->removeItem(tracer_p);
        clickedLayer->coordLabels.remove(bind.graph);
      }
      binds.clear();
    }
    });

  connect(menu_rm_it, &QMenu::aboutToShow, [this, menu_rm_it]() {
    menu_rm_it->clear();
    QPoint pos_toggle = splitter->mapFromGlobal(menuActivatePos);
    CustomPlotLayer* clickedLayer = getCustomPlotLayer(pos_toggle);
    if (clickedLayer) {

      auto & binds = clickedLayer->bindList();
      for (auto& bind : binds) {
        FluAction* it = new FluAction(bind.graph->name());
        menu_rm_it->addAction(it);

        QObject::connect(it, &QAction::triggered, [this, clickedLayer, graph = bind.graph, bind_it = &bind , &binds]() {
          // 删除对应坐标标签
          if (!clickedLayer->coordLabels.contains(graph)) {
            publishNotify(GCW::NotifyType::Warning, "Warning", "Can not delete the coordinate label");
            return;
          }
          auto label_p = clickedLayer->coordLabels[graph].itemText;
          auto tracer_p = clickedLayer->coordLabels[graph].itemTracer;
          clickedLayer->removeItem(label_p);
          clickedLayer->removeItem(tracer_p);
          clickedLayer->coordLabels.remove(graph);

          binds.removeOne(*bind_it);
          clickedLayer->removeGraph(graph);
          });
      }
    }
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


}

CustomPlotLayer* CustomPlotMap::getCustomPlotLayer(const QPoint& point)
{
  CustomPlotLayer* clickedLayer = nullptr;

  // 遍历 QSplitter 中的所有控件
  int customPlotLayerCount = splitter->count();
  for (int i = 0; i < customPlotLayerCount; ++i) {
    QWidget* widget = splitter->widget(i);
    if (widget->geometry().contains(point)) {
      clickedLayer = qobject_cast<CustomPlotLayer*>(widget);
      if(clickedLayer){
        return clickedLayer;
      } else {
        return nullptr;
      }
    }
  }

  return nullptr;
}

void CustomPlotMap::layerAxisScalingChanged(CustomPlotLayer* p, const QCPRange& range)
{
  if (!config_->plot.link) {
    return;
  }
  for (auto layer : layerList) {
    if (layer != p) {
      layer->xAxis->setRange(range);
      layer->layer("main")->replot();
    }
  }
}

void CustomPlotMap::addCustomPlotLayer() {
  CustomPlotLayer* layer = new CustomPlotLayer();
  layer->setConfiguration(config_);
  layer->init();

  connect(layer, &CustomPlotLayer::rightClicked, [this]() {
    menuActivatePos = QCursor::pos();
    menu.exec(menuActivatePos);
  });
  connect(layer->xAxis, qOverload<const QCPRange&>(&QCPAxis::rangeChanged),
    [this, layer](const QCPRange& range) {
      layerAxisScalingChanged(layer, range);
    });

  // 鼠标移动事件（更新十字线位置）
  connect(layer, &QCustomPlot::mouseMove, [this, layer](QMouseEvent* e) {
    QPoint pos = e->pos();
    double x = layer->xAxis->pixelToCoord(pos.x());
    double y = layer->yAxis->pixelToCoord(pos.y());

    for (auto it : layerList) {
      auto &p = it->mousePoint();
      p.setX(it->xAxis->coordToPixel(x));
      p.setY(it->yAxis->coordToPixel(y));
      it->updateLineAndText(true);
    }
    });

  splitter->addWidget(layer);
  splitter->setContentsMargins(0, 0, 0, 0);
  layerList.append(layer);

  resetSplitterLayout();
}

void CustomPlotMap::delCustomPlotLayer() {
  QPoint pos_toggle = splitter->mapFromGlobal(menuActivatePos);
  int customPlotLayerCount = splitter->count();
  CustomPlotLayer* clickedLayer = getCustomPlotLayer(pos_toggle);

  if (clickedLayer) {
    layerList.removeOne(clickedLayer);
    clickedLayer->deleteLater();

    // 最低存留一个
    if (customPlotLayerCount == 1) {
      addCustomPlotLayer();
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