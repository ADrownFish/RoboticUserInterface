#include "robotic_user_interface/custom/CustomRobot.h"

#include "qwool/qwwindowwidget.h"

#include <QDebug>

#include <stdint.h>
#include <cstdint> 
#include <string>
#include <iostream>
#include <iomanip> 
#include <sstream> 

namespace robot{

class PrivateRobot {
public:
  struct {

  }cmd;
  struct {

  }state;


public:
  PrivateRobot() {

  }


};

CustomRobot::CustomRobot(QObject *parent)
: RobotBase(parent) {

  solver_.setDeviceID(0x01);
  solver_.setEnableFilter(false);
  
  p_ = std::make_unique<PrivateRobot>();

  QObject::connect(&timer_sendCmd, &QTimer::timeout, this, &CustomRobot::writeData);
  timer_sendCmd.start(1000/50);
}
CustomRobot::~CustomRobot(){

}

QStringList CustomRobot::getActuators() { 
  QStringList list;
  list 
    << "Joint 0" << "Joint 1" << "Joint 2"
    << "Joint 3" << "Joint 4" << "Joint 5"
    << "Joint 6" << "Joint 7" << "Joint 8"
    << "Joint 9";

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

void CustomRobot::writeData() {

  // send
  copyFromCommands();

  Data cmdData;
  cmdData.appendData(p_->cmd);
  cmdData.head.dstID = 2;
  cmdData.head.DataID = 1;
  const Solver::ByteArrayPointer bap = solver_.makeDataPacket(cmdData);

  readyWrite(QByteArray((char *)bap.data, bap.size));
}

void CustomRobot::readyRead(const QByteArray &data) {

  // receive
  solver_.pushBytes((uint8_t *)data.data(), data.size());
  while (solver_.getAvailableSize()) {
    auto data = solver_.getFirstDataPacket();

    processData(data);
  }
}

void CustomRobot::init(int numberOfActuator, int numberOfEndEffector){
  RobotBase::init(numberOfActuator, numberOfEndEffector);

  setupWidgetsControls();
  setupSignalConnection();

}

void CustomRobot::processData(const Data& data){
  //switch (data.head.srcID)
  //{
  //case 0x02:{
  //  if(data.head.DataID == 0x01){
  //    if(data.length == sizeof(high::HighState)){
  //      memcpy(&sevnceData->state, data.data, data.length);
  //      copyToObservations();
  //      recordDataOnce();
  //    } else {
  //      qDebug() << "[CustomRobot] Received HighState data length is not match";
  //    }
  //  }
  //} break;
  //
  //default:
  //  break;
  //}
}

void CustomRobot::copyToObservations(){
  //auto &s = p_->state;
  //// odom
  //observations_->odom.position = {s.pose.x, s.pose.y, s.pose.z};
  //observations_->odom.velocity = {s.velocity.xVel, s.velocity.yVel, s.velocity.zVel};
  //
  //// imu
  //observations_->imu.quat = s.imu.quat;
  //observations_->imu.acceleration = {s.imu.linearAcc[0], s.imu.linearAcc[1], s.imu.linearAcc[2]};

  //observations_->imu.eulerAngles = {s.imu.rpy[0], s.imu.rpy[1], s.imu.rpy[2]};
  //observations_->imu.angularVelocity = {s.velocity.rollVel, s.velocity.pitchVel, s.velocity.yawVel};
  //observations_->imu.angularAcceleration = {0,0,0};

  //// system
  //observations_->system.cpuCoreMaxTemp = s.sys.cpuCoreMaxTemp;
  //observations_->system.cpuPackageTemp = s.sys.cpuPackageTemp;
  //observations_->system.cpuUsage       = s.sys.cpuUsage;
  //observations_->system.memoryUsage    = s.sys.memoryUsage;
  //observations_->system.diskUsage      = s.sys.diskUsage;
  //observations_->system.status         = s.sys.alarmStatus;
  //observations_->system.bodyTemp       = s.sys.bodyTemp;
  //observations_->system.bodyHumidity   = s.sys.bodyHumidity;

  //// bms
  //observations_->battery.voltage = s.bms.voltage;
  //observations_->battery.current = s.bms.current;
  //observations_->battery.soc     = s.bms.soc;
  //observations_->battery.cycle   = s.bms.cycle;
  //observations_->battery.temp    = s.bms.temp;
  //observations_->battery.status  = s.bms.status;

  //for (size_t i = 0; i < numberOfActuator_; i++)  {
  //  auto &m = observations_->actuator;
  //  m[i].pos = s.joints[i].q;
  //  m[i].vel = s.joints[i].dq;
  //  m[i].torque = s.joints[i].tau;
  //  m[i].temperature = s.joints[i].motorTemp;
  //  m[i].current = s.joints[i].motorCurrent;
  //  m[i].voltage = s.joints[i].motorVoltage;
  //  m[i].driverTemperature = s.joints[i].driverTemp;
  //  m[i].state = s.joints[i].motorFault;
  //  m[i].power = s.joints[i].motorCurrent * s.joints[i].motorVoltage;
  //}
}

void CustomRobot::copyFromCommands(){
  //auto &c = p_->cmd;
  //c.velocity.xVel = command_->input.velocity[0];
  //c.velocity.yVel = command_->input.velocity[1];
  //c.velocity.yawVel = command_->input.angularVelocity[2];

  //c.goal.x = command_->input.position[0];
  //c.goal.y = command_->input.position[1];
  //c.goal.yaw = command_->input.eulerAngles[2];
}

QWidget *CustomRobot::createCustomOperationWidget(){
  return nullptr;
}

QList<QWidget *> CustomRobot::createCustomInfoWidgets(){
  return {};
}

void CustomRobot::catchData(){
  
}

void CustomRobot::displayData(){

}

void CustomRobot::setupWidgetsControls(){

}

void CustomRobot::setupSignalConnection() {

}

void CustomRobot::flushConfiguration(){

}

void CustomRobot::networkStatusChanged(bool status) {

}
}