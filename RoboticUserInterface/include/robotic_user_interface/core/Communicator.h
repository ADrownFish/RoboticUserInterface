#pragma once

#include <atomic>

#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>
#include <QSerialPort>

#include <chrono>

#include "robotic_user_interface/core/RingBuffer.h"
#include "robotic_user_interface/core/Types.h"
#include "robotic_user_interface/core/BluetoothManager.h"
#include "../libraries/qt_gcw/QSnackbarManager.h"

class Communicator : public QObject{
  Q_OBJECT
public:
  using CCDataBits = CommunicationConfiguration::DataBits;
  using CCStopBits = CommunicationConfiguration::StopBits;
  using CCFlowControl = CommunicationConfiguration::FlowControl;
  using CCParity = CommunicationConfiguration::Parity;
  using CCType = CommunicationConfiguration::CommType;
  
public:
  Communicator(QObject *parent);
  
  ~Communicator();

  void init();

  void setup(const CommunicationConfiguration& comm);

  void shutdown();

  bool isOpen();

  void open();

  void close();

  void write(const QByteArray& buffer);

  void read(QByteArray& buffer);

  uint64_t getReadBytesLength();

  uint64_t getWriteBytesLength();

private:
  void setupSignalConnection();

  void start();

  void tcpNewConnection();

  void readyReadUdp();

  void readyReadTcp();

  void readyReadServerTcp(QTcpSocket* socket);

  void readyReadSerial();

  void sendSignal_readyread(const QByteArray& buffer);

  void readySendSocket();
  
  void close_threadImpl();

  void open_threadImpl();

signals:

  void CommStatusChanged(bool);

  // for private
  void closed();    
  
  void publishFocusStatus(const QString& status);

  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

  void readyRead();

  void readySend();

  void requestOpen();

  void requestClose();

public:
  QSerialPort::Parity toQtParity(CCParity parity);

  QSerialPort::FlowControl toQtFlowControl(CCFlowControl flow);

  QSerialPort::StopBits toQtStopBits(CCStopBits stopBits);

  QSerialPort::DataBits toQtDataBits(CCDataBits dataBits);

  QString toString(CCType type);

private:
  QThread *thread_ = nullptr;

  CommunicationConfiguration config_;

  QUdpSocket *socket_udp_ = nullptr;
  QUdpSocket* socket_udp_tx_ = nullptr;
  QTcpSocket *socket_tcp_ = nullptr;
  QTcpServer *server_tcp_ = nullptr;
  QSerialPort *socket_serial = nullptr;
  BluetoothConnectionManager* bluetooth_manager_ = nullptr;

  std::atomic_bool open_udp_ = false;
  std::atomic_bool open_tcp_ = false;
  std::atomic_bool open_tcp_server_ = false;
  std::atomic_bool open_serial_ = false;
  std::atomic_bool open_bluetooth_ = false;

  std::atomic_bool dataTaken_ = true;

  static constexpr int bufferSize = 1 << 24;
  RingBuffer<uint8_t, bufferSize> rx_buffer;
  RingBuffer<uint8_t, bufferSize> tx_buffer;

  uint64_t writeBytesLength = 0;
  uint64_t readBytesLength = 0;

  int64_t server_tcp_client_count = 0;
};
