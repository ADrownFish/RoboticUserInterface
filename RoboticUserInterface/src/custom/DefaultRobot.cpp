#include "robotic_user_interface/custom/DefaultRobot.h"

#include "qwool/qwwindowwidget.h"

#include <QDebug>

#include <stdint.h>
#include <cstdint> 
#include <string>
#include <iostream>
#include <iomanip> 
#include <sstream> 

#include "robotic_user_interface/custom/DefaultRobot.h"

namespace robot{

DefaultRobot::DefaultRobot(QObject *parent)
: RobotBase(parent) {

  solver_.setDeviceID(0xFF);
  solver_.setEnableFilter(false);
}
DefaultRobot::~DefaultRobot(){

}

QStringList DefaultRobot::getActuators() { 
  QStringList list;
  list 
    << "Joint 0" << "Joint 1" << "Joint 2"
    << "Joint 3" << "Joint 4" << "Joint 5"
    << "Joint 6" << "Joint 7" << "Joint 8"
    << "Joint 9" << "Joint 10" << "Joint 11"
    << "Joint 12" << "Joint 13" << "Joint 14"
    << "Joint 15" << "Joint 16" << "Joint 17"
    << "Joint 18" << "Joint 19" << "Joint 20"
    << "Joint 21" << "Joint 22" << "Joint 23"
    << "Joint 24" << "Joint 25" << "Joint 26"
    << "Joint 27" << "Joint 28" << "Joint 29";

  return list; 
}
QString DefaultRobot::getPluginName() { return QString("Default"); };

// std::string formatByteArray(const uint8_t *array, size_t size) {
//   std::stringstream ss;
//   ss << std::hex << std::setw(2) << std::setfill('0');
//   for (size_t i = 0; i < size; ++i) {
//     ss << std::setw(2) << static_cast<int>(array[i]) << " ";
//   }
//   ss << std::dec;
//   return ss.str();
// }

void DefaultRobot::writeData() {


  // RobotBase::writeData(QByteArray((char *)bap.data, bap.size));
}

void DefaultRobot::readyRead() {

  // receive
  DataPktBufferTimePtrVec vec;
  bool ret = RobotBase::readData(vec);
  if (!ret) {
    return;
  }

  for (auto& it : vec) {
    solver_.pushBytes(it->buffer.data(), it->buffer.size());
    scalar_t& time = it->timestamp;
    while (solver_.getAvailableSize()) {
      auto data = solver_.getFirstDataPacket();
      unpackData(time, data);
    }
  }
}

void DefaultRobot::init(){
  int numberOfActuator = 30;
  int numberOfEndEffector = 4;
  
  RobotBase::init(numberOfActuator, numberOfEndEffector);

  setupWidgetsControls();
  setupSignalConnection();
}

void DefaultRobot::unpackData(scalar_t time, const Data &data) {
  uint32_t id = data.head.DataID;
  uint32_t length = data.length;
  const uint8_t *rawData = data.data;

  scalar_t timestamp = time - dataSource_->startTime();
  scalar_t packTimestamp = NAN;

  switch (id) {
  case ODOM: {
    // 解析 Odom
    if (length != 7 * sizeof(float)) {
      return;
    }
    const float *ptr = reinterpret_cast<const float *>(rawData);
    packTimestamp = ptr[0];
    observations_->odom.position = {ptr[1], ptr[2], ptr[3]};
    observations_->odom.velocity = {ptr[4], ptr[5], ptr[6]};

    if(!std::isnan(packTimestamp))
      timestamp = packTimestamp;
    break;
  }

  case BATTERY: {
    // 解析 Battery
    if (length != 7 * sizeof(float)) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    packTimestamp = dptr[0];
    observations_->battery.cycle = dptr[1];
    observations_->battery.status = dptr[2];
    observations_->battery.soc = dptr[3];
    observations_->battery.temp = dptr[4];
    observations_->battery.current = dptr[5];
    observations_->battery.voltage = dptr[6];

    if(!std::isnan(packTimestamp))
      timestamp = packTimestamp;
    break;
  }

  case SYSTEM: {
    // 解析 System 
    if (length != sizeof(int32_t) + 5 * sizeof(float)) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    packTimestamp = dptr[0];
  
    observations_->system.status = dptr[1];
    observations_->system.cpuUsage = dptr[2];
    observations_->system.memoryUsage = dptr[3];
    observations_->system.diskUsage = dptr[4];
    observations_->system.cpuTemp = dptr[5];
    observations_->system.cpuFerq = dptr[6];

    if(!std::isnan(packTimestamp))
      timestamp = packTimestamp;
    break;
  }

  case SENSOR: {
    // 解析 Sensor
    if (length != 2 * sizeof(float)) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    packTimestamp = dptr[0];
    observations_->sensor.temp = dptr[1];
    observations_->sensor.humidity = dptr[2];

    if(!std::isnan(packTimestamp))
      timestamp = packTimestamp;
    break;
  }

  case END_EFFECTOR: {
    // 解析 EndEffector (position, velocity, force, contact flags)
    const size_t expectedSize =
        3 * 3 * sizeof(float) + // position, velocity, force (各 3 个 float)
        2 * observations_->numberOfEndEffector_ * sizeof(bool); // contact flags
    if (length != expectedSize) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    const bool *bptr = reinterpret_cast<const bool *>(dptr + 9);

    observations_->endEffector.position = {dptr[0], dptr[1], dptr[2]};
    observations_->endEffector.velocity = {dptr[3], dptr[4], dptr[5]};
    observations_->endEffector.force = {dptr[6], dptr[7], dptr[8]};

    for (int i = 0; i < observations_->numberOfEndEffector_; i++) {
      observations_->endEffector.contactFlag_sensor[i] = bptr[i];
      observations_->endEffector.contactFlag_estimate[i] =
          bptr[observations_->numberOfEndEffector_ + i];
    }

    if(!std::isnan(packTimestamp))
      timestamp = packTimestamp;
    break;
  }

  case IMU: {
    // 解析完整 IMU 数据 (timestamp + quat + euler + acc + angularVel +
    // angularAcc + mag)
    if (length !=  17 * sizeof(float)) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    packTimestamp = dptr[0];
    observations_->imu.quat = {dptr[1], dptr[2], dptr[3], dptr[4]};
    observations_->imu.angularVelocity = {dptr[5], dptr[6], dptr[7]};
    observations_->imu.acceleration = {dptr[8], dptr[9], dptr[10]};
    observations_->imu.Magnetometer = {dptr[11], dptr[12], dptr[13]};

    observations_->imu.eulerAngles = {dptr[14], dptr[15], dptr[16]};

    if(!std::isnan(packTimestamp))
      timestamp = packTimestamp;
    break;
  }

  case IMU_QUAT: {
    // 仅解析四元数 (4 个 float)
    if (length != 5 * sizeof(float)) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    packTimestamp = dptr[0];
    observations_->imu.quat = {dptr[1], dptr[2], dptr[3], dptr[4]};

    if(!std::isnan(packTimestamp))
      timestamp = packTimestamp;
    break;
  }

  case IMU_RAW: {
    // 解析 IMU 原始数据 (timestamp + quat + acc + mag + angularVel)
    if (length != 14 * sizeof(float)) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    packTimestamp = dptr[0];
    observations_->imu.quat = {dptr[1], dptr[2], dptr[3], dptr[4]};
    observations_->imu.angularVelocity = {dptr[5], dptr[6], dptr[7]};
    observations_->imu.acceleration = {dptr[8], dptr[9], dptr[10]};
    observations_->imu.Magnetometer = {dptr[11], dptr[12], dptr[13]};

    if(!std::isnan(packTimestamp))
      timestamp = packTimestamp;
    break;
  }

  default: {
    // 处理执行器数据 (0x200~0x250)
    if (id >= ACTUATOR_BASE && id <= ACTUATOR_BASE + 0x50) {
      uint32_t actuatorId = id - ACTUATOR_BASE;
      if (actuatorId >= observations_->actuator.size()) {
        return;
      }
      // 解析 Actuator 数据 (state + 7 个 float)
      if (length != sizeof(uint32_t) + 7 * sizeof(float)) {
        return;
      }
      const uint32_t *uptr = reinterpret_cast<const uint32_t *>(rawData);
      const float *fptr = reinterpret_cast<const float *>(uptr + 1);
      auto &act = observations_->actuator[actuatorId];
      act.state = uptr[0];
      act.pos = fptr[0];
      act.vel = fptr[1];
      act.torque = fptr[2];
      act.power = fptr[5];
      act.temperature = fptr[6];
      act.driverTemperature = fptr[7];

      if(!std::isnan(packTimestamp))
        timestamp = packTimestamp;
    } else {
      // 什么也没有
      return;
    }
    break;
  }
  }

  updateDataSource(timestamp);
}

void DefaultRobot::copyToObservations(){
  
}

void DefaultRobot::copyFromCommands(){

}

QWidget *DefaultRobot::createCustomOperationWidget(){
  return nullptr;
}

QList<QWidget *> DefaultRobot::createCustomInfoWidgets(){
  return {};
}

void DefaultRobot::catchData(){
  
  writeData();
}

void DefaultRobot::recordData() {


}

void DefaultRobot::displayData(){

}

void DefaultRobot::setupWidgetsControls(){

}

void DefaultRobot::setupSignalConnection() {

}

void DefaultRobot::flushConfiguration(){

}

void DefaultRobot::commStatusChanged(bool status) {


}
}
