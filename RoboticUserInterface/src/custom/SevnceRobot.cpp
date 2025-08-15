#include "robotic_user_interface/custom/SevnceRobot.h"

#include "ui_SevnceRobot_Ctrl.h"
#include "ui_SevnceRobot_State.h"

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
    QColor bgColor(50, 50, 50);

    widget_SevnceRobot_Ctrl = new QWWindowWidget();
    widget_SevnceRobot_Ctrl->setBorderRadius(8);
    widget_SevnceRobot_Ctrl->setBackgroundColor(bgColor);
    ui_SevnceRobot_Ctrl.setupUi(widget_SevnceRobot_Ctrl);

    widget_SevnceRobot_State = new QWWindowWidget();
    widget_SevnceRobot_State->setBorderRadius(8);
    widget_SevnceRobot_State->setBackgroundColor(bgColor);
    ui_SevnceRobot_State.setupUi(widget_SevnceRobot_State);

    ui_SevnceRobot_Ctrl.toggle->setPenWidth(2);
    ui_SevnceRobot_Ctrl.toggle->setBackgroundColor(QColor(0,0,0,0));
    ui_SevnceRobot_Ctrl.toggle->setToggle(true);
    ui_SevnceRobot_Ctrl.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

    ui_SevnceRobot_State.toggle->setPenWidth(2);
    ui_SevnceRobot_State.toggle->setBackgroundColor(QColor(0,0,0,0));
    ui_SevnceRobot_State.toggle->setToggle(true);
    ui_SevnceRobot_State.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));
  }
  ~SevnceWidget(){

  }

  QList<QWidget *> createCustomInfoWidgets(){
    QList<QWidget *> listWidget;
    listWidget.append(widget_SevnceRobot_Ctrl);
    listWidget.append(widget_SevnceRobot_State);

    return listWidget;
  }

  

public:
  Ui::SevnceRobot_Ctrl ui_SevnceRobot_Ctrl;
  Ui::SevnceRobot_State ui_SevnceRobot_State;
  QWWindowWidget *widget_SevnceRobot_Ctrl;
  QWWindowWidget *widget_SevnceRobot_State;
};



SevnceRobot::SevnceRobot(QObject *parent)
: RobotBase(parent) {

  solver_.setDeviceID(0x01);
  solver_.setEnableFilter(false);
  
  sevnceData = std::make_unique<SevnceData>();
  sevnceWidget = std::make_unique<SevnceWidget>();

  QObject::connect(&timer_sendCmd, &QTimer::timeout, 
                   this, static_cast<void (SevnceRobot::*)()>(&SevnceRobot::writeData));
  timer_sendCmd.start(1000/10);
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

QString SevnceRobot::getPluginName() { return QString("Sevnce"); }

// std::string formatByteArray(const uint8_t *array, size_t size) {
//   std::stringstream ss;
//   ss << std::hex << std::setw(2) << std::setfill('0');
//   for (size_t i = 0; i < size; ++i) {
//     ss << std::setw(2) << static_cast<int>(array[i]) << " ";
//   }
//   ss << std::dec;
//   return ss.str();
// }

void SevnceRobot::writeData() {

  // send
  copyFromCommands();

  Data cmdData;
  cmdData.appendData(sevnceData->cmd);
  cmdData.head.dstID = 2;
  cmdData.head.DataID = 1;
  const Solver::ByteArrayPointer bap = solver_.makeDataPacket(cmdData);

  RobotBase::writeData(QByteArray((char *)bap.data, bap.size));
}

void SevnceRobot::writeData(uint8_t dstID, uint8_t dataID, const QByteArray &data){

  Data cmdData;
  cmdData.appendData(data.data(),data.size());
  cmdData.head.dstID = dstID;
  cmdData.head.DataID = dataID;
  const Solver::ByteArrayPointer bap = solver_.makeDataPacket(cmdData);

  RobotBase::writeData(QByteArray((char *)bap.data, bap.size));
}

void SevnceRobot::readyRead() {

  // receive
  QByteArray data;
  RobotBase::readData(data);

  solver_.pushBytes((uint8_t *)data.data(), data.size());
  while (solver_.getAvailableSize()) {
    auto data = solver_.getFirstDataPacket();

    unpackData(data);
  }
}

void SevnceRobot::init(){
  int numberOfActuator = 12;
  int numberOfEndEffector = 4;
  
  RobotBase::init(numberOfActuator, numberOfEndEffector);

  setupWidgetsControls();
  setupSignalConnection();

  sevnceData->state.alarmStatus = -1;
}

void SevnceRobot::unpackData(const Data& data){
  switch (data.head.srcID)
  {
  case 0x02:{
    if(data.head.DataID == 0x01){
      if(data.length == sizeof(high::HighState)){
        memcpy(&sevnceData->state, data.data, data.length);
        copyToObservations();
        recordData();
        dataUpdate = true;
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

  // sensor
  observations_->sensor.temp       = s.sys.bodyTemp;
  observations_->sensor.humidity   = s.sys.bodyHumidity;

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

  updateDataSource(dataSource_->time());
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
  if (!dataUpdate) {
    return;
  }
  dataUpdate = false;

  auto& ui_State = sevnceWidget->ui_SevnceRobot_State;
  auto& ui_Ctrl = sevnceWidget->ui_SevnceRobot_Ctrl;
  auto &state__ = sevnceData->state;

  ui_State.lineEdit_bodyHeight  ->setText(QString::number(state__.bodyHeight, 'f', config_->display.precision)  + " m");
  ui_State.lineEdit_swingHeight ->setText(QString::number(state__.swingHeight, 'f', config_->display.precision) + " m");
  ui_State.widget_CtrlMode      ->setSelectUnitIndex(state__.ctrlMode      /* ? "是" : "否"*/ );
  ui_Ctrl.widget_isPerceptive  ->setSelectUnitIndex(state__.isPerceptive  /* ? "是" : "否"*/ );
  ui_Ctrl.widget_keepWalking   ->setSelectUnitIndex(state__.keepWalking   /* ? "是" : "否"*/ );
  ui_Ctrl.widget_lockedTerrain ->setSelectUnitIndex(state__.lockedTerrain /* ? "是" : "否"*/ );
  ui_State.widget_isSwitching   ->setSelectUnitIndex(state__.isSwitching   /* ? "是" : "否"*/ );
  ui_State.widget_isStance      ->setSelectUnitIndex(state__.isStance      /* ? "是" : "否"*/ );

  QString curName = gotRobotMode(state__.robotMode);
  ui_Ctrl.widget_robotMode->setSelectUnit(curName);

  switch(state__.walkGait){
    case sevnce::high::WalkGait::Walk:
      ui_Ctrl.widget_walkGait->setSelectUnitIndex(0);
      break;
    case sevnce::high::WalkGait::Run:
      ui_Ctrl.widget_walkGait->setSelectUnitIndex(1);
      break;
    case sevnce::high::WalkGait::Climb:
      ui_Ctrl.widget_walkGait->setSelectUnitIndex(2);
      break;
  }

  switch (state__.walkMode){
    case sevnce::high::WalkMode::Velocity:
      ui_Ctrl.widget_walkMode->setSelectUnitIndex(0);

      break;
    case sevnce::high::WalkMode::Goal:
      ui_Ctrl.widget_walkMode->setSelectUnitIndex(1);

      break;
  }

  ui_State.widget_alarmStatus->setMultiSelectUnitIndex((uint32_t)state__.alarmStatus);
}

void SevnceRobot::setupWidgetsControls(){
  QStringList alarmStatusNameList = {
      "程控板通讯", "IMU通讯",      "电机通讯",   "电机故障", "电机过热",
      "驱动器过热", "单电机超功率", "总功率超限", "电池故障", "紧急停止",
      "舱体过热",   "系统警告",     "机器人摔倒", "云台通讯",
  };
  auto& ui_State = sevnceWidget->ui_SevnceRobot_State;
  auto& ui_Ctrl = sevnceWidget->ui_SevnceRobot_Ctrl;

  ui_Ctrl.widget_walkGait->addUnit("Walk");
  ui_Ctrl.widget_walkGait->addUnit("Run");
  ui_Ctrl.widget_walkGait->addUnit("Climb");
  ui_Ctrl.widget_walkGait->setAllowMouseClicked(true);
  ui_Ctrl.widget_walkGait->setBackgroundColor(QColor(100,100,100,50));

  ui_Ctrl.widget_robotMode->addUnit("Stop");
  ui_Ctrl.widget_robotMode->addUnit("Prone");
  ui_Ctrl.widget_robotMode->addUnit("Stand");
  ui_Ctrl.widget_robotMode->addUnit("Pose");
  ui_Ctrl.widget_robotMode->addUnit("Walk");
  ui_Ctrl.widget_robotMode->addUnit("RL1");
  ui_Ctrl.widget_robotMode->addUnit("RL2");
  ui_Ctrl.widget_robotMode->setAllowMouseClicked(true);
  ui_Ctrl.widget_robotMode->setBackgroundColor(QColor(100,100,100,50));

  ui_Ctrl.widget_walkMode->addUnit("Velocity");
  ui_Ctrl.widget_walkMode->addUnit("Goal");
  ui_Ctrl.widget_walkMode->setAllowMouseClicked(true);
  ui_Ctrl.widget_walkMode->setBackgroundColor(QColor(100,100,100,50));

  ui_State.widget_CtrlMode      ->addUnit("自动");
  ui_State.widget_CtrlMode      ->addUnit("手动");

  ui_Ctrl.widget_isPerceptive  ->addUnit("否");
  ui_Ctrl.widget_isPerceptive  ->addUnit("是");

  ui_Ctrl.widget_keepWalking   ->addUnit("否");
  ui_Ctrl.widget_keepWalking   ->addUnit("是");

  ui_Ctrl.widget_lockedTerrain ->addUnit("否");
  ui_Ctrl.widget_lockedTerrain ->addUnit("是");

  ui_Ctrl.widget_TakingControl ->addUnit("否");
  ui_Ctrl.widget_TakingControl ->addUnit("是");
  ui_Ctrl.widget_TakingControl ->setSelectdColor(QColor(220, 120, 120));
  ui_Ctrl.widget_TakingControl ->setBackgroundColor(QColor(100,100,100,50));

  ui_State.widget_isSwitching   ->addUnit("否");
  ui_State.widget_isSwitching   ->addUnit("是");

  ui_State.widget_isStance      ->addUnit("否");
  ui_State.widget_isStance      ->addUnit("是");

  ui_State.widget_CtrlMode     ->setBackgroundColor(QColor(100,100,100,50));
  ui_Ctrl.widget_isPerceptive  ->setBackgroundColor(QColor(100,100,100,50));
  ui_Ctrl.widget_keepWalking   ->setBackgroundColor(QColor(100,100,100,50));
  ui_Ctrl.widget_lockedTerrain ->setBackgroundColor(QColor(100,100,100,50));
  ui_State.widget_isSwitching   ->setBackgroundColor(QColor(100,100,100,50));
  ui_State.widget_isStance      ->setBackgroundColor(QColor(100,100,100,50));

  ui_State.widget_CtrlMode     ->setSelectdColor(QColor(50, 180, 120));
  ui_State.widget_isStance      ->setSelectdColor(QColor(50, 180, 120));
  ui_State.widget_isSwitching   ->setSelectdColor(QColor(50, 180, 120));


  for(const QString &name : alarmStatusNameList){
    ui_State.widget_alarmStatus->addUnit(name);
  }
  ui_State.widget_alarmStatus->setSelectionMode(QWWindowButton::SelectionMode::MultiSelection);
  ui_State.widget_alarmStatus->setMinimumWidth(ui_State.widget_alarmStatus->getUnitSize() * 120);
  ui_State.widget_alarmStatus->setAllowMouseClicked(true);
  ui_State.widget_alarmStatus->setBackgroundColor(QColor(100,100,100,50));
  ui_State.widget_alarmStatus->setSelectdColor(QColor(220, 100, 100));
}

void SevnceRobot::setupSignalConnection() {
  auto& ui_State = sevnceWidget->ui_SevnceRobot_State;
  auto& ui_Ctrl = sevnceWidget->ui_SevnceRobot_Ctrl;

  QObject::connect(ui_Ctrl.toggle,          &QWSwitcher::toggled, ui_Ctrl.widget,         &QWidget::setVisible);
  QObject::connect(ui_State.toggle,         &QWSwitcher::toggled, ui_State.widget,         &QWidget::setVisible);

  QObject::connect(ui_Ctrl.widget_robotMode, &QWWindowButton::selectUnitIndexChanged,
        [this,&ui_Ctrl](unsigned int index) {
          switchRobotMode(ui_Ctrl.widget_robotMode->getUnitName(index));
        });

  QObject::connect(ui_Ctrl.widget_walkGait, &QWWindowButton::selectUnitIndexChanged, [this,&ui_Ctrl](unsigned int index) {
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
        NotifyType::Info, QString("Command"),
        QString("Clicked: %1 %2 >> ")
                .arg(index)
                .arg(ui_Ctrl.widget_walkGait->getUnitName(index)) +
            QString("WalkGait to: %1").arg((int)sevnceData->cmd.walkGait));
  });

  QObject::connect(ui_Ctrl.widget_walkMode, &QWWindowButton::selectUnitIndexChanged, [this,&ui_Ctrl](unsigned int index) {
    switch (index) {
    case 0:
      sevnceData->cmd.walkMode = sevnce::high::WalkMode::Velocity;
      break;
    case 1:
      sevnceData->cmd.walkMode = sevnce::high::WalkMode::Goal;
      break;
    }
    publishNotify(
        NotifyType::Info, QString("Command"),
        QString("Clicked: %1 %2 >> ")
                .arg(index)
                .arg(ui_Ctrl.widget_walkMode->getUnitName(index)) +
            QString("WalkMode to: %1").arg((int)sevnceData->cmd.walkMode));
  });

  QObject::connect(ui_Ctrl.widget_isPerceptive, &QWWindowButton::selectUnitIndexChanged, [this](unsigned int index) {
    sevnceData->cmd.isPerceptive = index; 
  });
  QObject::connect(ui_Ctrl.widget_keepWalking,  &QWWindowButton::selectUnitIndexChanged, [this](unsigned int index) {
    sevnceData->cmd.keepWalking = index; 
  });
  QObject::connect(ui_Ctrl.widget_lockedTerrain,&QWWindowButton::selectUnitIndexChanged, [this](unsigned int index) {
    sevnceData->cmd.lockedTerrain = index; 
  });

  QObject::connect(ui_Ctrl.widget_TakingControl,&QWWindowButton::selectUnitIndexChanged, [this](unsigned int index) {
    QByteArray data;
    data.append(index);
    writeData(0x02, 0x03, data);  // appAuthority

    if(index){
      timer_sendCmd.start();
    } else {
      timer_sendCmd.stop();
    }
  });

}

void SevnceRobot::switchRobotMode(const QString &str) {
    if(str == "Stop") {   

      auto& ui_Ctrl = sevnceWidget->ui_SevnceRobot_Ctrl;
      int result = displayMessageDialog("This could be dangerous !", "Are you sure you want to stop robot?");
      if (result == QDialog::Rejected) {
        auto& state__ = sevnceData->state;
        QString curName = gotRobotMode(state__.robotMode);
        ui_Ctrl.widget_robotMode->setSelectUnit(curName);
        return;
      }

      sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::Stop;    
      }
    else if(str == "Prone"){  sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::Prone;    }
    else if(str == "Stand"){   sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::Stand;   }
    else if(str == "Pose") {   sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::Pose;    }
    else if(str == "Walk") {   sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::Walk;    }
    else if(str == "RL1")  {   sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::WalkRL1; }
    else if(str == "RL2")  {   sevnceData->cmd.switchCmd = sevnce::high::SwitchCmd::WalkRL2; }

    publishNotify(NotifyType::Info, tr("Send to the robot"),
    tr("robot to: %1 %2").arg((int)sevnceData->cmd.switchCmd).arg(str));
  }

void SevnceRobot::flushConfiguration(){

}

void SevnceRobot::commStatusChanged(bool status) {
  if(status){
    // sevnceData->state.alarmStatus = -1;
  } else {
    sevnceData->state.alarmStatus = -1;
  }
}


void SevnceRobot::recordData(){
  if(dataRecorder_ == nullptr)
    return;
  if(!record_){
    return;
  }

  const auto & odom = observations_->odom;
  const auto & imu = observations_->imu;
  const auto & sys = observations_->system;
  const auto & bat = observations_->battery;
  const auto & act = observations_->actuator;

  auto& quat_buffer = imu.quat;

  QVector<scalar_t> data;
  data 
  << odom.position[0] << odom.position[1] << odom.position[2]
  << odom.velocity[0] << odom.velocity[1] << odom.velocity[2]

  << quat_buffer[0] << quat_buffer[1] << quat_buffer[2] << quat_buffer[3]
  << imu.eulerAngles[0] << imu.eulerAngles[1] << imu.eulerAngles[2]
  << imu.acceleration[0] << imu.acceleration[1] << imu.acceleration[2]
  << imu.angularVelocity[0] << imu.angularVelocity[1] << imu.angularVelocity[2]
  << imu.angularAcceleration[0] << imu.angularAcceleration[1] << imu.angularAcceleration[2]

  << (int)sys.status << (int)sys.cpuCoreMaxTemp << (int)sys.cpuPackageTemp << (int)sys.cpuUsage << (int)sys.memoryUsage << (int)sys.diskUsage

  << (int)bat.status << (int)bat.soc << bat.current << bat.voltage << bat.cycle << bat.temp;


  for (size_t i = 0; i < numberOfActuator_; i++){
    data 
    << act[i].state
    << act[i].pos
    << act[i].vel
    << act[i].torque
    << act[i].current
    << act[i].voltage
    << act[i].power
    << act[i].temperature
    << act[i].driverTemperature;
  }

  dataRecorder_->submitRecord(data);
}


void SevnceRobot::setEnabledRecord(bool enabled){
  record_ = enabled;
  if(dataRecorder_ == nullptr){
    dataRecorder_ = new robot::AsyncDataRecorder(getRecordFilePath(), this);
    QStringList col;
    col 
    // odom
    << "odom_position_x" << "odom_position_y" << "odom_position_z"
    << "odom_velocity_x" << "odom_velocity_y" << "odom_velocity_z"

    // imu
    << "imu_quat_w" << "imu_quat_x" << "imu_quat_y" << "imu_quat_z"
    << "imu_euler_x" << "imu_euler_y" << "imu_euler_z"
    << "imu_linear_acc_x" << "imu_linear_acc_y" << "imu_linear_acc_z"
    << "imu_angular_vel_x" << "imu_angular_vel_y" << "imu_angular_vel_z"
    << "imu_angular_acc_x" << "imu_angular_acc_y" << "imu_angular_acc_z"

    // system
    << "sys_status" << "sys_cpu_CoreMaxTemp" << "sys_cpuPackageTemp" << "sys_cpuUsage" << "sys_memoryUsage" << "sys_diskUsage"

    // battery
    << "bms_status" << "bms_soc" << "bms_current" << "bms_voltage" << "bms_cycle" << "bms_temp";

    for (size_t i = 0; i < numberOfActuator_; i++){
      col 
      << QString("actuator%1_state").arg(i) 
      << QString("actuator%1_pos").arg(i) 
      << QString("actuator%1_vel").arg(i) 
      << QString("actuator%1_tau").arg(i) 
      << QString("actuator%1_current").arg(i) 
      << QString("actuator%1_voltage").arg(i) 
      << QString("actuator%1_power").arg(i)
      << QString("actuator%1_temp").arg(i)
      << QString("actuator%1_driverTemp").arg(i);
    }
    
    dataRecorder_->init(1000,col);
  }
}

}


extern "C" {
RobotBase *createRobot(QObject* parent) { return new sevnce::SevnceRobot(parent); }

void destroyRobot(RobotBase *robot) { delete robot; }
}