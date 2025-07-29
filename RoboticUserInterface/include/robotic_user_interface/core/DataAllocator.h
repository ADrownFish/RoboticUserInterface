#pragma once

#include "Types.h"
#include "Communicator.h"

#include <QByteArray>
#include <QPointer>

class DataAllocator :public QObject{
  Q_OBJECT

public:
  DataAllocator(QObject *parent = nullptr);
  
  ~DataAllocator();
  
  void setCommPtr(QPointer<Communicator> p);

  void setConfiguration(std::shared_ptr<Configuration> config);

  void write(CommunicationConfiguration::CommProtocol protocol, const QByteArray& buffer);

  void read(CommunicationConfiguration::CommProtocol protocol, QByteArray& buffer);

  void init();

signals:
  void readyRead();

private:
  QPointer<Communicator> comm_;
  std::shared_ptr<Configuration> config_;

};