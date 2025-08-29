#include "robotic_user_interface/plot/CurveDisplay.h"
#include "robotic_user_interface/form/ListChoiceDialog.h"
#include "robotic_user_interface/core/TemplateMethod.h"

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
  appendCustomPlot("");
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
    } else if (time > 2000) {
      time = 2000;
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
      appendCustomPlot("");
    }
   });
  QObject::connect(ui.tabWidget_curve, &QTabWidget::currentChanged, [this](int index) {
    currentPlotMap = qobject_cast<CustomPlotMap*>(ui.tabWidget_curve->currentWidget());
    });

  QObject::connect(&flushTimer_, &QTimer::timeout, this, &CurveDisplay::updateOnce);

  QObject::connect(dataStreamSolver_, &DataStreamSolver::updateTree, dataSourceViewer_, &DataSourceViewer::flushTreeItem);
}

void CurveDisplay::setupWidgetsControls() {
  dataSourceViewer_ = new DataSourceViewer(this);
  dataSourceViewer_->setConfiguration(config_);
  dataSourceViewer_->setSteamSolver(dataStreamSolver_);
  dataSourceViewer_->setDataSource(dataSource_);
  dataSourceViewer_->init();

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
  FluMenu* menu = new FluMenu();
  FluAction* action_export_image = new FluAction(tr("Export As Image"));
  QObject::connect(action_export_image, &QAction::triggered, this, &CurveDisplay::exportDataImage);
  menu->addAction(action_export_image);

  FluAction* action_export_file = new FluAction(tr("Export As File"));
  QObject::connect(action_export_file, &QAction::triggered, this, &CurveDisplay::exportDataFile);
  menu->addAction(action_export_file);

  menu->addSeparator();

  FluAction* action_save_layout = new FluAction(tr("Save layout"));
  QObject::connect(action_save_layout, &QAction::triggered, this, &CurveDisplay::SaveLayout);
  menu->addAction(action_save_layout);

  FluAction* action_load_layout = new FluAction(tr("Load layout"));
  QObject::connect(action_load_layout, &QAction::triggered, this, &CurveDisplay::loadLayout);
  menu->addAction(action_load_layout);


  dropButton_->setMenu(menu);

  QtMaterialRaisedButton* button;
  button = new QtMaterialRaisedButton();
  button->setFont(font());
  button->setText(tr("Add Map"));
  button->setFixedHeight(25);
  button->setBackgroundColor(QColor(63, 63, 70, 50));
  QObject::connect(button, &QPushButton::clicked, this, [this](){
    appendCustomPlot("");
  });
  dropButton_->setWidget(button);

  dropButton_->setDropDirection(QWDropWidget::DropDirection::Up);
  dropButton_->setDropIcon(QIcon(":/svg/svg/arrow-up2.svg"));
  ui.horizontalLayout->insertWidget(0, dropButton_);

  // =========== 添加的菜单按钮 ===========

  settingsVisible(false);
}
CustomPlotMap* CurveDisplay::appendCustomPlot(const QString& name ){
  CustomPlotMap *obj = new CustomPlotMap();
  obj->setConfiguration(config_);
  obj->setDataSource(dataSource_);
  obj->init();

  QObject::connect(obj, &CustomPlotMap::publishNotify, this, &CurveDisplay::publishNotify);
  QObject::connect(obj, &CustomPlotMap::dragAccepted, dataSourceViewer_->getTreeWidget(), &QTreeWidget::clearSelection);
  QObject::connect(dataSourceViewer_, &DataSourceViewer::checkObjectData, obj, &CustomPlotMap::checkObjectData);

  QString tabName;
  if(name.isEmpty()){
    tabName = tr("plot map ") + QString::number(ui.tabWidget_curve->count() + 1);
  } else {
    tabName = name;
  }
  ui.tabWidget_curve->addTab(obj, tabName);
  ui.tabWidget_curve->setTabToolTip(ui.tabWidget_curve->count() - 1, tabName);

  currentPlotMap = qobject_cast<CustomPlotMap*>(ui.tabWidget_curve->currentWidget());

  return obj;
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

void exportNodeToCSV(const ObjectNode::Ptr& node,
  const QString& parentPath,
  QTextStream& out,
  QElapsedTimer& timer)
{
  // 拼接路径
  QString currentPath = parentPath.isEmpty() ? node->name : parentPath + "/" + node->name;

  // 遍历该节点的所有数据
  for (const auto& objData : node->data)
  {
    if (!objData) continue;

    // 每个 ObjectData 单独输出
    QString header = currentPath + "/" + objData->name;
    out << "Path,Time,Value\n";

    int count = qMin(objData->time.size(), objData->data.size());
    for (int i = 0; i < count; ++i)
    {
      out << "\"" << header << "\","
        << QString::number(objData->time[i], 'f', 6) << ","
        << QString::number(objData->data[i], 'f', 6)
        << "\n";

      // 按时间间隔刷新 UI
      if (timer.elapsed() >= 50) {
        QCoreApplication::processEvents();
        timer.restart();
      }
    }
  }

  // 递归导出子节点
  for (const auto& child : node->children)
  {
    if (child) {
      exportNodeToCSV(child, currentPath, out, timer);
    }
  }
}

bool exportTopNodeToCSV(const ObjectNode::Ptr& topNode, const QString& filePath)
{
  if (!topNode) return false;

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    qWarning() << "Failed to open file:" << filePath;
    return false;
  }

  QTextStream out(&file);

  QElapsedTimer timer;
  timer.start();

  exportNodeToCSV(topNode, "", out, timer);

  file.close();
  return true;
}

void CurveDisplay::exportDataFile()
{
  QString defaultName = QDir::homePath() + "/Data_Packet.csv";
  QString selectedFilter;

  QString path = QFileDialog::getSaveFileName(
    this,
    tr("Save Data File"),
    defaultName,
    tr("CSV File (*.csv)"),
    &selectedFilter
  );

  if (path.isEmpty())
    return;

  // 调用导出功能
  if (exportTopNodeToCSV(dataSource_->topNode(), path)) {
    publishNotify(GCW::NotifyType::Success, tr("Export"), tr("successful:%1").arg(path));
  }
  else {
    publishNotify(GCW::NotifyType::Warning, tr("Export"), tr("failed: %1").arg(path));
  }
}

void CurveDisplay::SaveLayout() {
  QString defaultName = QDir::homePath() + "/layout_main.json";
  QString selectedFilter;

  QString path = QFileDialog::getSaveFileName(
    this,
    tr("Save Layout File"),
    defaultName,
    tr("Json File (*.json)"),
    &selectedFilter
  );

  if (path.isEmpty())
    return;

  QJsonObject root;
  auto& tabWidget = ui.tabWidget_curve;
  bool ContainsTopLevelNode = config_->plot.leggedTopNode;
  auto topNode = dataSource_->topNode();

  // 保存tabWidget信息
  QJsonArray tabsArray;
  for (int i = 0; i < tabWidget->count(); ++i) {
    QJsonObject tabObj;
    CustomPlotMap *cpm = qobject_cast<CustomPlotMap *>(tabWidget->widget(i));
    
    if (!cpm){
      continue;
    }
      
    auto splitter = cpm->getSplitter();

    tabObj["name"] = tabWidget->tabText(i);

    if (splitter) {
      QVariantList variantList;
      QList<int> intList = splitter->sizes();
      std::transform(intList.begin(), intList.end(), 
                  std::back_inserter(variantList),
                  [](int val) { return QVariant(val); });
      tabObj["splitter"] = QJsonArray::fromVariantList(variantList);
    }

    // 保存图层信息
    QJsonArray layersArray;
    auto& layerList = cpm->getLayerList();
    for (CustomPlotLayer *layer : layerList) {
      QJsonObject layerObj;

      // 保存绑定信息
      QJsonArray bindsArray;
      for (const CustomPlotMapBind &bind : layer->bindList()) {
        QJsonObject bindObj;
        bindObj["color"] = bind.color.name(QColor::HexArgb);
        bindObj["data"]  = topNode->findPathFromObjectData(bind.data, ContainsTopLevelNode);
        bindObj["name"]  = bind.data->type;
        bindObj["type"]  = bind.data->enable;
        bindsArray.append(bindObj);
      }

      layerObj["binds"] = bindsArray;
      layersArray.append(layerObj);
    }

    tabObj["layers"] = layersArray;
    tabsArray.append(tabObj);
  }

  root["tabs"] = tabsArray;

  // 写入文件
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    return;
  }

  file.write(QJsonDocument(root).toJson());
  file.close();
  return;
}

void CurveDisplay::loadLayout() {

  QString filePath;
  QString defaultName = QDir::homePath() + "/layout_main.json";

  if(config_->plot.recentLayoutFiles.isEmpty()){

    filePath = QFileDialog::getOpenFileName(
      this,
      tr("Open Layout File"),
      defaultName,
      tr("Json File (*.json)"),
      nullptr);
    if (filePath.isEmpty())
      return;

  } else {
    QStringList displayList = config_->plot.recentLayoutFiles;
    QString &defaultChice = config_->plot.recentLayoutFile;

    displayList.append(tr("I want to select a file"));

    auto result = ListChoiceDialog::getChoice(
      this->topLevelWidget(),
      tr("Load Layout"),
      tr("Some recent layouts"),
      displayList,
      defaultChice.isEmpty() ?  displayList.first() : defaultChice
    );

    if(result.first == -1 ){
      return;
    }
    if(result.first == displayList.size() - 1 ){
      filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Layout File"),
        defaultName,
        tr("Json File (*.json)"),
        nullptr);
    }

    filePath = result.second;
    if (filePath.isEmpty())
      return;
  }

  config_->plot.recentLayoutFile = filePath;
  InsertHistory(config_->plot.recentLayoutFiles, filePath, 5);

  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    return;
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) {
    return;
  }

  auto topNode = dataSource_->topNode();

  QJsonObject root = doc.object();
  QJsonArray tabsArray = root["tabs"].toArray();

  auto& tabWidget = ui.tabWidget_curve;
  while (tabWidget->count() > 0) {
    QWidget *widget = tabWidget->widget(0);
    tabWidget->removeTab(0);
    widget->deleteLater();
  }

  // 加载每个tab
  for (const QJsonValue &tabValue : tabsArray) {
    QJsonObject tabObj = tabValue.toObject();

    // 创建新的CustomPlotMap
    QString tabName = tabObj["name"].toString();
    CustomPlotMap *cpm = appendCustomPlot(tabName);
    auto splitter = cpm->getSplitter();

    // 恢复splitter状态
    if (splitter && tabObj.contains("splitter")) {
      QJsonArray splitterArray = tabObj["splitter"].toArray();
      QList<int> sizes;
      for (const QJsonValue &sizeValue : splitterArray) {
        sizes.append(sizeValue.toInt());
      }
      splitter->setSizes(sizes);
    }

    // 恢复图层信息
    auto &layerList = cpm->getLayerList();
    for(auto& layer : layerList){
      layer->deleteLater();
    }
    layerList.clear();

    QJsonArray layersArray = tabObj["layers"].toArray();
    for (const QJsonValue &layerValue : layersArray) {
      QJsonObject layerObj = layerValue.toObject();
      CustomPlotLayer *layer = cpm->addCustomPlotLayer();

      // 恢复绑定信息
      QJsonArray bindsArray = layerObj["binds"].toArray();
      for (const QJsonValue &bindValue : bindsArray) {
        QJsonObject bindObj = bindValue.toObject();

        QString colorString(bindObj["color"].toString());
        QString name = bindObj["data"].toString();

        auto itNode = topNode->findObjectDataFromPath(name, true);
        layer->appedObjectData(itNode.get(), name, colorString);
      }
    }
  }
}
