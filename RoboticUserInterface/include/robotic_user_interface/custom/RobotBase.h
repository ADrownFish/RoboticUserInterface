#pragma once

#include "../../robotic_user_interface/core/ConfigManager.h"
#include "../../robotic_user_interface/core/AsyncDataRecorder.h"
#include "../../robotic_user_interface/core/DataAllocator.h"
#include "../libraries/qt_gcw/QSnackbarManager.h"

#include "robotic_user_interface/core/DataPacketSolver.h"
#include "robotic_user_interface/core/DataSource.h"

#include <QString>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QPointer>
#include <QDialog>
#include <QMutex>

class RobotBase : public QObject{
  Q_OBJECT
public:
  using Solver = robot::DataPacketSolver<10240,50>;
  using Data = Solver::Data;

public:
  RobotBase(QObject *parent = nullptr) : QObject(parent) {}

  virtual ~RobotBase(){}

  /****************  Called from outside  ****************/
  virtual void init(int numberOfActuator, int numberOfEndEffector);
  
  void setTopWidget(QWidget *topWidget);

  void setDataAllocator(const QPointer<DataAllocator>& p);

  const std::shared_ptr<Configuration> configuration() const;

  const std::shared_ptr<ObservationsBase>& observations() const;

  const std::shared_ptr<CommandBase>& command() const;

  const std::shared_ptr<DataSource>& dataSource() const;

  void saveConfiguration() const;

  void readConfiguration() const;

  /****************  virtual  ****************/
  virtual QList<QWidget *> createCustomInfoWidgets() = 0;

  virtual QWidget *createCustomOperationWidget() = 0;

  virtual QStringList getActuators() = 0;

  virtual QString getPluginName() = 0;

  virtual void flushConfiguration() = 0;

  virtual void setEnabledRecord(bool enabled) = 0;
  
  virtual void commStatusChanged(bool enable) = 0;

protected:
  /****************  Called by the parent class  ****************/  
  /****************  virtual  ****************/
  virtual void displayData() = 0;

  virtual void catchData() = 0;

  virtual void recordData() = 0;

  virtual void readyRead() = 0;

  const QString& getRecordFilePath();

  QDialog::DialogCode displayMessageDialog(const QString& title, const QString text);

  /****************  Called by subclasses  ****************/
  void updateDataSource(scalar_t time);

  void writeData(const QByteArray& data);

  void readData(QByteArray& data);

signals:
  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

private: signals:
  void dataReaches();

protected:
  bool record_ = false;

  int numberOfActuator_ = 12;
  int numberOfEndEffector_ = 4;

  Solver solver_;

  std::shared_ptr<ConfigManager>     configManager_;
  std::shared_ptr<DataSource>        dataSource_;
  std::shared_ptr<Configuration>     config_;
  std::shared_ptr<ObservationsBase>  observations_;
  std::shared_ptr<CommandBase>       command_;
  
  QPointer<robot::AsyncDataRecorder> dataRecorder_ = nullptr;
  QPointer<DataAllocator>                   dataAllocator_ = nullptr;

private:
  QByteArray recvbuffer_;
  QMutex recvMutex_; 

  // use for notify
  QWidget* topWidget_;

  // configuration
  QString configFilePath;
  QString recordFilePath;

  // flush ui
  QTimer timer_flush;
};
