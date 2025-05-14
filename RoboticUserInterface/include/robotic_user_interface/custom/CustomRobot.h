#pragma once

#include "RobotBase.h"

namespace robot{

class PrivateRobot;

class CustomRobot : public RobotBase{
  Q_OBJECT
public:
  CustomRobot(QObject *parent = nullptr);

  ~CustomRobot();
  // init
  void init(int numberOfActuator, int numberOfEndEffector);

/****************  virtual  ****************/
public:  
  QList<QWidget *> createCustomInfoWidgets() override;

  QWidget *createCustomOperationWidget() override;

  QStringList getActuators() override;
protected:
  void displayData() override;

  void catchData() override;

  void readyRead(const QByteArray& data) override;   // received data

  void flushConfiguration() override;

  void networkStatusChanged(bool status) override;
private:
  void setupWidgetsControls();

  void setupSignalConnection();

  void processData(const Data& data);
  
  void copyToObservations();
  
  void copyFromCommands();
  
  void writeData();

private:
  std::unique_ptr<PrivateRobot> p_;

  QTimer timer_sendCmd;
};

}