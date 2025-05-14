#include "robotic_user_interface/plot/DataSourceViewer.h"
  
DataSourceViewer::DataSourceViewer(QWidget *parent)
  : QWidget(parent){
  
  QString scrollAreaStyleSheet =
      "QWidget {"
      "    background-color: rgba(255, 255, 255, 0);"
      "    border: none;"
      "}"
      "QScrollBar:vertical {"
      "    width: 15px;"
      "    background: rgba(0, 0, 0, 0%);"
      "    margin: 0px, 0px, 0px, 0px;"
      "}"
      "QScrollBar::handle:vertical {"
      "    width: 5px;"
      "    background: rgba(216, 185, 195, 255);"
      "    min-height: 12;"
      "}"
      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
      "    height: 0px;"
      "    width: 0px;"
      "    subcontrol-position: top;"
      "}"
      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
      "    background: rgba(0, 0, 0, 0);"
      "}";

  this->setStyleSheet(scrollAreaStyleSheet);

  treeWidget = new DataSourceViewerTreeWidget(this);
  objectNodeViewer_ = std::make_shared<ObjectNodeViewer>();

  QVBoxLayout *layout = new QVBoxLayout(this);
  treeWidget->setHeaderLabels({ "ObjectNode Tree" , "Value"});
  treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  layout->addWidget(treeWidget);
}
DataSourceViewer::~DataSourceViewer(){
  
}

void DataSourceViewer::updateDataSource(const DataSource::Ptr &ds){
  dataSource_ = ds;
  auto rootNode = dataSource_->topNode();

  treeWidget->clear();
  for (auto &node : rootNode->children)  {
    QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget, QStringList(node->name));
    treeWidget->addTopLevelItem(item);
    objectNodeViewer_->populateTree(item, node);
  }
  treeWidget->resizeColumnToContents(0);  // 自动调整第一列宽度以适应内容
  //treeWidget->expandAll(); // 默认展开所有节点
}

void DataSourceViewer::setConfiguration(std::shared_ptr<Configuration> config){
  config_ = config;
}

void DataSourceViewer::init(){
  setupWidgetsControls();
  setupSignalConnection();
}

void DataSourceViewer::setupWidgetsControls(){
  //treeWidget->setHeaderHidden(true);
  treeWidget->setColumnCount(2);
}

void DataSourceViewer::setupSignalConnection(){
  QObject::connect(treeWidget, &DataSourceViewerTreeWidget::readyDrag,this, &DataSourceViewer::readyDrag);
}

void DataSourceViewer::makeImage(){
  
}


void DataSourceViewer::paintEvent(QPaintEvent *event){
  
}
void DataSourceViewer::resizeEvent(QResizeEvent *event){
  
}

void DataSourceViewer::updateNodeValue() {
  objectNodeViewer_->updateViewer();
}

void DataSourceViewer::readyDrag(QTreeWidgetItem* item) {
  auto result = objectNodeViewer_->findObjectDataViewer(item);
  if (!result) return;

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