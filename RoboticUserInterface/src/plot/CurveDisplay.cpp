#include "robotic_user_interface/plot/CurveDisplay.h"

CurveDisplay::CurveDisplay(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
}

CurveDisplay::~CurveDisplay() {}

void CurveDisplay::init() {
  setupWidgetsControls();
  setupSignalConnection();

  flushTimer_.start(1000 / config_->plot.plotFlushRate);

  // 最少一个画布
  appendCustomPlot();
}

void CurveDisplay::setConfiguration(const std::shared_ptr<Configuration>& config) {
  config_ = config;
}

void CurveDisplay::setObservations(const std::shared_ptr<ObservationsBase>& obs) {
  observations_ = obs;
}

void CurveDisplay::setDataSource(const std::shared_ptr<DataSource>& ds) {
  dataSource_ = ds;
}

void CurveDisplay::setupSignalConnection() {
  QObject::connect(dataSourceViewer_, &DataSourceViewer::publishNotify, this, &CurveDisplay::publishNotify);

  QObject::connect(ui.lineEdit_cacheDuration, &QLineEdit::textChanged, [&](const QString &value) {
    uint32_t time = value.toInt();

    if (time < 1) {
      time = 1;
    } else if (time > 1000) {
      time = 1000;
    }

    config_->plot.cacheDuration = time;
    dataSource_->topNode()->setTimeWindow(time);
    ui.lineEdit_cacheDuration->setText(QString::number(time));
  });
  QObject::connect(ui.curve_moreLess, &QWSwitcherButton::selectIndexChanged, [this](int index) {
    settingsVisible(index);
  });
  QObject::connect(ui.curve_gridLine, &QWSwitcherButton::selectIndexChanged, [this](int index) {
    gridLineVisible(index);
  });
  QObject::connect(ui.curve_pauseResumeButton, &QWSwitcherButton::selectIndexChanged, [this](int index) {
    setPaused(index);
  });
  QObject::connect(ui.AutoScaleAxisY, &QWSwitcherButton::selectIndexChanged, [this](int index) {
     config_->plot.yAutoScale = index;
  });
  QObject::connect( ui.AutoScaleAxisX, &QWSwitcherButton::selectIndexChanged, [this](int index) {
     config_->plot.xAutoScale = index;
   });
  QObject::connect(ui.qWSwitcherButton_PlotMode, &QWSwitcherButton::selectIndexChanged, [this](int index) {
    config_->plot.plotLine = static_cast<PlotLineType>(index);
    //QCPGraph::LineStyle lineStyle;
    //QCPScatterStyle scatterStyle;
    //if (index == 1) {
    //  lineStyle = QCPGraph::LineStyle::lsNone;
    //  scatterStyle = QCPScatterStyle(QCPScatterStyle::ssDiamond, 4);
    //} else if (index == 2) {
    //  lineStyle = QCPGraph::LineStyle::lsLine;
    //  scatterStyle = QCPScatterStyle(QCPScatterStyle::ssSquare, 4);
    //} else {
    //  lineStyle = QCPGraph::LineStyle::lsLine;
    //  scatterStyle = QCPScatterStyle(QCPScatterStyle::ssNone);
    //}

    //int curveCount = ui.tabWidget_curve->count();
    //for (int i = 0; i < curveCount; i++) {
    //  CustomPlotMap *ptr = qobject_cast<CustomPlotMap *>(ui.tabWidget_curve->widget(i));
    //  int graphCount = ptr->graphCount();
    //  for (int j = 0; j < graphCount; j++) {
    //    ptr->graph(j)->setLineStyle(lineStyle);
    //    ptr->graph(j)->setScatterStyle(scatterStyle);
    //  }
    //  ptr->replot(QCustomPlot::rpQueuedReplot);
    //}
  });

  QObject::connect(ui.curve_clearData, &QPushButton::clicked, this, &CurveDisplay::clearAllDataSource);
  QObject::connect(ui.curve_addTab, &QPushButton::clicked, this, &CurveDisplay::appendCustomPlot);
  QObject::connect(ui.tabWidget_curve, &QTabWidget::tabBarClicked, [this](int index) {
    CustomPlotMap* castPlotMap = qobject_cast<CustomPlotMap*>(ui.tabWidget_curve->currentWidget());
    if (castPlotMap) {
      currentPlotMap = castPlotMap;
    }
    else {
      publishNotify(GCW::NotifyType::Error, QStringLiteral("错误"), QStringLiteral("获取当前Plot句柄失败"));
    }
  });
  QObject::connect(ui.tabWidget_curve->tabBar(), &QTabBar::tabBarDoubleClicked, [this](int index) {
    QString currentText = ui.tabWidget_curve->tabBar()->tabText(index);

    bool ok;
    QString newText = QInputDialog::getText(this, QStringLiteral("修改画布名称"),
        QStringLiteral("新的画布名称:"), QLineEdit::Normal, currentText, &ok);

    if (ok && !newText.isEmpty()) {
      ui.tabWidget_curve->tabBar()->setTabText(index, newText);
    }
  });
  QObject::connect(ui.tabWidget_curve, &QTabWidget::tabCloseRequested, [this](int index) {
    // 关闭相应的选项卡,要保留一个
    int tabCount = ui.tabWidget_curve->count();
    auto ptr = ui.tabWidget_curve->widget(index);
    ui.tabWidget_curve->removeTab(index);
    ptr->deleteLater();

    if (tabCount <= 1) {
      appendCustomPlot();
    }
   });
  QObject::connect(ui.tabWidget_curve, &QTabWidget::currentChanged, [this](int index) {
    currentPlotMap = qobject_cast<CustomPlotMap*>(ui.tabWidget_curve->currentWidget());
    });

  QObject::connect(&flushTimer_, &QTimer::timeout, [this]() {
    if (currentPlotMap) {
      currentPlotMap->updatePlot();
    }
    dataSourceViewer_->updateNodeValue();
  });
}

void CurveDisplay::setupWidgetsControls() {

  dataSourceViewer_ = new DataSourceViewer(this);
  dataSourceViewer_->setConfiguration(config_);
  dataSourceViewer_->init();
  dataSourceViewer_->updateDataSource(dataSource_);

  QSplitter* splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(dataSourceViewer_);
  splitter->addWidget(ui.rightWidget);
  splitter->setStretchFactor(0, 1);  // 设置控件的拉伸比例
  splitter->setStretchFactor(1, 2);  // 设置控件的拉伸比例
  splitter->setContentsMargins(0, 0, 0, 0);
  splitter->setHandleWidth(20);  // 设置分隔条的宽度 像素

  ui.gridLayout->addWidget(splitter);

  ui.curve_pauseResumeButton->setBorder(false);
  ui.curve_pauseResumeButton->setBorderRadius(5);
  ui.curve_pauseResumeButton->appendState({ true, QString("Pause"), QColor(80, 85, 90) , QString(":/svg/svg/stop.svg") });
  ui.curve_pauseResumeButton->appendState({ true, QString("Continue"), QColor(80, 85, 90) , QString(":/svg/svg/continue.svg") });

  ui.qWSwitcherButton_PlotMode->setBorder(false);
  ui.qWSwitcherButton_PlotMode->setBorderRadius(5);
  ui.qWSwitcherButton_PlotMode->appendState({ true, QString("Line"), QColor(80, 85, 90) , QString(":/svg/svg/line.svg") });
  ui.qWSwitcherButton_PlotMode->appendState({ true, QString("Point"), QColor(80, 85, 90) , QString(":/svg/svg/dot.svg") });
  ui.qWSwitcherButton_PlotMode->appendState({ true, QString("Line & Point"), QColor(80, 85, 90) , QString(":/svg/svg/dotline.svg") });

  ui.curve_moreLess->setBorder(false);
  ui.curve_moreLess->setBorderRadius(5);
  ui.curve_moreLess->appendState({ true, QString("Expand"), QColor(80, 85, 90) , QString(":/svg/svg/more.svg") });
  ui.curve_moreLess->appendState({ true, QString("Collapse"), QColor(80, 85, 90) , QString(":/svg/svg/less.svg") });
  //ui.curve_gridLine->setSelectState(1);

  ui.curve_gridLine->setBorder(false);
  ui.curve_gridLine->setBorderRadius(5);
  ui.curve_gridLine->appendState({ true, QString("Grid Show"), QColor(80, 85, 90) , QString(":/svg/svg/show.svg") });
  ui.curve_gridLine->appendState({ true, QString("Grid Hide"), QColor(80, 85, 90) , QString(":/svg/svg/hide.svg") });
  ui.curve_gridLine->setSelectState(1);

  ui.AutoScaleAxisX->setBorder(false);
  ui.AutoScaleAxisX->setBorderRadius(5);
  ui.AutoScaleAxisX->appendState({ true, QString("Manual Scaling X"), QColor(80, 85, 90) , QString(":/svg/svg/manual.svg") });
  ui.AutoScaleAxisX->appendState({ true, QString("Auto Scaling X"), QColor(80, 85, 90) , QString(":/svg/svg/auto.svg") });
  ui.AutoScaleAxisX->setSelectState(1);

  ui.AutoScaleAxisY->setBorder(false);
  ui.AutoScaleAxisY->setBorderRadius(5);
  ui.AutoScaleAxisY->appendState({ true, QString("Manual Scaling Y"), QColor(80, 85, 90) , QString(":/svg/svg/manual.svg") });
  ui.AutoScaleAxisY->appendState({ true, QString("Auto Scaling Y"), QColor(80, 85, 90) , QString(":/svg/svg/auto.svg") });
  ui.AutoScaleAxisY->setSelectState(1);
  
  ui.curve_addTab->setText("Add map");
  ui.curve_clearData->setText("Clear data");

  ui.lineEdit_cacheDuration->setLabel("Cache Duration (s)");
  ui.lineEdit_cacheDuration->setText(QString::number(config_->plot.cacheDuration));

  settingsVisible(false);
}
void CurveDisplay::appendCustomPlot(){
  CustomPlotMap *obj = new CustomPlotMap();
  obj->setConfiguration(config_);
  obj->init();
  QObject::connect(obj, &CustomPlotMap::publishNotify, this, &CurveDisplay::publishNotify);

  ui.tabWidget_curve->addTab(obj,QStringLiteral("plot map ") + QString::number(ui.tabWidget_curve->count() + 1));
  currentPlotMap = qobject_cast<CustomPlotMap*>(ui.tabWidget_curve->currentWidget());
}
void CurveDisplay::clearAllDataSource(){
//    core->getCommunicationPtr()->getDataSource()->clearData();
}

void CurveDisplay::settingsVisible(bool ok){
  if (ok) {
    ui.widget_curve_more->show();
  }
  else {
    ui.widget_curve_more->hide();
  }
}

void CurveDisplay::gridLineVisible(bool ok){
  config_->plot.gridLine = ok;
}

void CurveDisplay::setPaused(bool ok) {
  config_->plot.isPaused = ok;
}
