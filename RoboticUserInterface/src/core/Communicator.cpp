#include "robotic_user_interface/core/Communicator.h"

#include <QEventLoop>

Communicator::Communicator(QObject *parent) : QObject(nullptr) {

  thread_ = new QThread(parent);
  this->moveToThread(thread_);
}
Communicator::~Communicator(){

}

void Communicator::init(){

  setupSignalConnection();
  thread_->start();
}

void Communicator::setup(const CommunicationConfiguration& comm){
  config_ = comm;
}

void Communicator::shutdown() {
  QEventLoop loop;
  QObject::connect(this, &Communicator::closed, &loop, &QEventLoop::quit);
  requestClose();
  loop.exec();

  QObject::connect(thread_, &QThread::finished, &loop, &QEventLoop::quit);
  thread_->quit();
  loop.exec();
}

bool Communicator::isOpen(){
  switch (config_.commType){
    case CCType::UDP:{
      return open_udp_;
    }break;
    case CCType::TCP:{
      if (config_.tcp.server) {
        return open_tcp_server_;
      } else {
        return open_tcp_;
      }
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

void Communicator::open() {
  requestOpen();
}

void Communicator::close() {
  requestClose();
}

void Communicator::write(const QByteArray& buffer){
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

void Communicator::read(QByteArray& buffer) {
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

uint64_t Communicator::getReadBytesLength() {
  return readBytesLength;
}

uint64_t Communicator::getWriteBytesLength() {
  return writeBytesLength;
}

void Communicator::setupSignalConnection(){

  QObject::connect(thread_, &QThread::started,  this, &Communicator::start);
}

void Communicator::start() {
  thread_->setPriority(QThread::HighPriority);

  // on obj thread
  socket_udp_ = new QUdpSocket(this);
  socket_udp_tx_ = new QUdpSocket(this);
  socket_tcp_ = new QTcpSocket(this);
  server_tcp_ = new QTcpServer(this);
  socket_serial = new QSerialPort(this);
  bluetooth_manager_ = new BluetoothConnectionManager(this);
  

  QObject::connect(socket_udp_, &QUdpSocket::readyRead, this,   &Communicator::readyReadUdp);
  QObject::connect(socket_udp_, &QUdpSocket::disconnected, [this]() {  open_udp_ = false; });

  QObject::connect(socket_tcp_, &QTcpSocket::readyRead, this,     &Communicator::readyReadTcp);
  QObject::connect(socket_tcp_, &QTcpSocket::stateChanged, this, [this](QAbstractSocket::SocketState state) {
    switch (state)      {
    case QAbstractSocket::ConnectedState:
      open_tcp_ = true;
      emit CommStatusChanged(true);
      emit publishNotify(GCW::NotifyType::Success, "TCP Client", tr("Successfully connected to the server"));
      break;
    case QAbstractSocket::UnconnectedState:
      open_tcp_ = false;
      emit CommStatusChanged(false);
      emit publishNotify(GCW::NotifyType::Success, "TCP Client", tr("The remote connection is disconnected"));
      break;
      // 其他状态可以忽略
    default:
      break;
    }
    });
  QObject::connect(socket_tcp_, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
    this, [this](QAbstractSocket::SocketError socketError) {
      open_tcp_ = false;
      socket_tcp_->disconnectFromHost();
      emit publishNotify(GCW::NotifyType::Warning, "TCP Client", tr("An error occurred: ") + socket_tcp_->errorString());
    });


  QObject::connect(bluetooth_manager_, &BluetoothConnectionManager::connected, this, [this]() {
    publishNotify(GCW::NotifyType::Success, "Bluetooth", tr("Successfully connected to the device"));
    open_bluetooth_ = true;
    emit CommStatusChanged(true);
    });
  QObject::connect(bluetooth_manager_, &BluetoothConnectionManager::disconnected, this, [this]() {
    publishNotify(GCW::NotifyType::Warning, "Bluetooth", tr("The device disconnected"));
    open_bluetooth_ = false;
    emit CommStatusChanged(false);
    });
  QObject::connect(bluetooth_manager_, &BluetoothConnectionManager::publishNotify, this, &Communicator::publishNotify);
  QObject::connect(bluetooth_manager_, &BluetoothConnectionManager::readyRead, this, [this](const QByteArray& data) {
    sendSignal_readyread(data);
    });

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

  QObject::connect(socket_serial, &QSerialPort::readyRead, this, [this]() { 
  });

  QObject::connect(this,             &Communicator::readySend, this,            &Communicator::readySendSocket);
  QObject::connect(this,             &Communicator::requestOpen, this,        &Communicator::open_threadImpl);
  QObject::connect(this,             &Communicator::requestClose, this,        &Communicator::close_threadImpl);
}

void Communicator::tcpNewConnection() {
  // 获取客户端socket
  QTcpSocket* clientSocket = server_tcp_->nextPendingConnection();
  server_tcp_client_count++;

  // 连接socket的信号
  connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket]() {
    readyReadServerTcp(clientSocket);
    });
  connect(clientSocket, &QTcpSocket::disconnected, this, [this, clientSocket]() {
    qDebug() << "Client disconnected:" << clientSocket->peerAddress().toString();
    clientSocket->deleteLater();  // 安全删除
    server_tcp_client_count--;
    if (server_tcp_client_count == 0) {
      emit publishFocusStatus(QString());
    } else {
      emit publishFocusStatus(QString("Client %1").arg(server_tcp_client_count));
    }
    });
  connect(clientSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
    this, [this, clientSocket](QAbstractSocket::SocketError) {
      QString errStr = clientSocket->errorString();
      emit publishNotify(GCW::NotifyType::Warning, "TCP", tr("An error occurred: ") + errStr);
      clientSocket->disconnectFromHost();
    });

  emit publishFocusStatus(QString("Client %1").arg(server_tcp_client_count));
  qDebug() << "New client connected:" << clientSocket->peerAddress().toString() << ":" << clientSocket->peerPort();
}

void Communicator::readyReadUdp(){
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
void Communicator::readyReadTcp(){
  // on obj thread
  QByteArray buffer = socket_tcp_->readAll();
  sendSignal_readyread(buffer);
}
void Communicator::readyReadServerTcp(QTcpSocket* socket) {
  // on obj thread
  QByteArray buffer = socket->readAll();
  sendSignal_readyread(buffer);
}

void Communicator::readyReadSerial(){
  // on obj thread
  QByteArray buffer = socket_serial->readAll();
  sendSignal_readyread(buffer);
}

void Communicator::sendSignal_readyread(const QByteArray& buffer){
    rx_buffer.WriteBatch((uint8_t*)buffer.data(), buffer.size());
    // If the data has not been taken, no signal is sent. maybe wrong ?
    if(dataTaken_){
      dataTaken_ = false;
      readyRead();
    } else {
    }
}

void Communicator::readySendSocket() {
  // on obj thread
  QByteArray buffer;
  buffer.resize(tx_buffer.Size());
  tx_buffer.ReadBatch((uint8_t *)buffer.data(), buffer.size());

  switch (config_.commType){
    case CCType::UDP:{
      if(open_udp_){
        socket_udp_tx_->write(buffer);
      }
    }break;
    case CCType::TCP:{
      if (config_.tcp.server) {
        if (open_tcp_server_) {
          QList<QTcpSocket*> clients = server_tcp_->findChildren<QTcpSocket*>();
          for (QTcpSocket* client : clients) {
            client->write(buffer);
          }
        }
      } else {
        if (open_tcp_) {
          socket_tcp_->write(buffer);
        }
      }
    }break;
    case CCType::SERIAL:{
      if(open_serial_){
        socket_serial->write(buffer);
      }
    }break;
    case CCType::BLUETOOTH:{
      bluetooth_manager_->write(buffer);
    }break;
    default:
    break;
  }
}


void Communicator::close_threadImpl() {

  switch (config_.commType){
    case CCType::UDP:{
      emit publishNotify(GCW::NotifyType::Info, "UDP", tr("closed"));
      socket_udp_->close();
      open_udp_ = false;

      emit closed();
    }break;
    case CCType::TCP:{
      if (config_.tcp.server) {
        server_tcp_->close();
        open_tcp_server_ = false;
        QList<QTcpSocket*> clients = server_tcp_->findChildren<QTcpSocket*>();
        for (QTcpSocket* client : clients) {
          client->disconnectFromHost();
          client->deleteLater();
        }

        emit publishNotify(GCW::NotifyType::Info, "TCP Server", tr("closed"));
        emit closed();
      } else {
        socket_tcp_->disconnectFromHost();
        socket_tcp_->abort();          // 立即关闭并重置socket
        socket_tcp_->readAll();       // 清除缓冲区
        socket_tcp_->close();
        server_tcp_client_count = 0;

        open_tcp_ = false;
        emit publishNotify(GCW::NotifyType::Info, "TCP Client", tr("closed"));
        emit closed();
      }
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
      bluetooth_manager_->close();
      emit closed();
    }break;
    default:{
      emit publishNotify(GCW::NotifyType::Error, tr("Communicator"), tr("Unsupported communication type"));
    } break;
  }
}
void Communicator::open_threadImpl() {

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
    // 清理socket
    if (socket_tcp_->isOpen()) {
      socket_tcp_->disconnectFromHost();
      socket_tcp_->abort();          // 立即关闭并重置socket
      socket_tcp_->readAll();       // 清除缓冲区
      socket_tcp_->close();
    }
    //  清理所有客户端连接
    server_tcp_->close();
    QList<QTcpSocket*> clients = server_tcp_->findChildren<QTcpSocket*>();
    for (QTcpSocket* client : clients) {
      client->disconnectFromHost();
      client->deleteLater();
    }
    server_tcp_client_count = 0;

    if (config_.tcp.server) {
      result &= server_tcp_->listen(QHostAddress::Any, config_.tcp.listen);
      if (result) {
        open_tcp_server_ = true;
        emit publishNotify(GCW::NotifyType::Success, "TCP Server", tr("Port monitoring has been started"));
      } else{
        error_msg = server_tcp_->errorString();
      }
    } else {
      emit CommStatusChanged(false);
      socket_tcp_->connectToHost(config_.tcp.ip, config_.tcp.port);
      emit publishNotify(GCW::NotifyType::Success, "TCP Client", tr("Connecting to server . . ."));
      return;
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
    emit CommStatusChanged(false);
    bluetooth_manager_->setConfig(config_.bluetooth);
    bluetooth_manager_->open();
    emit publishNotify(GCW::NotifyType::Info, "Bluetooth", tr("Connecting to device . . ."));
    
  } break;
  default:
    emit publishNotify(GCW::NotifyType::Error, tr("Communicator"), tr("Unsupported communication type"));
    result = false;
    break;
  }
  if(!result){
    emit publishNotify(GCW::NotifyType::Warning, tr("Communicator"), tr("%1 startup error: %2").arg(toString(config_.commType),error_msg));
  }
  emit CommStatusChanged(result);
}

QSerialPort::Parity Communicator::toQtParity(CCParity parity) {
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

QSerialPort::FlowControl Communicator::toQtFlowControl(CCFlowControl flow) {
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

QSerialPort::StopBits Communicator::toQtStopBits(CCStopBits stopBits) {
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

QSerialPort::DataBits Communicator::toQtDataBits(CCDataBits dataBits) {
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

QString Communicator::toString(CCType type){
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
