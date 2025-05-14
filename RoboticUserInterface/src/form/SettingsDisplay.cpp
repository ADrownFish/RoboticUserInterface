#include "robotic_user_interface/form/SettingsDisplay.h"


SettingsDisplay::SettingsDisplay(QWidget *parent)
: QWWindowWidget(parent){
  ui.setupUi(this);
}

SettingsDisplay::~SettingsDisplay(){
  
}


void SettingsDisplay::init(){
  setupWidgetsControls();
  setupSignalConnection();
}


void SettingsDisplay::setConfiguration(std::shared_ptr<Configuration> config){
  config_ = config;

}


void SettingsDisplay::SettingsDisplay::setupSignalConnection(){

}


void SettingsDisplay::setupWidgetsControls(){
  setBorderRadius(8);
  setBackgroundColor(QColor(74,85,115));

  ui.widget_degRad->addUnit("Deg");
  ui.widget_degRad->addUnit("Rad");

//  ui.lineEdit_farmRate->setLabel("Farm Rate");
//  ui.lineEdit_precision->setLabel("Precision");
}


void SettingsDisplay::pushParameters(){
  ui.widget_degRad->setSelectUnitIndex(static_cast<int>(config_->display.angleUnit));
  ui.lineEdit_farmRate->setText(QString::number(config_->display.farmRate));
  ui.lineEdit_precision->setText(QString::number(config_->display.precision));
}

void SettingsDisplay::pullParameters(){
  config_->display.setAngleUnit(static_cast<AngleUnit>(ui.widget_degRad->getCurrentUnitIndex()));
  config_->display.farmRate = ui.lineEdit_farmRate->text().toFloat();
  config_->display.precision = ui.lineEdit_precision->text().toFloat();
}
