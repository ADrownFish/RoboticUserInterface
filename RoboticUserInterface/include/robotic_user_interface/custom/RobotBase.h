#pragma once

#include "robotic_user_interface/core/ConfigManager.h"
#include "robotic_user_interface/core/AsyncDataRecorder.h"
#include "robotic_user_interface/core/DataAllocator.h"
#include "robotic_user_interface/core/DataPacketSolver.h"
#include "robotic_user_interface/core/DataSource.h"

#include <QString>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QPointer>
#include <QDialog>
#include <QMutex>
#include <QSharedPointer>

class RobotBase : public QObject{
  Q_OBJECT
public:
  using Solver = robot::DataPacketSolver<10240,50>;
  using Data = Solver::Data;

  enum NotifyType
	{
		Info = 0x00,          // 提示信息，用于提供一般性的信息
		Success,                // 操作成功，表示操作成功完成
		Debug,                  // 调试信息，用于调试目的，仅在调试模式下显示
		Warning,               // 警告信息，表示一般性的警告
		Error,                     // 错误信息，表示操作失败或出现错误
	};

public:
  RobotBase(QObject *parent = nullptr) : QObject(parent) {
    qRegisterMetaType<RobotBase::NotifyType>("RobotBase::NotifyType");
  }

  virtual ~RobotBase(){}

  /****************  Called from outside  ****************/
  virtual void init(int numberOfActuator, int numberOfEndEffector);

  virtual void init(){};
  
  void setTopWidget(QWidget *topWidget);

  void setDataAllocator(const QPointer<DataAllocator>& p);

  const std::shared_ptr<Configuration> configuration() const;

  const std::shared_ptr<ObservationsBase>& observations() const;

  const std::shared_ptr<CommandBase>& command() const;

  const std::shared_ptr<DataSource>& dataSource() const;

  void saveConfiguration() const;

  void readConfiguration() const;

public:
  /****************  virtual  ****************/
  virtual QList<QWidget *> createCustomInfoWidgets() = 0;

  virtual QWidget *createCustomOperationWidget() = 0;

  virtual QStringList getActuators() = 0;

  virtual QString getPluginName() = 0;

  virtual void flushConfiguration() = 0;

  virtual void commStatusChanged(bool enable) = 0;

public:

protected:
  /****************  Called by the parent class  ****************/  
  /****************  virtual  ****************/
  virtual void displayData() = 0;

  virtual void catchData() = 0;

  virtual void readyRead() = 0;

  const QString& getRecordFilePath();

  QDialog::DialogCode displayMessageDialog(const QString& title, const QString text);

  /****************  Called by subclasses  ****************/
  void updateDataSource(scalar_t time);

  bool writeData(const QByteArray& data);

  bool readData(DataPktBufferTimePtrVec& vec);

signals:
  void publishNotify(NotifyType type,const QString &title, const QString& text);

protected:
  int numberOfActuator_ = 12;
  int numberOfEndEffector_ = 4;

  Solver solver_;

  std::shared_ptr<ConfigManager>     configManager_;
  std::shared_ptr<Configuration>     config_;
  std::shared_ptr<ObservationsBase>  observations_;
  std::shared_ptr<CommandBase>       command_;
  std::shared_ptr<DataSource>        dataSource_;
  
  QPointer<robot::AsyncDataRecorder> dataRecorder_  = nullptr;
  QPointer<DataAllocator>            dataAllocator_ = nullptr;
private:
  // use for notify
  QWidget* topWidget_;

  // configuration
  QString configFilePath;
  QString recordFilePath;

  // flush ui
  QTimer timer_flush;
};

// 定义插件创建和销毁的函数类型
typedef RobotBase* (*CreateRobotFunc)(QObject* parent);
typedef void (*DestroyRobotFunc)(RobotBase*);