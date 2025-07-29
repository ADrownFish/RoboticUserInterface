#include "robotic_user_interface/custom/CustomRobot.h"

#include "qwool/qwwindowwidget.h"

#include <QDebug>

#include <stdint.h>
#include <cstdint> 
#include <string>
#include <iostream>
#include <iomanip> 
#include <sstream> 

#include "robotic_user_interface/custom/CustomRobot.h"

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
}
QString CustomRobot::getPluginName() { return QString("Custom"); };

// std::string formatByteArray(const uint8_t *array, size_t size) {
//   std::stringstream ss;
//   ss << std::hex << std::setw(2) << std::setfill('0');
//   for (size_t i = 0; i < size; ++i) {
//     ss << std::setw(2) << static_cast<int>(array[i]) << " ";
//   }
//   ss << std::dec;
//   return ss.str();
// }

void CustomRobot::writeData() {


  // RobotBase::writeData(QByteArray((char *)bap.data, bap.size));
}

void CustomRobot::readyRead() {

  // receive
  QByteArray data;
  RobotBase::readData(data);

  solver_.pushBytes((uint8_t *)data.data(), data.size());
  while (solver_.getAvailableSize()) {
    auto data = solver_.getFirstDataPacket();

    unpackData(data);
  }
}

void CustomRobot::init(int numberOfActuator, int numberOfEndEffector){
  RobotBase::init(numberOfActuator, numberOfEndEffector);

  setupWidgetsControls();
  setupSignalConnection();

}

void CustomRobot::unpackData(const Data& data){

}

void CustomRobot::copyToObservations(){
  
}

void CustomRobot::copyFromCommands(){

}

QWidget *CustomRobot::createCustomOperationWidget(){
  return nullptr;
}

QList<QWidget *> CustomRobot::createCustomInfoWidgets(){
  return {};
}

void CustomRobot::catchData(){
  
}

void CustomRobot::recordData() {}

void CustomRobot::displayData(){

}

void CustomRobot::setupWidgetsControls(){

}

void CustomRobot::setupSignalConnection() {

}

void CustomRobot::flushConfiguration(){

}

void CustomRobot::setEnabledRecord(bool enabled) {


}
void CustomRobot::commStatusChanged(bool status) {


}
}