#include "robotic_user_interface/plot/CurveDisplay.h"

#include "qt_material_widgets/qtmaterialraisedbutton.h"

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

void CurveDisplay::setActivate(bool ok) {
  if(ok){
    flushTimer_.start();
  } else {
    flushTimer_.stop();
  }
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

void CurveDisplay::setSteamSolver(DataStreamSolver* ss)
{
  dataStreamSolver_ = ss;
}

void CurveDisplay::setupSignalConnection() {
  QObject::connect(dataSourceViewer_, &DataSourceViewer::publishNotify, this, &CurveDisplay::publishNotify);

  QObject::connect(ui.lineEdit_cacheDuration, &QLineEdit::editingFinished, [&]() {
    uint32_t time = ui.lineEdit_cacheDuration->text().toInt();

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
    updateOnce();
  });

  QObject::connect(ui.curve_pauseResume, &QWSwitcherButton::selectIndexChanged, [this](int index) {
    setPaused(index);
    updateOnce();
  });

  QObject::connect(ui.AutoScaleAxisY, &QWSwitcherButton::selectIndexChanged, [this](int index) {
     config_->plot.yAutoScale = index;
  });

  QObject::connect( ui.AutoScaleAxisX, &QWSwitcherButton::selectIndexChanged, [this](int index) {
     config_->plot.xAutoScale = index;
   });

  QObject::connect(ui.curve_legend, &QWSwitcherButton::selectIndexChanged, [this](int index) {
    config_->plot.legend = index;

    });

  QObject::connect(ui.curve_link, &QWSwitcherButton::selectIndexChanged, [this](int index) {
    config_->plot.link = index;
    });

  QObject::connect(ui.curve_dataTraker, &QWSwitcherButton::selectIndexChanged, [this](int index) {
    config_->plot.tracker = index;
    });
  
  QObject::connect(ui.tabWidget_curve, &QTabWidget::tabBarClicked, [this](int index) {
    CustomPlotMap* castPlotMap = qobject_cast<CustomPlotMap*>(ui.tabWidget_curve->currentWidget());
    if (castPlotMap) {
      currentPlotMap = castPlotMap;
    }
    else {
      publishNotify(GCW::NotifyType::Error, tr("error"), tr("Failed to obtain the current Plot handle."));
    }
  });
  QObject::connect(ui.tabWidget_curve->tabBar(), &QTabBar::tabBarDoubleClicked, [this](int index) {
    QString currentText = ui.tabWidget_curve->tabBar()->tabText(index);

    bool ok;
    QString newText = QInputDialog::getText(this, tr("Modify map name"),
        tr("New map name:"), QLineEdit::Normal, currentText, &ok);

    if (ok && !newText.isEmpty()) {
      ui.tabWidget_curve->tabBar()->setTabText(index, newText);
      ui.tabWidget_curve->setTabToolTip(index, newText);
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

  QObject::connect(&flushTimer_, &QTimer::timeout, this, &CurveDisplay::updateOnce);
}

void CurveDisplay::setupWidgetsControls() {
    QString css = "QMenu{"
  "background:rgba(255,255,255,1);"
  "border:none;"
  "}"
  "QMenu::item{"
  "padding:11px 10px;"
  "color:rgba(51,51,51,1);"
  // "font-size:12px;"
  "}"
  "QMenu::item:hover{"
  "background-color:#409CE1;"
  "}"
  "QMenu::item:selected{"
  "background-color:#409CE1;"
  "}";

  this->setStyleSheet(css);

  dataSourceViewer_ = new DataSourceViewer(this);
  dataSourceViewer_->setConfiguration(config_);
  dataSourceViewer_->setSteamSolver(dataStreamSolver_);
  dataSourceViewer_->init();
  dataSourceViewer_->setDataSource(dataSource_);

  QSplitter* splitter = new QSplitter(Qt::Horizontal);
  splitter->addWidget(dataSourceViewer_);
  splitter->addWidget(ui.rightWidget);
  splitter->setStretchFactor(0, 1);  // 设置控件的拉伸比例
  splitter->setStretchFactor(1, 2);  // 设置控件的拉伸比例
  splitter->setContentsMargins(0, 0, 0, 0);
  splitter->setHandleWidth(20);  // 设置分隔条的宽度 像素

  QColor onColor  = QColor(60, 100, 120);
  QColor offColor = QColor(63, 63, 70, 50);

  ui.gridLayout->addWidget(splitter);

  ui.curve_pauseResume->setBorder(false);
  ui.curve_pauseResume->setBorderRadius(5);
  ui.curve_pauseResume->appendState({ true, tr("Continue"), offColor , QString(":/svg/svg/continue.svg") });
  ui.curve_pauseResume->appendState({ true, tr("Pause"), onColor , QString(":/svg/svg/stop.svg") });
  ui.curve_pauseResume->setSelectState(config_->plot.isPaused);

  ui.curve_moreLess->setBorder(false);
  ui.curve_moreLess->setBorderRadius(5);
  ui.curve_moreLess->appendState({ true, tr("Collapse"), offColor , QString(":/svg/svg/less.svg") });
  ui.curve_moreLess->appendState({ true, tr("Expand"), onColor , QString(":/svg/svg/more.svg") });
  ui.curve_moreLess->setSelectState(0);

  ui.curve_gridLine->setBorder(false);
  ui.curve_gridLine->setBorderRadius(5);
  ui.curve_gridLine->appendState({ true, tr("Grid"), offColor , QString(":/svg/svg/hide.svg") });
  ui.curve_gridLine->appendState({ true, tr("Grid"), onColor , QString(":/svg/svg/grid.svg") });
  ui.curve_gridLine->setSelectState(config_->plot.gridLine);

  ui.AutoScaleAxisX->setBorder(false);
  ui.AutoScaleAxisX->setBorderRadius(5);
  ui.AutoScaleAxisX->appendState({ true, tr("Manual Scaling X"), offColor , QString(":/svg/svg/manual.svg") });
  ui.AutoScaleAxisX->appendState({ true, tr("Auto Scaling X"), onColor , QString(":/svg/svg/auto.svg") });
  ui.AutoScaleAxisX->setSelectState(config_->plot.xAutoScale);

  ui.AutoScaleAxisY->setBorder(false);
  ui.AutoScaleAxisY->setBorderRadius(5);
  ui.AutoScaleAxisY->appendState({ true, tr("Manual Scaling Y"), offColor , QString(":/svg/svg/manual.svg") });
  ui.AutoScaleAxisY->appendState({ true, tr("Auto Scaling Y"), onColor , QString(":/svg/svg/auto.svg") });
  ui.AutoScaleAxisY->setSelectState(config_->plot.yAutoScale);

  ui.curve_legend->setBorder(false);
  ui.curve_legend->setBorderRadius(5);
  ui.curve_legend->appendState({ true, tr("Legend"), offColor , QString(":/svg/svg/hide.svg") });
  ui.curve_legend->appendState({ true, tr("Legend"), onColor , QString(":/svg/svg/legend.svg") });
  ui.curve_legend->setSelectState(config_->plot.legend);

  ui.curve_link->setBorder(false);
  ui.curve_link->setBorderRadius(5);
  ui.curve_link->appendState({ true, tr("Link"), offColor , QString(":/svg/svg/disconnect.svg") });
  ui.curve_link->appendState({ true, tr("Link"), onColor , QString(":/svg/svg/connect.svg") });
  ui.curve_link->setSelectState(config_->plot.link);

  ui.curve_dataTraker->setBorder(false);
  ui.curve_dataTraker->setBorderRadius(5);
  ui.curve_dataTraker->appendState({ true, tr("Data Tracker"), offColor , QString(":/svg/svg/hide.svg") });
  ui.curve_dataTraker->appendState({ true, tr("Data Tracker"), onColor , QString(":/svg/svg/Cross.svg") });
  ui.curve_dataTraker->setSelectState(config_->plot.tracker);

  QTabBar* tabBar = ui.tabWidget_curve->tabBar();
  tabBar->setElideMode(Qt::ElideNone);

  ui.lineEdit_cacheDuration->setLabel(tr("Cache Duration (s)"));
  ui.lineEdit_cacheDuration->setText(QString::number(config_->plot.cacheDuration));

  // =========== 添加的菜单按钮 ===========
  dropButton_ = new QWDropWidget(this);
  FluMenu* menu = new FluMenu(this);
  FluAction* action_export_image = new FluAction(tr("Export As Image"));
  QObject::connect(action_export_image, &QAction::triggered, this, &CurveDisplay::exportDataImage);
  menu->addAction(action_export_image);
  FluAction* action_export_file = new FluAction(tr("Export As File"));
  QObject::connect(action_export_file, &QAction::triggered, this, &CurveDisplay::exportDataFile);
  menu->addAction(action_export_file);
  dropButton_->setMenu(menu);

  QtMaterialRaisedButton* button;
  button = new QtMaterialRaisedButton();
  button->setFont(font());
  button->setText(tr("Add Map"));
  button->setFixedHeight(25);
  button->setBackgroundColor(QColor(63, 63, 70, 50));
  QObject::connect(button, &QPushButton::clicked, this, &CurveDisplay::appendCustomPlot);
  dropButton_->setWidget(button);

  dropButton_->setDropDirection(QWDropWidget::DropDirection::Up);
  dropButton_->setDropIcon(QIcon(":/svg/svg/arrow-up2.svg"));
  ui.horizontalLayout->insertWidget(0, dropButton_);

  // =========== 添加的菜单按钮 ===========

  settingsVisible(false);
}
void CurveDisplay::appendCustomPlot(){
  CustomPlotMap *obj = new CustomPlotMap();
  obj->setConfiguration(config_);
  obj->setDataSource(dataSource_);
  obj->init();

  QObject::connect(obj, &CustomPlotMap::publishNotify, this, &CurveDisplay::publishNotify);
  QObject::connect(obj, &CustomPlotMap::dragAccepted, dataSourceViewer_->getTreeWidget(), &QTreeWidget::clearSelection);
  QObject::connect(dataSourceViewer_, &DataSourceViewer::checkObjectData, obj, &CustomPlotMap::checkObjectData);

  QString tabName = tr("plot map ") + QString::number(ui.tabWidget_curve->count() + 1);
  ui.tabWidget_curve->addTab(obj, tabName);
  ui.tabWidget_curve->setTabToolTip(ui.tabWidget_curve->count() - 1, tabName);

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

void CurveDisplay::updateOnce()
{
  if (currentPlotMap) {
    currentPlotMap->updatePlot();
  }

  dataSourceViewer_->updateNodeValue();
}

void CurveDisplay::clearData()
{
  dataSource_->clearData();
}

void CurveDisplay::exportDataImage()
{
  if(!currentPlotMap){
    return;
  }

  QString defaultName = QDir::homePath() + "/Data_Capture.png";
  QString selectedFilter;
  QString path = QFileDialog::getSaveFileName(
      this, tr("Save Widget Screenshot"), defaultName,
      tr("PNG Image (*.png);;JPEG Image (*.jpg *.jpeg);;Bitmap Image (*.bmp)"),
      &selectedFilter);

  if (path.isEmpty())
    return;

  QPixmap pixmap(currentPlotMap->size());
  currentPlotMap->render(&pixmap);

  bool success = false;
  if (path.endsWith(".jpg", Qt::CaseInsensitive) || path.endsWith(".jpeg", Qt::CaseInsensitive)) {
    success = pixmap.save(path, "JPEG", 100);  // 高质量
  } else {
    success = pixmap.save(path);  // 其他格式默认
  }

  if (!success) {
    publishNotify(GCW::NotifyType::Error, tr("Warn"), tr("Failed to save the screenshot."));
  } else {
    publishNotify(GCW::NotifyType::Success, tr("Success"), tr("Screenshot saved to: %1").arg(path));
  }
}

void CurveDisplay::exportDataFile()
{

}
