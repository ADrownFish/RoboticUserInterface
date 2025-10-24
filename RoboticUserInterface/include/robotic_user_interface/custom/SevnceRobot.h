#pragma once

#include "RobotBase.h"

namespace sevnce {

class SevnceData;
class SevnceWidget;

class SevnceRobot : public RobotBase{
  Q_OBJECT
public:
  explicit SevnceRobot(QObject *parent = nullptr);

  ~SevnceRobot() override;

  void init() override;

  /****************  override  ****************/
  QList<QWidget *> createCustomInfoWidgets() override;

  QWidget *createCustomOperationWidget() override;

  QStringList getActuators() override;

  QString getPluginName() override;

  void flushConfiguration() override;

  void commStatusChanged(bool enable) override;

protected:
  void displayData() override;

  void catchData() override;

  void readyRead() override;

private:
  void setupWidgetsControls();

  void setupSignalConnection();

  void unpackData(scalar_t time, const Data& data);
  
  void copyToObservations(scalar_t time);
  
  void copyFromCommands();

  void switchRobotMode(const QString &str);
  
  void writeData();

  void writeData(uint8_t dstID, uint8_t dataID, const uint8_t *data, size_t size);

  void setEnabledRecord(bool enabled);

  void recordData();

private:
  bool dataUpdate = false;
  bool record_ = false;

  std::unique_ptr<SevnceData> sevnceData;
  std::unique_ptr<SevnceWidget> sevnceWidget;
};

}