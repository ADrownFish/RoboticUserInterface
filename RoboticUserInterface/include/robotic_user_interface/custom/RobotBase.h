#pragma once

#include "../../robotic_user_interface/core/ConfigManager.h"
#include "../../robotic_user_interface/core/AsyncDataRecorder.h"
#include "../libraries/qt_gcw/QSnackbarManager.h"

#include "robotic_user_interface/core/DataPacketSolver.h"
#include "robotic_user_interface/core/DataSource.h"

#include <QString>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QPointer>

class RobotBase : public QObject{
  Q_OBJECT
public:
  using Solver = robot::DataPacketSolver<10240,50>;
  using Data = Solver::Data;
  enum DegRad {
    DEG,
    RAD,
  };

public:
  RobotBase(QObject *parent = nullptr) : QObject(parent) {}

  ~RobotBase(){}

  void init(int numberOfActuator, int numberOfEndEffector);

  void setEnabledRecord(bool enabled);

  const std::shared_ptr<Configuration> configuration() const;

  const std::shared_ptr<ObservationsBase>& observations() const;

  const std::shared_ptr<CommandBase>& command() const;

  const std::shared_ptr<DataSource>& dataSource() const;

  void saveConfiguration();

  void readConfiguration();

/****************  virtual  ****************/
public:
  virtual QList<QWidget *> createCustomInfoWidgets() = 0;

  virtual QWidget *createCustomOperationWidget() = 0;

  virtual QStringList getActuators() { return QStringList(); };

  virtual void flushConfiguration() = 0;

  virtual void networkStatusChanged(bool status) {};

protected:
  virtual void displayData() = 0;

  virtual void catchData() = 0;

  virtual void recordDataOnce();

  void appendBaseDataSource();

public slots:
  virtual void readyRead(const QByteArray& data) = 0;   // received data

signals:
  void readyWrite(const QByteArray& data);   // ready command

  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

protected:
  bool record_ = false;

  int numberOfActuator_ = 12;
  int numberOfEndEffector_ = 4;

  Solver solver_;
  QTimer timer_flush;

  std::shared_ptr<ConfigManager> configManager_;
  std::shared_ptr<Configuration> config_;
  std::shared_ptr<ObservationsBase> observations_;
  std::shared_ptr<CommandBase> command_;
  std::shared_ptr<DataSource> dataSource_;
  
  QPointer<robot::AsyncDataRecorder> dataRecorder_ = nullptr;

  QString configFilePath;
  QString recordFilePath;
};
