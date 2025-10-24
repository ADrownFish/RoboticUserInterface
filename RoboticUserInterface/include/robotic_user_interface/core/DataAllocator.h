#pragma once

#include "Types.h"

#include <QByteArray>
#include <QPointer>

class Communicator;

class DataAllocator :public QObject{
  Q_OBJECT

public:
  DataAllocator(QObject *parent = nullptr);
  
  ~DataAllocator();
  
  void setCommPtr(QPointer<Communicator> p);

  void setConfiguration(std::shared_ptr<Configuration> config);

  bool write(CommunicationConfiguration::CommProtocol protocol, const QByteArray& buffer);

  bool read(CommunicationConfiguration::CommProtocol protocol, DataPktBufferTimePtrVec& vec);

  void init();

signals:
  //void readyRead();

private:
  QPointer<Communicator> comm_;
  std::shared_ptr<Configuration> config_;

};