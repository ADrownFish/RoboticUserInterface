#include "robotic_user_interface/plot/DataSourceViewer.h"

#include <QFileInfo>

ObjectNodeViewer::ObjectNodeViewer() {

}

ObjectNodeViewer::~ObjectNodeViewer() {
  for (auto& d : children) {
    d.reset();
  }

  for (auto& d : data) {
    d.reset();
  }
}

void ObjectNodeViewer::addNode(ObjectNodeViewer::Ptr d) { children.append(d); }

void ObjectNodeViewer::addObject(ObjectDataViewer::Ptr d) { data.append(d); }

void ObjectNodeViewer::updateViewer() {
  for (auto& d : data) {
    if (d->data->data.count()) {
      scalar_t value = d->data->data.back();
      d->item->setText(1, QString::number(value, 'f', 2));
    } else {
      d->item->setText(1, QString::number(0, 'f', 2));
    }
  }

  for (auto& d : children) {
    d->updateViewer();
  }
}

ObjectDataViewer::Ptr ObjectNodeViewer::findObjectDataViewer(const  QTreeWidgetItem* item) const {
  for (auto& d : data) {
    if (d->item == item) {
      return d;
    }
  }

  for (auto& d : children) {
    ObjectDataViewer::Ptr result = d->findObjectDataViewer(item);
    if (result) {
      return result;
    }
  }

  return nullptr;
}

void ObjectNodeViewer::clear() {
  for (auto& d : data) {
    d->data->clear();
    d.reset();
  }
  data.clear();

  for (auto& d : children) {
    d->clear();
  }
  children.clear();
}

void ObjectNodeViewer::clearData()
{
  for (auto& d : data) {
    d->data->clear();
  }

  for (auto& d : children) {
    d->clearData();
  }
}

void ObjectNodeViewer::populateTree(QTreeWidgetItem* parentItem, ObjectNode::Ptr node) {
  // qDebug() << parentItem->flags();
  parentItem->setFlags(Qt::ItemIsEnabled);
  item = parentItem;

  for (const auto& dataObj : node->data) {
    auto dataViewer = std::make_shared<ObjectDataViewer>(dataObj);
    QTreeWidgetItem* dataItem = new QTreeWidgetItem(item, QStringList(dataObj->name));
    dataItem->setFlags(Qt::ItemIsSelectable|Qt::ItemIsDragEnabled|Qt::ItemIsDropEnabled|Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
    dataViewer->item = dataItem;

    addObject(dataViewer);
  }

  for (const auto& childNode : node->children) {
    auto childViewer = std::make_shared<ObjectNodeViewer>();
    QTreeWidgetItem * newItem = new QTreeWidgetItem(parentItem);
    newItem->setText(0, childNode->name);
    childViewer->populateTree(newItem, childNode);

    addNode(childViewer);
  }
}


 


DataSourceViewerTreeWidget::DataSourceViewerTreeWidget(QWidget* parent)
: QTreeWidget(parent) {
  setDragEnabled(true);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setContextMenuPolicy(Qt::CustomContextMenu);

  menu = menu.create();
  action_eliminate_selection = new FluAction(tr("Eliminate selection"));
  action_clear_item_data = new FluAction(tr("Clear item data"));
  action_delete_item = new FluAction(tr("Delete item"));

  menu->addAction(action_eliminate_selection);
  menu->addSeparator();
  menu->addAction(action_clear_item_data);
  menu->addAction(action_delete_item);

  connect(action_eliminate_selection, &QAction::triggered, this, &DataSourceViewerTreeWidget::clearSelection);

  connect(action_clear_item_data, &QAction::triggered, this, &DataSourceViewerTreeWidget::clearItem);

  connect(action_delete_item, &QAction::triggered, this, &DataSourceViewerTreeWidget::deleteItem);

  connect(this, &QTreeWidget::itemExpanded, this, &DataSourceViewerTreeWidget::onItemExpanded);

}

void DataSourceViewerTreeWidget::startDrag(Qt::DropActions supportedActions) {
  QList<QTreeWidgetItem*> items = selectedItems();
  if (items.isEmpty())
    return;

  emit readyDrag(items);
}

void DataSourceViewerTreeWidget::onItemExpanded(QTreeWidgetItem* item) {
  this->resizeColumnToContents(0);  // 自动调整列宽
}

void DataSourceViewerTreeWidget::mousePressEvent(QMouseEvent* event) {

  if (event->button() != Qt::RightButton) {
    QTreeWidget::mousePressEvent(event);
    return;
  }

  QPoint pos = event->pos();
  QTreeWidgetItem* item = itemAt(pos);
  QString item_name;

  if (item && (item_name = item->text(0)) != "Plugin") {
    //只有top才能清空
    bool isTop = false;
    for (const auto& it : topNode_->children) {
      if (it->name == item_name) {
        isTop = true;
        selectedNode = it;
        selectedItem = item;
        break;
      }
    }

    action_clear_item_data->setVisible(isTop);
    action_delete_item->setVisible(isTop);
  } else {
    action_clear_item_data->setVisible(false);
    action_delete_item->setVisible(false);
    menu->exec(viewport()->mapToGlobal(pos));
  }

  menu->exec(viewport()->mapToGlobal(pos));
}

void DataSourceViewerTreeWidget::clearItem() {

  if (selectedNode) {

    selectedNode->clearData();
    selectedNode = nullptr;
    selectedItem = nullptr;

    emit checkObjectData();
  }
}

void DataSourceViewerTreeWidget::deleteItem() {
  if (selectedNode) {


    selectedItem->takeChildren();
    selectedItem->setHidden(true);

    selectedNode->clear();

    selectedNode = nullptr;
    selectedItem = nullptr;

    emit checkObjectData();
  }
}

void DataSourceViewerTreeWidget::setTopNode(const ObjectNode::Ptr& p) {
  topNode_ = p;
}


DataSourceViewer::DataSourceViewer(QWidget *parent)
  : QWidget(parent){

  setAcceptDrops(true);

  treeWidget = new DataSourceViewerTreeWidget(this);
  treeWidget->setHeaderLabels({ tr("ObjectNode Tree") , tr("Value")});
  treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  treeWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  
  layout = new QVBoxLayout(this);
  layout->addWidget(treeWidget);
  // layout->setSpacing(5);
}
DataSourceViewer::~DataSourceViewer(){

}

void DataSourceViewer::setDataSource(const DataSource::Ptr &ds){
  dataSource_ = ds;
  auto rootNode = dataSource_->topNode();
  auto node = rootNode->findObjectNode("Plugin");

  treeWidget->addTopLevelItem(item_plugin);
  viewer_plugin->populateTree(item_plugin, node);
  item_plugin->setHidden(false);

  treeWidget->resizeColumnToContents(0);  // 自动调整第一列宽度以适应内容
  treeWidget->setTopNode(dataSource_->topNode());
}

void DataSourceViewer::setConfiguration(std::shared_ptr<Configuration> config){
  config_ = config;
}

void DataSourceViewer::setSteamSolver(DataStreamSolver* ss)
{
  dataStreamSolver_ = ss;
}

void DataSourceViewer::init(){
  setupWidgetsControls();
  setupSignalConnection();
}

void DataSourceViewer::setupWidgetsControls(){
  treeWidget->setColumnCount(2);

  item_csv = new QTreeWidgetItem(treeWidget, QStringList("Csv"));
  item_plugin = new QTreeWidgetItem(treeWidget, QStringList("Plugin"));
  item_float = new QTreeWidgetItem(treeWidget, QStringList("Float"));
  item_json = new QTreeWidgetItem(treeWidget, QStringList("Json"));

  item_csv->setHidden(true);
  item_plugin->setHidden(true);
  item_float->setHidden(true);
  item_json->setHidden(true);

  viewer_plugin  = std::make_shared<ObjectNodeViewer>();
  viewer_csv     = std::make_shared<ObjectNodeViewer>();
  viewer_float   = std::make_shared<ObjectNodeViewer>();
  viewer_json    = std::make_shared<ObjectNodeViewer>();

  csvLoadDialog_ = new CsvLoadDialog();
  csvLoadDialog_->setConfiguration(config_);
  csvLoadDialog_->setSteamSolver(dataStreamSolver_);
  csvLoadDialog_->init();
  QVBoxLayout* commSelectorLayout = new QVBoxLayout;
  commSelectorLayout->setContentsMargins(0, 0, 0, 0);
  commSelectorLayout->setSpacing(0);
  SelectorDialog_ = new QtMaterialDialog(this->topLevelWidget());
  SelectorDialog_->setWindowLayout(commSelectorLayout);
  commSelectorLayout->addWidget(csvLoadDialog_);
  SelectorDialog_->hideDialog();

  menu = new FluMenu();
}

void DataSourceViewer::setupSignalConnection(){
  QColor onColor  = QColor(60, 100, 120);
  QColor offColor = QColor(63, 63, 70, 50);

  QObject::connect(treeWidget, &DataSourceViewerTreeWidget::readyDrag,this, &DataSourceViewer::readyDrag);

  QObject::connect(treeWidget, &DataSourceViewerTreeWidget::checkObjectData, this, &DataSourceViewer::checkObjectData);
  
  QObject::connect(this, &DataSourceViewer::readyLoad, this, [this](const QString& path) {
    csvLoadDialog_->loadFile(path);
    SelectorDialog_->showDialog();
    }, Qt::QueuedConnection);

  QObject::connect(csvLoadDialog_, &CsvLoadDialog::ok, this, &DataSourceViewer::flushItem);

  QObject::connect(csvLoadDialog_, &CsvLoadDialog::cancel, SelectorDialog_, &QtMaterialDialog::hideDialog);

  dropButton_ = new QWDropWidget(this);
  dropButton_->setSizePolicy(QSizePolicy::Minimum ,QSizePolicy::Minimum);
  dropButton_->setFixedHeight(30);
  

  FluAction* action_clearData = new FluAction(tr("Clear all data"));
  QObject::connect(action_clearData, &QAction::triggered, this, &DataSourceViewer::clearAllData);
  menu->addAction(action_clearData);

  FluMenu* menu_clear_item_data = new FluMenu();
  menu_clear_item_data->setTitle(tr("Clear item data"));
  QObject::connect(menu_clear_item_data, &QMenu::aboutToShow, [this, menu_clear_item_data](){
    clearItsData(menu_clear_item_data);
  });
  menu->addMenu(menu_clear_item_data);
  menu->addSeparator();

  FluMenu* menu_remove_item_data = new FluMenu();
  menu_remove_item_data->setTitle(tr("Remove item"));
  QObject::connect(menu_remove_item_data, &QMenu::aboutToShow, [this, menu_remove_item_data](){
    removeItem(menu_remove_item_data);
  });
  menu->addMenu(menu_remove_item_data);

  FluAction* action_reset_tree = new FluAction(tr("Reset tree"));
  QObject::connect(action_reset_tree, &QAction::triggered, this, &DataSourceViewer::resetTree);
  menu->addAction(action_reset_tree);
  dropButton_->setMenu(menu);

  QtMaterialRaisedButton* button;
  button = new QtMaterialRaisedButton();
  button->setFont(font());
  button->setText(tr("Importing File"));
  button->setFixedHeight(25);
  button->setBackgroundColor(offColor);
  QObject::connect(button, &QPushButton::clicked, [this](){
    QString fileName = QFileDialog::getOpenFileName(
      this->topLevelWidget(), tr("Select File"), "", tr("CSV files (*.csv);;All files (*)"));

    if(QFileInfo(fileName).exists()){
      csvLoadDialog_->loadFile(fileName);
      SelectorDialog_->showDialog();
    }
  });
  dropButton_->setWidget(button);

  dropButton_->setDropDirection(QWDropWidget::DropDirection::Up);
  dropButton_->setDropIcon(QIcon(":/svg/svg/arrow-up2.svg"));
  layout->addWidget(dropButton_);
}

void DataSourceViewer::makeImage(){
  
}


void DataSourceViewer::paintEvent(QPaintEvent *event){
  
}
void DataSourceViewer::resizeEvent(QResizeEvent *event){
  
}

void DataSourceViewer::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) {
    event->acceptProposedAction();  // 允许拖拽
  }
}

void DataSourceViewer::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    QStringList filePaths;

    for (const QUrl &url : urls) {
      if (url.isLocalFile()) {
        QString filePath = url.toLocalFile();
        if (QFileInfo(filePath).isFile()) {
          filePaths << filePath;
        }
      }
    }

    if (!filePaths.isEmpty()) {
        emit readyLoad(filePaths.first());
    }
}

void DataSourceViewer::updateNodeValue() {
  qint64 currentTime = elapsedTimer.elapsed(); // 获取当前时间（毫秒）

  if (lastEnterTime != 0 && (currentTime - lastEnterTime) < 500) {
    return;
  }
  lastEnterTime = currentTime; // 更新上次进入时间

  viewer_plugin->updateViewer();
  viewer_float->updateViewer();
  viewer_json->updateViewer();
}

QPointer<DataSourceViewerTreeWidget> DataSourceViewer::getTreeWidget() {
  return treeWidget;
}
void DataSourceViewer::readyDrag(QList<QTreeWidgetItem *> items) {
  QList<ObjectDataViewer::Ptr> validViewers;

  // 1. 遍历所有 items，收集有效的 ObjectDataViewer
  for (auto *item : items) {
    ObjectDataViewer::Ptr result;
    if ((result = viewer_plugin->findObjectDataViewer(item)) ||
        (result = viewer_csv->findObjectDataViewer(item)) ||
        (result = viewer_json->findObjectDataViewer(item)) ||
        (result = viewer_float->findObjectDataViewer(item))) {
      validViewers.append(result);
    }
  }

  if (validViewers.isEmpty()) {
    return; // 没有可拖放的对象
  }

  // 2. 准备 MIME 数据（存储多个对象的指针和名称）
  QJsonArray objectDataArray;
  QStringList objectNames;

  for (const auto &viewer : validViewers) {
    quintptr ptrValue = reinterpret_cast<quintptr>(viewer->data.get());
    objectDataArray.append(static_cast<qint64>(ptrValue));
    objectNames.append(
        dataSource_->topNode()->findObjectDataPath(viewer->data));
  }

  QJsonDocument doc(objectDataArray);
  QByteArray ptrData = doc.toJson(QJsonDocument::Compact);

  QMimeData *mimeData = new QMimeData();
  mimeData->setData("application/robotuserinterface-objectdata-list", ptrData);
  mimeData->setData("application/robotuserinterface-objectname-list",
                    objectNames.join(";").toUtf8());

  // 3. 执行拖放操作
  QDrag *drag = new QDrag(this);
  drag->setMimeData(mimeData);
  drag->exec(Qt::MoveAction);
}

void DataSourceViewer::flushItem(){
  SelectorDialog_->hideDialog();
  
  auto rootNode = dataSource_->topNode();
  auto node = rootNode->findObjectNode("Csv");

  if (item_csv) {
    item_csv->takeChildren(); // 移除并自动删除所有子项
  }

  treeWidget->addTopLevelItem(item_csv);
  viewer_csv->populateTree(item_csv, node);
  item_csv->setHidden(false);

  node->setrefreshOnce();
  publishNotify(GCW::NotifyType::Info, tr("Data loading completed"), tr("Data updated"));
}

void DataSourceViewer::clearAllData()
{
  dataSource_->clearData();
}

void DataSourceViewer::clearItsData(QMenu *subMenu)
{
  subMenu->clear();
  if(!item_plugin->isHidden()){
    FluAction *action = new FluAction(tr("Plugin"));
    subMenu->addAction(action);
    QObject::connect(action, &QAction::triggered, [this]() {
      auto p = dataSource_->topNode()->findObjectNode("Plugin");
      if(p){
        p->clearData();
      }
    });
  } 
  if(!item_csv->isHidden()){
    FluAction *action = new FluAction(tr("Csv"));
    subMenu->addAction(action);
    QObject::connect(action, &QAction::triggered, [this]() {
      auto p = dataSource_->topNode()->findObjectNode("Csv");
      if(p){
        p->clearData();
      }
    });
  } 
  if(!item_json->isHidden()){
    FluAction *action = new FluAction(tr("Json"));
    subMenu->addAction(action);
    QObject::connect(action, &QAction::triggered, [this]() {
      auto p = dataSource_->topNode()->findObjectNode("Json");
      if(p){
        p->clearData();
      }
    });
  } if(!item_float->isHidden()){
    FluAction *action = new FluAction(tr("Float"));
    subMenu->addAction(action);
    QObject::connect(action, &QAction::triggered, [this]() {
      auto p = dataSource_->topNode()->findObjectNode("Float");
      if(p){
        p->clearData();
      }
    });
  }
}

void DataSourceViewer::removeItem(QMenu *subMenu) {
  // 创建子菜单
  subMenu->clear();
  auto topNode = dataSource_->topNode();

  if(!item_csv->isHidden()){
    FluAction *action = new FluAction(tr("Csv"));
    subMenu->addAction(action);
    QObject::connect(action, &QAction::triggered, [this]() {
      auto p = dataSource_->topNode()->findObjectNode("Csv");
      if(p){
        p->clearData();
      }
      item_csv->setHidden(true);
    });
  } 
  if(!item_json->isHidden()){
    FluAction *action = new FluAction(tr("Json"));
    subMenu->addAction(action);
    QObject::connect(action, &QAction::triggered, [this]() {
      auto p = dataSource_->topNode()->findObjectNode("Json");
      if(p){
        p->clearData();
      }
      item_json->setHidden(true);
    });
  } if(!item_float->isHidden()){
    FluAction *action = new FluAction(tr("Float"));
    subMenu->addAction(action);
    QObject::connect(action, &QAction::triggered, [this]() {
      auto p = dataSource_->topNode()->findObjectNode("Float");
      if(p){
        p->clearData();
      }
      item_float->setHidden(true);
    });
  }

  emit checkObjectData();
}

void DataSourceViewer::resetTree()
{
  auto p = dataSource_->topNode()->findObjectNode("Csv");
  if(p){
    p->clear();
  }
  p = dataSource_->topNode()->findObjectNode("Float");
  if(p){
    p->clear();
  }
  p = dataSource_->topNode()->findObjectNode("Json");
  if(p){
    p->clear();
  }

  item_csv->takeChildren(); // 移除并自动删除所有子项
  item_float->takeChildren();
  item_json->takeChildren();

  item_csv->setHidden(true);
  item_float->setHidden(true);
  item_json->setHidden(true);

  viewer_csv->clear();
  viewer_float->clear();
  viewer_json->clear();

  emit checkObjectData();
}
