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
  Communicator(QObject *parent) : QObject(nullptr) {

    thread_ = new QThread(parent);
    this->moveToThread(thread_);
  }
  ~Communicator(){

  }

  void init(){
    setupSignalConnection();
    thread_->start();
  }

  void setup(const CommunicationConfiguration& comm){
    config_ = comm;
  }

  void shutdown() {
    QEventLoop loop;
    QObject::connect(this, &Communicator::closed, &loop, &QEventLoop::quit);
    requestClose();
    loop.exec();

    QObject::connect(thread_, &QThread::finished, &loop, &QEventLoop::quit);
    thread_->quit();
    loop.exec();
  }

  bool isOpen(){
    switch (config_.commType){
      case CCType::UDP:{
        return open_udp_;
      }break;
      case CCType::TCP:{
        return open_tcp_;
      }break;
      case CCType::SERIAL:{
        return open_serial_;
      }break;
      case CCType::BLUETOOTH:{
        return open_bluetooth_;
      }break;
    }
    return false;
  }

  void open() {
    requestOpen();
  }

  void close() {
    requestClose();
  }

  void write(const QByteArray& buffer){
    if(buffer.isEmpty()){
      return;
    }

    if (isOpen()) {
      int size = buffer.size();
      //writeBytesLength += size;

      writeBytesLength += tx_buffer.WriteBatch((uint8_t*)buffer.data(), size);
      readySend();
    }
  }

  void read(QByteArray& buffer) {
    //qDebug() << "pull";
    
    if(rx_buffer.IsEmpty()){
      return;
    }

    int size = rx_buffer.Size();
    //readBytesLength += size;

    //qDebug() << "pull" << rx_buffer.Size();

    buffer.resize(size);
    readBytesLength += rx_buffer.ReadBatch((uint8_t*)buffer.data(), size);

    dataTaken_ = true;
  }

  uint64_t getReadBytesLength() {
    return readBytesLength;
  }

  uint64_t getWriteBytesLength() {
    return writeBytesLength;
  }

private:
  void setupSignalConnection(){

    QObject::connect(thread_, &QThread::started,  this, &Communicator::start);
  }

  void start() {
    thread_->setPriority(QThread::HighPriority);

    // on obj thread
    socket_udp_ = new QUdpSocket(this);
    socket_udp_tx_ = new QUdpSocket(this);
    socket_tcp_ = new QTcpSocket(this);
    server_tcp_ = new QTcpServer(this);
    socket_serial = new QSerialPort(this);

    QObject::connect(socket_udp_, &QUdpSocket::readyRead, this,   &Communicator::readyReadUdp);
    QObject::connect(socket_udp_, &QUdpSocket::disconnected, [this]() {  open_udp_ = false; });

    QObject::connect(socket_tcp_, &QTcpSocket::readyRead, this,     &Communicator::readyReadTcp);
    QObject::connect(socket_tcp_, &QUdpSocket::disconnected, [this]() {   open_tcp_ = false;  });

    QObject::connect(socket_serial, &QSerialPort::readyRead, this,     &Communicator::readyReadSerial);
    QObject::connect(socket_serial, &QSerialPort::aboutToClose, [this]() {   open_serial_ = false;  });
    QObject::connect(socket_serial, &QSerialPort::errorOccurred, [this](QSerialPort::SerialPortError error) {
      if (error == QSerialPort::NoError)
        return;

      // 避免重复响应：如果已经关闭就不再处理
      if (!socket_serial->isOpen())
        return;

      QString errStr = socket_serial->errorString();
      emit publishNotify(GCW::NotifyType::Warning, "Serial", tr("An error occurred: ") + errStr);

      // 通过 queued 方式关闭，避免在 Qt 信号槽处理中直接 close
      QMetaObject::invokeMethod(socket_serial, "close", Qt::QueuedConnection);
      open_serial_ = false;
    });

    QObject::connect(this,             &Communicator::readySend, this,            &Communicator::readySendSocket);
    QObject::connect(this,             &Communicator::requestOpen, this,        &Communicator::open_threadImpl);
    QObject::connect(this,             &Communicator::requestClose, this,        &Communicator::close_threadImpl);
  }

  void readyReadUdp(){
    // on obj thread
    while (socket_udp_->hasPendingDatagrams()) {
      qint64 size = socket_udp_->pendingDatagramSize();
      if (size <= 0) return;  // 无效数据

      QByteArray buffer;
      buffer.resize(size);
      QHostAddress sender;
      quint16 senderPort;
      socket_udp_->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

      sendSignal_readyread(buffer);
    }
  }
  void readyReadTcp(){
    // on obj thread
    QByteArray buffer = socket_tcp_->readAll();
    sendSignal_readyread(buffer);
  }

  void readyReadSerial(){
    // on obj thread
    QByteArray buffer = socket_serial->readAll();
    sendSignal_readyread(buffer);
  }

  void sendSignal_readyread(const QByteArray& buffer){
      // using namespace std::chrono;

      // auto now = steady_clock::now();
      // auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();

      // if((ms - timestamp) > 100){
      //     timestamp = ms;
      //     readyRead();
      // }
      //qDebug() << "push" << buffer.size();
      rx_buffer.WriteBatch((uint8_t*)buffer.data(), buffer.size());
      

      // If the data has not been taken, no signal is sent. maybe wrong ?
      if(dataTaken_){
        dataTaken_ = false;
        readyRead();
      } else {
      }
  }

  void readySendSocket() {
    // on obj thread
    QByteArray buffer;
    buffer.resize(tx_buffer.Size());
    tx_buffer.ReadBatch((uint8_t *)buffer.data(), buffer.size());

    switch (config_.commType){
      case CCType::UDP:{
        if(open_udp_){
          //socket_udp_tx_->writeDatagram(buffer,QHostAddress(config_.udp.ip),config_.udp.port);
          socket_udp_tx_->write(buffer);
        }
      }break;
      case CCType::TCP:{
        if(open_tcp_){
          socket_tcp_->write(buffer);
        }        
      }break;
      case CCType::SERIAL:{
        if(open_serial_){
          socket_serial->write(buffer);
        }
      }break;
      case CCType::BLUETOOTH:{
        // nullptr;
      }break;
      default:
      break;
    }
  }


  void close_threadImpl() {

    switch (config_.commType){
      case CCType::UDP:{
        emit publishNotify(GCW::NotifyType::Info, "UDP", tr("closed"));
        socket_udp_->close();
        open_udp_ = false;

        emit closed();
      }break;
      case CCType::TCP:{
        emit publishNotify(GCW::NotifyType::Info, "TCP", tr("closed"));
        socket_tcp_->close();
        open_tcp_ = false;

        emit closed();
      }break;
      case CCType::SERIAL:{
        emit publishNotify(GCW::NotifyType::Info, "Serial", tr("closed"));
        socket_serial->close();
        open_serial_ = false;

        emit closed();
      }break;
      case CCType::BLUETOOTH:{
        emit publishNotify(GCW::NotifyType::Info, "Bluetooth", tr("closed"));
        open_bluetooth_ = false;

        emit closed();
      }break;
      default:{
        emit publishNotify(GCW::NotifyType::Error, tr("Communicator"), tr("Unsupported communication type"));
      } break;
    }
  }
  void open_threadImpl() {

    bool result = true;
    QString error_msg;
    switch (config_.commType)
    {
    case CCType::UDP:{
      if (socket_udp_->isOpen()) {
        socket_udp_->close();
      }
      result &= socket_udp_->bind(
        QHostAddress::AnyIPv4, 
        config_.udp.listen ,
        QAbstractSocket::ReuseAddressHint | QAbstractSocket::ShareAddress);
      socket_udp_->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 2 * 1024 * 1024);

      socket_udp_tx_->connectToHost(QHostAddress(config_.udp.ip), config_.udp.port, QIODevice::WriteOnly);

      if(result){
        open_udp_ = true;
        emit publishNotify(GCW::NotifyType::Success, "UDP",
                           QString(tr("started! listen: %1, ip: %2, port: %3"))
                               .arg(QString::number(config_.udp.listen))
                               .arg(config_.udp.ip)
                               .arg(QString::number(config_.udp.port)));
      } else {
        error_msg = socket_udp_->errorString();
      }
    } break;
    case CCType::TCP:{
      if (socket_tcp_->isOpen()) {
        socket_tcp_->disconnectFromHost();
        socket_tcp_->close();
      }
      result = false;
      if(result){
        open_tcp_ = true;
        emit publishNotify(GCW::NotifyType::Success, "TCP", tr("started"));
      } else {
        error_msg = socket_tcp_->errorString();
      }
    } break;
    case CCType::SERIAL:{
      result = true;

      if (socket_serial->isOpen()){
        socket_serial->close();
      }

      socket_serial->setPortName(config_.serial.serialName);
      socket_serial->setBaudRate(config_.serial.baudRate);
      socket_serial->setParity(toQtParity(config_.serial.parity));
      socket_serial->setFlowControl(toQtFlowControl(config_.serial.flowControl));
      socket_serial->setStopBits(toQtStopBits(config_.serial.stopBits));
      socket_serial->setDataBits(toQtDataBits(config_.serial.dataBits));

      result &= socket_serial->open(QIODevice::ReadWrite);

      if(result){
        open_serial_ = true;
        emit publishNotify(GCW::NotifyType::Success, "Serial", tr("started"));
      } else {
        error_msg = socket_serial->errorString();
      }
    } break;
    case CCType::BLUETOOTH:{
      result = false;
      if(result){
        open_bluetooth_ = true;
        emit publishNotify(GCW::NotifyType::Success, "Bluetooth", tr("started"));
      }
    } break;
    default:
      emit publishNotify(GCW::NotifyType::Error, tr("Communicator"), tr("Unsupported communication type"));
      result = false;
      break;
    }
    if(!result){
      emit publishNotify(GCW::NotifyType::Warning, tr("Communicator"), tr("%1 startup error: %2").arg(toString(config_.commType),error_msg));
    }
    emit openResult(result);
  }

signals:
  void openResult(bool result);

  void closed();
  
  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

  void readyRead();

  void readySend();

  void finnished();

  void requestOpen();

  void requestClose();

public:
  QSerialPort::Parity toQtParity(CCParity parity) {
    switch (parity) {
    case CCParity::NONE:
      return QSerialPort::NoParity;
    case CCParity::ODD:
      return QSerialPort::OddParity;
    case CCParity::EVEN:
      return QSerialPort::EvenParity;
    case CCParity::MARK:
      return QSerialPort::MarkParity;
    case CCParity::SPACE:
      return QSerialPort::SpaceParity;
    }
    return QSerialPort::Parity::NoParity;
  }

  QSerialPort::FlowControl toQtFlowControl(CCFlowControl flow) {
    switch (flow) {
    case CCFlowControl::NONE:
      return QSerialPort::NoFlowControl;
    case CCFlowControl::SOFTWARE:
      return QSerialPort::SoftwareControl;
    case CCFlowControl::HARDWARE:
      return QSerialPort::HardwareControl;
    }
    return QSerialPort::NoFlowControl;
  }

  QSerialPort::StopBits toQtStopBits(CCStopBits stopBits) {
    switch (stopBits) {
    case CCStopBits::ONE:
      return QSerialPort::OneStop;
    case CCStopBits::TWO:
      return QSerialPort::TwoStop;
    case CCStopBits::ONEPOINTFIVE:
      return QSerialPort::OneAndHalfStop;
    }
    return QSerialPort::OneStop;
  }

  QSerialPort::DataBits toQtDataBits(CCDataBits dataBits) {
    switch (dataBits) {
    case CCDataBits::BITS_5:
      return QSerialPort::Data5;
    case CCDataBits::BITS_6:
      return QSerialPort::Data6;
    case CCDataBits::BITS_7:
      return QSerialPort::Data7;
    case CCDataBits::BITS_8:
      return QSerialPort::Data8;
    }
    return QSerialPort::Data8;
  }

  QString toString(CCType type){
    switch(type){
      case CCType::SERIAL:
        return "Serial";
      case CCType::TCP:
        return "TCP";
      case CCType::UDP:
        return "UDP";
      case CCType::BLUETOOTH:
        return "Bluetooth";
      default:
        return "Unknown";
    }
  }

private:
  QThread *thread_ = nullptr;

  CommunicationConfiguration config_;

  QUdpSocket *socket_udp_ = nullptr;
  QUdpSocket* socket_udp_tx_ = nullptr;
  QTcpSocket *socket_tcp_ = nullptr;
  QTcpServer *server_tcp_ = nullptr;
  QSerialPort *socket_serial = nullptr;

  std::atomic_bool open_udp_ = false;
  std::atomic_bool open_tcp_ = false;
  std::atomic_bool open_serial_ = false;
  std::atomic_bool open_bluetooth_ = false;

  std::atomic_bool dataTaken_ = true;

  static constexpr int bufferSize = 1 << 24;
  RingBuffer<uint8_t, bufferSize> rx_buffer;
  RingBuffer<uint8_t, bufferSize> tx_buffer;

  uint64_t writeBytesLength = 0;
  uint64_t readBytesLength = 0;
};
