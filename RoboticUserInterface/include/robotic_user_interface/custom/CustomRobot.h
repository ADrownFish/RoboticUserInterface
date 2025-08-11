#pragma once

#include "RobotBase.h"

namespace robot{

class PrivateRobot;

class CustomRobot : public RobotBase{
  Q_OBJECT
public:
  CustomRobot(QObject *parent = nullptr);

  ~CustomRobot() override;

  void init(int numberOfActuator, int numberOfEndEffector);

  /****************  virtual  ****************/
  QList<QWidget *> createCustomInfoWidgets() override;

  QWidget *createCustomOperationWidget() override;

  QStringList getActuators() override;

  QString getPluginName() override;

  void flushConfiguration() override;

  void setEnabledRecord(bool enabled) override;
  
  void commStatusChanged(bool enable) override;

protected:
  void displayData() override;

  void catchData() override;

  void recordData() override;

  void readyRead() override;   // received data

private:
  void setupWidgetsControls();

  void setupSignalConnection();

  void unpackData(const Data& data);
  
  void copyToObservations();
  
  void copyFromCommands();
  
  void writeData();

private:
  std::unique_ptr<PrivateRobot> p_;

  QTimer timer_sendCmd;
};

}
