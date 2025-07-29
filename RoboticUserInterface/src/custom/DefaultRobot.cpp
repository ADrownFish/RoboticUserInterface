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

  QObject::connect(&timer_sendCmd, &QTimer::timeout, this, &DefaultRobot::writeData);
  timer_sendCmd.start(1000/50);
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
    << "Joint 18" << "Joint 19";

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
  QByteArray data;
  RobotBase::readData(data);

  solver_.pushBytes((uint8_t *)data.data(), data.size());
  while (solver_.getAvailableSize()) {
    auto data = solver_.getFirstDataPacket();
    unpackData(data);
  }
}

void DefaultRobot::init(int numberOfActuator, int numberOfEndEffector){
  RobotBase::init(numberOfActuator, numberOfEndEffector);

  setupWidgetsControls();
  setupSignalConnection();

}

void DefaultRobot::unpackData(const Data &data) {
  uint32_t id = data.head.DataID;
  uint32_t length = data.length;
  const uint8_t *rawData = data.data;

  scalar_t timestamp =  dataSource_->time();

  switch (id) {
  case ODOM: {
    // 解析 Odom (position + velocity, 各 3 个 float)
    if (length != 6 * sizeof(float)) {
      return;
    }
    const float *ptr = reinterpret_cast<const float *>(rawData);
    observations_->odom.position = {ptr[0], ptr[1], ptr[2]};
    observations_->odom.velocity = {ptr[3], ptr[4], ptr[5]};

    if(!std::isnan(observations_->imu.timestamp))
      timestamp = observations_->imu.timestamp;
    break;
  }

  case BATTERY: {
    // 解析 Battery (int32_t cycle, int32_t status, 4 个 float)
    if (length != 2 * sizeof(int32_t) + 4 * sizeof(float)) {
      return;
    }
    const int32_t *iptr = reinterpret_cast<const int32_t *>(rawData);
    const float *dptr = reinterpret_cast<const float *>(iptr + 2);
    observations_->battery.cycle = iptr[0];
    observations_->battery.status = iptr[1];
    observations_->battery.soc = dptr[0];
    observations_->battery.temp = dptr[1];
    observations_->battery.current = dptr[2];
    observations_->battery.voltage = dptr[3];

    if(!std::isnan(observations_->imu.timestamp))
      timestamp = observations_->imu.timestamp;
    break;
  }

  case SYSTEM: {
    // 解析 System (int32_t status, 5 个 float)
    if (length != sizeof(int32_t) + 5 * sizeof(float)) {
      return;
    }
    const int32_t *iptr = reinterpret_cast<const int32_t *>(rawData);
    const float *dptr = reinterpret_cast<const float *>(iptr + 1);
    observations_->system.status = iptr[0];
    observations_->system.cpuUsage = dptr[0];
    observations_->system.memoryUsage = dptr[1];
    observations_->system.diskUsage = dptr[2];
    observations_->system.cpuCoreMaxTemp = dptr[3];
    observations_->system.cpuPackageTemp = dptr[4];

    if(!std::isnan(observations_->imu.timestamp))
      timestamp = observations_->imu.timestamp;
    break;
  }

  case SENSOR: {
    // 解析 Sensor (2 个 float: temp, humidity)
    if (length != 2 * sizeof(float)) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    observations_->sensor.temp = dptr[0];
    observations_->sensor.humidity = dptr[1];

    if(!std::isnan(observations_->imu.timestamp))
      timestamp = observations_->imu.timestamp;
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

    if(!std::isnan(observations_->imu.timestamp))
      timestamp = observations_->imu.timestamp;
    break;
  }

  case IMU: {
    // 解析完整 IMU 数据 (timestamp + quat + euler + acc + angularVel +
    // angularAcc + mag)
    if (length != (1 + 4 + 3 * 4) * sizeof(float)) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    observations_->imu.timestamp = dptr[0];
    observations_->imu.quat = {dptr[1], dptr[2], dptr[3], dptr[4]};
    observations_->imu.angularVelocity = {dptr[5], dptr[6], dptr[7]};
    observations_->imu.acceleration = {dptr[8], dptr[9], dptr[10]};
    observations_->imu.Magnetometer = {dptr[11], dptr[12], dptr[13]};

    observations_->imu.eulerAngles = {dptr[14], dptr[15], dptr[16]};

    if(!std::isnan(observations_->imu.timestamp))
      timestamp = observations_->imu.timestamp;
    break;
  }

  case IMU_QUAT: {
    // 仅解析四元数 (4 个 float)
    if (length != 4 * sizeof(float)) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    observations_->imu.quat = {dptr[0], dptr[1], dptr[2], dptr[3]};

    if(!std::isnan(observations_->imu.timestamp))
      timestamp = observations_->imu.timestamp;
    break;
  }

  case IMU_RAW: {
    // 解析 IMU 原始数据 (timestamp + quat + acc + mag + angularVel)
    if (length != (1 + 4 + 3 * 3) * sizeof(float)) {
      return;
    }
    const float *dptr = reinterpret_cast<const float *>(rawData);
    observations_->imu.timestamp = dptr[0];
    observations_->imu.quat = {dptr[1], dptr[2], dptr[3], dptr[4]};
    observations_->imu.angularVelocity = {dptr[5], dptr[6], dptr[7]};
    observations_->imu.acceleration = {dptr[8], dptr[9], dptr[10]};
    observations_->imu.Magnetometer = {dptr[11], dptr[12], dptr[13]};

    if(!std::isnan(observations_->imu.timestamp))
      timestamp = observations_->imu.timestamp;
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
      act.voltage = fptr[3];
      act.current = fptr[4];
      act.power = fptr[5];
      act.temperature = fptr[6];
      act.driverTemperature = fptr[7];

      if(!std::isnan(observations_->imu.timestamp))
        timestamp = observations_->imu.timestamp;
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

void DefaultRobot::setEnabledRecord(bool enabled) {


}
void DefaultRobot::commStatusChanged(bool status) {


}
}
