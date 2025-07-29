#include "robotic_user_interface/plot/DataSourceViewer.h"

#include <QFileInfo>
  
DataSourceViewer::DataSourceViewer(QWidget *parent)
  : QWidget(parent){

  setAcceptDrops(true);

  treeWidget = new DataSourceViewerTreeWidget(this);


  QVBoxLayout *layout = new QVBoxLayout(this);
  treeWidget->setHeaderLabels({ tr("ObjectNode Tree") , tr("Value")});
  treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  layout->addWidget(treeWidget);
}
DataSourceViewer::~DataSourceViewer(){
  
}

void DataSourceViewer::updateDataSource(const DataSource::Ptr &ds){
  dataSource_ = ds;
  auto rootNode = dataSource_->topNode();
  auto node = rootNode->findObjectNode("Plugin");

  treeWidget->addTopLevelItem(item_plugin);
  viewer_plugin->populateTree(item_plugin, node);
  item_plugin->setHidden(false);

  treeWidget->resizeColumnToContents(0);  // 自动调整第一列宽度以适应内容
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
  viewer_csv       = std::make_shared<ObjectNodeViewer>();
  viewer_float     = std::make_shared<ObjectNodeViewer>();
  viewer_json      = std::make_shared<ObjectNodeViewer>();

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
}

void DataSourceViewer::setupSignalConnection(){
  QObject::connect(treeWidget, &DataSourceViewerTreeWidget::readyDrag,this, &DataSourceViewer::readyDrag);

  QObject::connect(this, &DataSourceViewer::readyLoad, this, [this](const QString& path) {
    csvLoadDialog_->loadFile(path);
    SelectorDialog_->showDialog();
    }, Qt::QueuedConnection);

  QObject::connect(csvLoadDialog_, &CsvLoadDialog::ok, this, &DataSourceViewer::flushItem);

  QObject::connect(csvLoadDialog_, &CsvLoadDialog::cancel, SelectorDialog_, &QtMaterialDialog::hideDialog);
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

void DataSourceViewer::readyDrag(QTreeWidgetItem* item) {
  ObjectDataViewer::Ptr result;

  if ((result = viewer_plugin->findObjectDataViewer(item)) == nullptr) {
    if ((result = viewer_csv->findObjectDataViewer(item)) == nullptr) {
      if ((result = viewer_json->findObjectDataViewer(item)) == nullptr) {
        if ((result = viewer_float->findObjectDataViewer(item)) == nullptr) {
          return;
        }
      }
    }
  }

  // 直接存储 `ObjectDataViewer*` 地址
  quintptr ptrValue = reinterpret_cast<quintptr>(result->data.get());
  QByteArray data;
  data.setRawData(reinterpret_cast<const char*>(&ptrValue), sizeof(ptrValue));

  QString name = dataSource_->topNode()->findObjectDataPath(result->data);

  QMimeData* mimeData = new QMimeData();
  mimeData->setData("application-robotuserinterface-objectdata", data);
  mimeData->setData("application-robotuserinterface-objectname", name.toUtf8());

  QDrag* drag = new QDrag(this);
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


