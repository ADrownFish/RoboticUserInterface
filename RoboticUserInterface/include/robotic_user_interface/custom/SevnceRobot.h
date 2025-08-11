#pragma once

#include "RobotBase.h"

namespace sevnce{

class SevnceData;
class SevnceWidget;

class SevnceRobot : public RobotBase{
  Q_OBJECT
public:
  SevnceRobot(QObject *parent = nullptr);

  ~SevnceRobot() override;

  void init(int numberOfActuator, int numberOfEndEffector);

  /****************  override  ****************/
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

  void readyRead() override;

private:
  void setupWidgetsControls();

  void setupSignalConnection();

  void unpackData(const Data& data);
  
  void copyToObservations();
  
  void copyFromCommands();

  void switchRobotMode(const QString &str);
  
  void writeData();

  void writeData(uint8_t dstID, uint8_t dataID, const QByteArray &data);

private:
  std::unique_ptr<SevnceData> sevnceData;
  std::unique_ptr<SevnceWidget> sevnceWidget;

  QTimer timer_sendCmd;
  QTimer timer_app;

  bool dataUpdate = false;
};

}
