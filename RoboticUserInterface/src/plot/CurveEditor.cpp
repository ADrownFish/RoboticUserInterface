#include "robotic_user_interface/plot/CurveEditor.h"


CurveEditor::CurveEditor(QWidget *parent)
: QDialog(parent)
{
  ui.setupUi(this);
  
}

CurveEditor::~CurveEditor(){

  
}


void CurveEditor::init(){
  setupWidgetsControls();
  setupSignalConnection();
}


void CurveEditor::setConfiguration(const std::shared_ptr<Configuration>& config){
  config_ = config;
}


void CurveEditor::setObservations(const std::shared_ptr<ObservationsBase>& obs){
  observations_ = obs;
}


void CurveEditor::setDataSource(const std::shared_ptr<DataSource>& ds){
  dataSource_ = ds;
}

void CurveEditor::execEditor(CustomPlotLayer *layer){
  layer_ = layer;

  auto list = layer_->bindList();
  ui.widget_data->clearUnit();
  
  auto &topNode = dataSource_->topNode();
  for(auto& bind : list){
    ui.widget_data->addUnit(topNode->findPathFromObjectData(bind.data, config_->plot.leggedTopNode));
  }

  this->exec();
}

void CurveEditor::setupSignalConnection(){
  QObject::connect(ui.widget_data, &QWWindowButton::selectUnitIndexChanged, [this](int index){
    auto &list = layer_->bindList();
    auto &bind = list[index];
    
    // QString name = data->name;
  });
  
}


void CurveEditor::setupWidgetsControls(){
  ui.widget_data->setFixedUnitSize(30);
  ui.widget_data->setIntervalDistance(2);
  ui.widget_data->setDirection(QWWindowButton::Direction::Vertical);
  ui.widget_data->setBackgroundColor(QColor(255, 255, 255, 0));
  ui.widget_data->setUnitSizeMode(QWWindowButton::UnitSizeMode::Fixed);
}


