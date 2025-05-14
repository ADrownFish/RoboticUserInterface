#pragma once

#include "RobotBase.h"

namespace sevnce{

class SevnceData;
class SevnceWidget;

class SevnceRobot : public RobotBase{
  Q_OBJECT
public:
  SevnceRobot(QObject *parent = nullptr);

  ~SevnceRobot();
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

  void switchRobotMode(const QString &str);
  
  void writeData();

private:
  std::unique_ptr<SevnceData> sevnceData;
  std::unique_ptr<SevnceWidget> sevnceWidget;

  QTimer timer_sendCmd;
};

}