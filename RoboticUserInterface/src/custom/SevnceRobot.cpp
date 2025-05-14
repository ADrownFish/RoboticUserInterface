#include "robotic_user_interface/custom/SevnceRobot.h"
#include "robotic_user_interface/custom/SevnceTypes.h"

#include "ui_SevnceRobot.h"
#include "qwool/qwwindowwidget.h"

#include <QDebug>
#include <QMessageBox>


#include <stdint.h>
#include <cstdint> 
#include <string>
#include <iostream>
#include <iomanip> 
#include <sstream> 

namespace sevnce{

class SevnceData{
public:
  SevnceData(){
    reset();
  }
  ~SevnceData(){

  }

  void reset(){
    memset(&cmd, 0, sizeof(high::HighCmd));
    memset(&state, 0, sizeof(high::HighState));
  }

public:
  high::HighCmd cmd;
  high::HighState state;
};

class SevnceWidget{
public:
  SevnceWidget(){
    widget_SevnceRobot = new QWWindowWidget();
    widget_SevnceRobot->setBorderRadius(8);
    QColor bgColor(50, 50, 50);
    widget_SevnceRobot->setBackgroundColor(bgColor);
    ui_SevnceRobot.setupUi(widget_SevnceRobot);

    ui_SevnceRobot.toggle->setPenWidth(2);
    ui_SevnceRobot.toggle->setBackgroundColor(QColor(0,0,0,0));
    ui_SevnceRobot.toggle->setToggle(true);
    ui_SevnceRobot.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));


  }
  ~SevnceWidget(){

  }

  QList<QWidget *> createCustomInfoWidgets(){
    QList<QWidget *> listWidget;
    listWidget.append(widget_SevnceRobot);

    return listWidget;
  }

  

public:
  Ui::SevnceRobot ui_SevnceRobot;
  QWWindowWidget *widget_SevnceRobot;
};



SevnceRobot::SevnceRobot(QObject *parent)
: RobotBase(parent) {

  solver_.setDeviceID(0x01);
  solver_.setEnableFilter(false);
  
  sevnceData = std::make_unique<SevnceData>();
  sevnceWidget = std::make_unique<SevnceWidget>();

  QObject::connect(&timer_sendCmd, &QTimer::timeout, this, &SevnceRobot::writeData);
  timer_sendCmd.start(1000/50);
}
SevnceRobot::~SevnceRobot(){

}

QStringList SevnceRobot::getActuators() { 
  QStringList list;
  list 
    << "LF HAA   Joint 0" << "LF HFE   Joint 1" << "LF KFE   Joint 2"
    << "RF HAA   Joint 3" << "RF HFE   Joint 4" << "RF KFE   Joint 5"
    << "LH HAA   Joint 6" << "LH HFE   Joint 7" << "LH KFE   Joint 8"
    << "RH HAA   Joint 9" << "RH HFE   Joint 10" << "RH KFE   Joint 11";

  return list; 
};

std::string formatByteArray(const uint8_t *array, size_t size) {
  std::stringstream ss;
  ss << std::hex << std::setw(2) << std::setfill('0');
  for (size_t i = 0; i < size; ++i) {
    ss << std::setw(2) << static_cast<int>(array[i]) << " ";
  }
  ss << std::dec;
  return ss.str();
}

void SevnceRobot::writeData() {

  // send
  copyFromCommands();

  Data cmdData;
  cmdData.appendData(sevnceData->cmd);
  cmdData.head.dstID = 2;
  cmdData.head.DataID = 1;
  const Solver::ByteArrayPointer bap = solver_.makeDataPacket(cmdData);

  readyWrite(QByteArray((char *)bap.data, bap.size));
}

void SevnceRobot::readyRead(const QByteArray &data) {

  // receive
  solver_.pushBytes((uint8_t *)data.data(), data.size());
  while (solver_.getAvailableSize()) {
    auto data = solver_.getFirstDataPacket();

    processData(data);
  }
}

void SevnceRobot::init(int numberOfActuator, int numberOfEndEffector){
  RobotBase::init(numberOfActuator, numberOfEndEffector);

  setupWidgetsControls();
  setupSignalConnection();

  sevnceData->state.alarmStatus = -1;
}

void SevnceRobot::processData(const Data& data){
  switch (data.head.srcID)
  {
  case 0x02:{
    if(data.head.DataID == 0x01){
      if(data.length == sizeof(high::HighState)){
        memcpy(&sevnceData->state, data.data, data.length);
        copyToObservations();
        recordDataOnce();
      } else {
        qDebug() << "[SevnceRobot] Received HighState data length is not match";
      }
    }
  } break;
  
  default:
    break;
  }
}

void SevnceRobot::copyToObservations(){
  auto &s = sevnceData->state;
  // odom
  observations_->odom.position = {s.pose.x, s.pose.y, s.pose.z};
  observations_->odom.velocity = {s.velocity.xVel, s.velocity.yVel, s.velocity.zVel};
  
  // imu
  observations_->imu.quat = { s.imu.quat[0], s.imu.quat[1], s.imu.quat[2], s.imu.quat[3] };
  observations_->imu.acceleration = {s.imu.linearAcc[0], s.imu.linearAcc[1], s.imu.linearAcc[2]};

  observations_->imu.eulerAngles = {s.imu.rpy[0], s.imu.rpy[1], s.imu.rpy[2]};
  observations_->imu.angularVelocity = {s.velocity.rollVel, s.velocity.pitchVel, s.velocity.yawVel};
  observations_->imu.angularAcceleration = {0,0,0};

  // system
  observations_->system.cpuCoreMaxTemp = s.sys.cpuCoreMaxTemp;
  observations_->system.cpuPackageTemp = s.sys.cpuPackageTemp;
  observations_->system.cpuUsage       = s.sys.cpuUsage;
  observations_->system.memoryUsage    = s.sys.memoryUsage;
  observations_->system.diskUsage      = s.sys.diskUsage;
  observations_->system.status         = s.sys.alarmStatus;
  observations_->system.bodyTemp       = s.sys.bodyTemp;
  observations_->system.bodyHumidity   = s.sys.bodyHumidity;

  // bms
  observations_->battery.voltage = s.bms.voltage;
  observations_->battery.current = s.bms.current;
  observations_->battery.soc     = s.bms.soc;
  observations_->battery.cycle   = s.bms.cycle;
  observations_->battery.temp    = s.bms.temp;
  observations_->battery.status  = s.bms.status;

  for (size_t i = 0; i < numberOfActuator_; i++)  {
    auto &m = observations_->actuator;
    m[i].pos = s.joints[i].q;
    m[i].vel = s.joints[i].dq;
    m[i].torque = s.joints[i].tau;
    m[i].temperature = s.joints[i].motorTemp;
    m[i].current = s.joints[i].motorCurrent;
    m[i].voltage = s.joints[i].motorVoltage;
    m[i].driverTemperature = s.joints[i].driverTemp;
    m[i].state = s.joints[i].motorFault;
    m[i].power = s.joints[i].motorCurrent * s.joints[i].motorVoltage;
  }

  appendBaseDataSource();
}

void SevnceRobot::copyFromCommands(){
  auto &c = sevnceData->cmd;
  c.velocity.xVel = command_->input.velocity[0];
  c.velocity.yVel = command_->input.velocity[1];
  c.velocity.yawVel = command_->input.angularVelocity[2];

  c.goal.x = command_->input.position[0];
  c.goal.y = command_->input.position[1];
  c.goal.yaw = command_->input.eulerAngles[2];
}

QWidget *SevnceRobot::createCustomOperationWidget(){
  return nullptr;
}

QList<QWidget *> SevnceRobot::createCustomInfoWidgets(){
  return sevnceWidget->createCustomInfoWidgets();
}

const QString gotRobotMode(sevnce::high::RobotMode mode){
  using namespace sevnce::high;

  switch (mode)
  {
  case RobotMode::Prone :    return QString("Prone");    break;
  case RobotMode::Lying :    return QString("Lying");  break;
  case RobotMode::Squat :    return QString("Squat");  break;
  case RobotMode::Stand :    return QString("Stand");  break;
  case RobotMode::Pose :     return QString("Pose");   break;
  case RobotMode::Walk :     return QString("Walk");   break;
  case RobotMode::WalkRL1 :  return QString("RL1");    break;
  case RobotMode::WalkRL2 :  return QString("RL2");    break;
  }

  return QString("");
}
void SevnceRobot::catchData(){
  
}

void SevnceRobot::displayData(){

  Ui::SevnceRobot& ui_SevnceRobot = sevnceWidget->ui_SevnceRobot;
  auto &state__ = sevnceData->state;

  ui_SevnceRobot.lineEdit_bodyHeight  ->setText(QString::number(state__.bodyHeight, 'f', config_->display.precision)  + " m");
  ui_SevnceRobot.lineEdit_swingHeight ->setText(QString::number(state__.swingHeight, 'f', config_->display.precision) + " m");
  ui_SevnceRobot.widget_CtrlMode      ->setSelectUnitIndex(state__.ctrlMode      /* ? "是" : "否"*/ );
  ui_SevnceRobot.widget_isPerceptive  ->setSelectUnitIndex(state__.isPerceptive  /* ? "是" : "否"*/ );
  ui_SevnceRobot.widget_keepWalking   ->setSelectUnitIndex(state__.keepWalking   /* ? "是" : "否"*/ );
  ui_SevnceRobot.widget_lockedTerrain ->setSelectUnitIndex(state__.lockedTerrain /* ? "是" : "否"*/ );
  ui_SevnceRobot.widget_isSwitching   ->setSelectUnitIndex(state__.isSwitching   /* ? "是" : "否"*/ );
  ui_SevnceRobot.widget_isStance      ->setSelectUnitIndex(state__.isStance      /* ? "是" : "否"*/ );

  QString curName = gotRobotMode(state__.robotMode);
  ui_SevnceRobot.widget_robotMode->setSelectUnit(curName);

  switch(state__.walkGait){
    case sevnce::high::WalkGait::Walk:
      ui_SevnceRobot.widget_walkGait->setSelectUnitIndex(0);
      break;
    case sevnce::high::WalkGait::Run:
      ui_SevnceRobot.widget_walkGait->setSelectUnitIndex(1);
      break;
    case sevnce::high::WalkGait::Climb:
      ui_SevnceRobot.widget_walkGait->setSelectUnitIndex(2);
      break;
  }

  switch (state__.walkMode){
    case sevnce::high::WalkMode::Velocity:
      ui_SevnceRobot.widget_walkMode->setSelectUnitIndex(0);

      break;
    case sevnce::high::WalkMode::Goal:
      ui_SevnceRobot.widget_walkMode->setSelectUnitIndex(1);

      break;
  }

  ui_SevnceRobot.widget_alarmStatus->setMultiSelectUnitIndex((uint32_t)state__.alarmStatus);
}

void SevnceRobot::setupWidgetsControls(){
  QStringList alarmStatusNameList = {
      "程控板通讯", "IMU通讯",      "电机通讯",   "电机故障", "电机过热",
      "驱动器过热", "单电机超功率", "总功率超限", "电池故障", "紧急停止",
      "舱体过热",   "系统警告",     "机器人摔倒", "云台通讯",
  };
  Ui::SevnceRobot& ui_SevnceRobot = sevnceWidget->ui_SevnceRobot;

  ui_SevnceRobot.widget_walkGait->addUnit("Walk");
  ui_SevnceRobot.widget_walkGait->addUnit("Run");
  ui_SevnceRobot.widget_walkGait->addUnit("Climb");
  ui_SevnceRobot.widget_walkGait->setAllowMouseClicked(true);
  ui_SevnceRobot.widget_walkGait->setBackgroundColor(QColor(100,100,100,50));

  ui_SevnceRobot.widget_robotMode->addUnit("Stop");
  ui_SevnceRobot.widget_robotMode->addUnit("Prone");
  ui_SevnceRobot.widget_robotMode->addUnit("Stand");
  ui_SevnceRobot.widget_robotMode->addUnit("Pose");
  ui_SevnceRobot.widget_robotMode->addUnit("Walk");
  ui_SevnceRobot.widget_robotMode->addUnit("RL1");
  ui_SevnceRobot.widget_robotMode->addUnit("RL2");
  ui_SevnceRobot.widget_robotMode->setAllowMouseClicked(true);
  ui_SevnceRobot.widget_robotMode->setBackgroundColor(QColor(100,100,100,50));

  ui_SevnceRobot.widget_walkMode->addUnit("Velocity");
  ui_SevnceRobot.widget_walkMode->addUnit("Goal");
  ui_SevnceRobot.widget_walkMode->setAllowMouseClicked(true);
  ui_SevnceRobot.widget_walkMode->setBackgroundColor(QColor(100,100,100,50));

  ui_SevnceRobot.widget_CtrlMode      ->addUnit("自动");
  ui_SevnceRobot.widget_isPerceptive  ->addUnit("否");
  ui_SevnceRobot.widget_keepWalking   ->addUnit("否");
  ui_SevnceRobot.widget_lockedTerrain ->addUnit("否");
  ui_SevnceRobot.widget_isSwitching   ->addUnit("否");
  ui_SevnceRobot.widget_isStance      ->addUnit("否");

  ui_SevnceRobot.widget_CtrlMode      ->addUnit("手动");
  ui_SevnceRobot.widget_isPerceptive  ->addUnit("是");
  ui_SevnceRobot.widget_keepWalking   ->addUnit("是");
  ui_SevnceRobot.widget_lockedTerrain ->addUnit("是");
  ui_SevnceRobot.widget_isSwitching   ->addUnit("是");
  ui_SevnceRobot.widget_isStance      ->addUnit("是");

  ui_SevnceRobot.widget_CtrlMode     ->setBackgroundColor(QColor(100,100,100,50));
  ui_SevnceRobot.widget_isPerceptive  ->setBackgroundColor(QColor(100,100,100,50));
  ui_SevnceRobot.widget_keepWalking   ->setBackgroundColor(QColor(100,100,100,50));
  ui_SevnceRobot.widget_lockedTerrain ->setBackgroundColor(QColor(100,100,100,50));
  ui_SevnceRobot.widget_isSwitching   ->setBackgroundColor(QColor(100,100,100,50));
  ui_SevnceRobot.widget_isStance      ->setBackgroundColor(QColor(100,100,100,50));

  ui_SevnceRobot.widget_CtrlMode     ->setSelectdColor(QColor(50, 180, 120));
  ui_SevnceRobot.widget_isStance      ->setSelectdColor(QColor(50, 180, 120));
  ui_SevnceRobot.widget_isSwitching   ->setSelectdColor(QColor(50, 180, 120));


  for(const QString &name : alarmStatusNameList){
    ui_SevnceRobot.widget_alarmStatus->addUnit(name);
  }
  ui_SevnceRobot.widget_alarmStatus->setSelectionMode(QWWindowButton::SelectionMode::MultiSelection);
  ui_SevnceRobot.widget_alarmStatus->setMinimumWidth(ui_SevnceRobot.widget_alarmStatus->getUnitSize() * 120);
  ui_SevnceRobot.widget_alarmStatus->setAllowMouseClicked(true);
  ui_SevnceRobot.widget_alarmStatus->setBackgroundColor(QColor(100,100,100,50));
  ui_SevnceRobot.widget_alarmStatus->setSelectdColor(QColor(220, 100, 100));
}

void SevnceRobot::setupSignalConnection() {
  Ui::SevnceRobot& ui_SevnceRobot = sevnceWidget->ui_SevnceRobot;

  QObject::connect(ui_SevnceRobot.toggle,         &QWSwitcher::toggled, ui_SevnceRobot.widget,         &QWidget::setVisible);

  QObject::connect(ui_SevnceRobot.widget_robotMode, &QWWindowButton::selectUnitIndexChanged,
        [this,&ui_SevnceRobot](unsigned int index) {
          switchRobotMode(ui_SevnceRobot.widget_robotMode->getUnitName(index));
        });

  QObject::connect(ui_SevnceRobot.widget_walkGait, &QWWindowButton::selectUnitIndexChanged, [this,&ui_SevnceRobot](unsigned int index) {
    switch (index) {
    case 0:
      sevnceData->cmd.walkGait = sevnce::high::WalkGait::Walk;
      break;
    case 1:
      sevnceData->cmd.walkGait = sevnce::high::WalkGait::Run;
      break;
    case 2:
      sevnceData->cmd.walkGait = sevnce::high::WalkGait::Climb;
      break;
    }
    publishNotify(
        GCW::NotifyType::Info, QString("Command"),
        QString("Clicked: %1 %2 >> ")
                .arg(index)
                .arg(ui_SevnceRobot.widget_walkGait->getUnitName(index)) +
            QString("WalkGait to: %1").arg((int)sevnceData->cmd.walkGait));
  });

  QObject::connect(ui_SevnceRobot.widget_walkMode, &QWWindowButton::selectUnitIndexChanged, [this,&ui_SevnceRobot](unsigned int index) {
    switch (index) {
    case 0:
      sevnceData->cmd.walkMode = sevnce::high::WalkMode::Velocity;
      break;
    case 1:
      sevnceData->cmd.walkMode = sevnce::high::WalkMode::Goal;
      break;
    }
    publishNotify(
        GCW::NotifyType::Info, QString("Command"),
        QString("Clicked: %1 %2 >> ")
                .arg(index)
                .arg(ui_SevnceRobot.widget_walkMode->getUnitName(index)) +
            QString("WalkMode to: %1").arg((int)sevnceData->cmd.walkMode));
  });

  QObject::connect(ui_SevnceRobot.widget_isPerceptive, &QWWindowButton::selectUnitIndexChanged, [this](unsigned int index) {
    sevnceData->cmd.isPerceptive = index; 
  });
  QObject::connect(ui_SevnceRobot.widget_keepWalking,  &QWWindowButton::selectUnitIndexChanged, [this](unsigned int index) {
    sevnceData->cmd.keepWalking = index; 
  });
  QObject::connect(ui_SevnceRobot.widget_lockedTerrain,&QWWindowButton::selectUnitIndexChanged, [this](unsigned int index) {
    sevnceData->cmd.lockedTerrain = index; 
  });
}

void SevnceRobot::switchRobotMode(const QString &str) {

    if(str == "Stop") {   
      QMessageBox::StandardButton reply = QMessageBox::warning(
        sevnceWidget->widget_SevnceRobot,
        "This could be dangerous !",
        "Are you sure you want to stop robot?",
        QMessageBox::Yes | QMessageBox::No,       // 显示的按钮
        QMessageBox::No                           // 默认选中按钮
        );
      if (reply == QMessageBox::No) {
        return;
      }
      sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::Stop;    
      }
    if(str == "Prone"){  sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::Prone;    }
    if(str == "Stand"){   sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::Stand;   }
    if(str == "Pose") {   sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::Pose;    }
    if(str == "Walk") {   sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::Walk;    }
    if(str == "RL1")  {   sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::WalkRL1; }
    if(str == "RL2")  {   sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::WalkRL2; }

    publishNotify(GCW::NotifyType::Info, QString("Send to the robot"),
    QString("robot to: %1 %2").arg((int)sevnceData->cmd.switchCmd).arg(str));
  }

void SevnceRobot::flushConfiguration(){

}

void SevnceRobot::networkStatusChanged(bool status) {
  if(status){
    // sevnceData->state.alarmStatus = -1;
  } else {
    sevnceData->state.alarmStatus = -1;
  }
}
}
