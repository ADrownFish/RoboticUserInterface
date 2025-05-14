#include "robotic_user_interface/form/ActuatorDisplay.h"

ActuatorUnit::ActuatorUnit(QWidget *parent)
: QWWindowWidget(parent){
  ui_actuator.setupUi(this);
  ui_actuator.toggle->setToggle(true);

  setBorderRadius(8);
  QColor bgColor(58, 58, 58);
  setBackgroundColor(bgColor);
}

ActuatorUnit::~ActuatorUnit() {
  
}

void ActuatorUnit::setConfiguration(std::shared_ptr<Configuration> config){
  config_ = config;
}

void ActuatorUnit::setupWidgetsControls(){
  ui_actuator.lineEdit_q->setLabel(QString("Position %1").arg(config_->display.getDegRad()));
  ui_actuator.lineEdit_dq->setLabel(QString("Velocity %1").arg(config_->display.getDegRadPerSecond()));
  ui_actuator.lineEdit_tau->setLabel("Torque Nm");
  ui_actuator.lineEdit_temp_m->setLabel("Motor Temp °C");
  ui_actuator.lineEdit_temp_d->setLabel("Driver Temp °C");
  ui_actuator.lineEdit_power->setLabel("Power W");
}

void ActuatorUnit::setActuator(const ObservationsBase::Actuator &actuator){
  if (!isDishplay()) {
    return;
  }
  actuator_ = actuator;
  ui_actuator.lineEdit_q->setText(QString::number(actuator_.pos * config_->display.getAngleScale(), 'f', config_->display.precision));
  ui_actuator.lineEdit_dq->setText(QString::number(actuator_.vel * config_->display.getAngleScale(), 'f', config_->display.precision));
  ui_actuator.lineEdit_tau->setText(QString::number(actuator_.torque, 'f', config_->display.precision));
  ui_actuator.lineEdit_temp_m->setText(QString::number(actuator_.temperature, 'f', config_->display.precision));
  ui_actuator.lineEdit_temp_d->setText(QString::number(actuator_.driverTemperature, 'f', config_->display.precision));
  ui_actuator.lineEdit_power->setText(QString::number(actuator_.power, 'f', config_->display.precision));
}

void ActuatorUnit::setTitle(const QString &title){
  ui_actuator.title->setText(title);
}

void ActuatorUnit::setupSignalConnection(){
  QObject::connect(ui_actuator.toggle, &QWSwitcher::toggled, ui_actuator.widget, &QWidget::setVisible);
}

void ActuatorUnit::init(){
  setupWidgetsControls();
  setupSignalConnection();
}




ActuatorDisplay::ActuatorDisplay(QWidget *parent)
: QWWindowWidget(parent)  {
  ui_display_.setupUi(this);

}
ActuatorDisplay::~ActuatorDisplay(){}

void ActuatorDisplay::setActuators(const QStringList &title){
  for(ActuatorUnit *unit : actuator_){
    unit->deleteLater();
  }
  
  for (int i = 0; i < title.size(); i++){
    ActuatorUnit *unit = new ActuatorUnit(this);
    unit->setConfiguration(config_);
    unit->init();
    unit->setTitle(title[i]);
    
    layout_->addWidget(unit);
    actuator_.push_back(unit);
  }
}

void ActuatorDisplay::updateActuators(const std::vector<ObservationsBase::Actuator> &actuators){
  if(actuator_.size() != actuators.size()){
    throw std::runtime_error("ActuatorDisplay::updateActuators >> actuators.size() != actuators.size()");
  }

  for (size_t i = 0; i < actuator_.size(); i++) {
    actuator_[i]->setActuator(actuators[i]);
  }
}

void ActuatorDisplay::setConfiguration(std::shared_ptr<Configuration> config){
  config_ = config;
}

void ActuatorDisplay::flushConfiguration(){
  for(ActuatorUnit *unit : actuator_){
    unit->setupWidgetsControls();
  }
}

void ActuatorDisplay::setupSignalConnection(){
  QObject::connect(ui_display_.toggle, &QWSwitcher::toggled, ui_display_.widget, &QWidget::setVisible);
}

void ActuatorDisplay::setupWidgetsControls(){
  layout_ = new QFlowLayout(ui_display_.widget);
  layout_->setContentsMargins(
    config_->display.contentsMargins,
    config_->display.contentsMargins,
    config_->display.contentsMargins,
    config_->display.contentsMargins);
  layout_->setSpacing(config_->display.spacing);
  ui_display_.widget->setLayout(layout_);

  ui_display_.toggle->setToggle(true);

  setBorderRadius(8);
  QColor bgColor(50, 50, 50);
  setBackgroundColor(bgColor);

}

void ActuatorDisplay::init(){
  setupWidgetsControls();
  setupSignalConnection();
}