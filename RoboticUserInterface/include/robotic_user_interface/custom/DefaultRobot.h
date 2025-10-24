#pragma once

#include "RobotBase.h"

namespace robot{

class PrivateRobot;

class DefaultRobot : public RobotBase{
  Q_OBJECT
public:
  enum DataID {
    ODOM         = 10,       // 里程计数据
    BATTERY      = 11,       // 电池数据
    SYSTEM       = 12,       // 系统状态数据
    SENSOR       = 13,       // 环境传感器数据
    END_EFFECTOR = 14,       // 末端执行器数据

    IMU = 100,               // 完整 IMU 数据
    IMU_QUAT = 101,          // 仅 IMU 四元数
    IMU_RAW = 102,           // IMU 原始数据（时间戳 + 传感器数据）
    
    ACTUATOR_BASE = 200      // 200~250 对应电机 0~N
  };

public:
  DefaultRobot(QObject *parent = nullptr);

  ~DefaultRobot() override;

  void init() override;

  /****************  virtual  ****************/
  QList<QWidget *> createCustomInfoWidgets() override;

  QWidget *createCustomOperationWidget() override;

  QStringList getActuators() override;

  QString getPluginName() override;

  void flushConfiguration() override;

  void commStatusChanged(bool enable) override;

protected:
  void displayData() override;

  void catchData() override;

  void readyRead() override;   // received data

private:
  void setupWidgetsControls();

  void setupSignalConnection();

  void unpackData(scalar_t time, const Data& data);
  
  void copyToObservations();
  
  void copyFromCommands();
  
  void writeData();

  void recordData();
};

}
