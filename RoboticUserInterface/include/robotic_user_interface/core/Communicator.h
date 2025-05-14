#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>

#include "robotic_user_interface/core/Types.h"
#include "../libraries/qt_gcw/QSnackbarManager.h"

class Communicator : public QObject{
  Q_OBJECT
  
public:
  Communicator() : QObject(nullptr) {
    socket_udp_ = new QUdpSocket(this);
    socket_tcp_ = new QTcpSocket(this);
    server_tcp_ = new QTcpServer(this);
    
    //thread_ = new QThread(this);
    //this->moveToThread(thread_);
  }
  ~Communicator(){
    delete socket_udp_;
    delete socket_tcp_;
    delete server_tcp_;
  }

  void init(){
    setupSignalConnection();
  }

  void setup(const CommunicationConfiguration& comm){
    config_ = comm;
  }

  bool isOpen(){
    switch (config_.commType){
      case CommunicationConfiguration::CommType::UDP:{
        return open_udp_;
      }break;
      case CommunicationConfiguration::CommType::TCP:{
        return open_tcp_;
      }break;
      case CommunicationConfiguration::CommType::SERIAL:{
        return open_serial_;
      }break;
      case CommunicationConfiguration::CommType::BLUETOOTH:{
        return open_bluetooth_;
      }break;
    }
    return false;
  }
  void close(){
    switch (config_.commType){
      case CommunicationConfiguration::CommType::UDP:{
        emit publishNotify(GCW::NotifyType::Info, "UDP", "closed");
        socket_udp_->close();
        open_udp_ = false;
      }break;
      case CommunicationConfiguration::CommType::TCP:{
        emit publishNotify(GCW::NotifyType::Info, "TCP", "close");
        socket_tcp_->close();
        open_tcp_ = false;
      }break;
      case CommunicationConfiguration::CommType::SERIAL:{
        emit publishNotify(GCW::NotifyType::Info, "Serial", "close");
        open_serial_ = false;
      }break;
      case CommunicationConfiguration::CommType::BLUETOOTH:{
        emit publishNotify(GCW::NotifyType::Info, "Bluetooth", "close");
        open_bluetooth_ = false;
      }break;
      default:{
        emit publishNotify(GCW::NotifyType::Error, "Communicator", "Unsupported communication type");
      } break;
    }
  }
  void open(){
    bool result = true;
    switch (config_.commType)
    {
    case CommunicationConfiguration::CommType::UDP:{
      if (socket_udp_->isOpen()) {
        socket_udp_->close();
      }
      result &= socket_udp_->bind(config_.udp.listen);
      //socket_udp_->connectToHost(QHostAddress(config_.udp.ip), config_.udp.port);
      //result &= socket_udp_->isOpen();
      if(result){
        open_udp_ = true;
        emit publishNotify(GCW::NotifyType::Success, "UDP",
                           QString("started! listen: %1, ip: %2, port: %3")
                               .arg(QString::number(config_.udp.listen))
                               .arg(config_.udp.ip)
                               .arg(QString::number(config_.udp.port)));
      }
    } break;
    case CommunicationConfiguration::CommType::TCP:{
      if (socket_tcp_->isOpen()) {
        socket_tcp_->disconnectFromHost();
        socket_tcp_->close();
      }
      result = false;
      if(result){
        open_tcp_ = true;
        emit publishNotify(GCW::NotifyType::Success, "TCP", "started");
      }
    } break;
    case CommunicationConfiguration::CommType::SERIAL:{
      result = false;
      if(result){
        open_serial_ = true;
        emit publishNotify(GCW::NotifyType::Success, "Serial", "started");
      }
    } break;
    case CommunicationConfiguration::CommType::BLUETOOTH:{
      result = false;
      if(result){
        open_bluetooth_ = true;
        emit publishNotify(GCW::NotifyType::Success, "Bluetooth", "started");
      }
    } break;
    default:
      emit publishNotify(GCW::NotifyType::Error, "Communicator", "Unsupported communication type");
      result = false;
      break;
    }
    if(!result){
      emit publishNotify(GCW::NotifyType::Warning, "Communicator", "Communication startup error");
    }
    emit openResult(result);
  }

  void readyWrite(const QByteArray& buffer){
    switch (config_.commType){
      case CommunicationConfiguration::CommType::UDP:{
        if(open_udp_){
          socket_udp_->writeDatagram(buffer,QHostAddress(config_.udp.ip),config_.udp.port);
        }
      }break;
      case CommunicationConfiguration::CommType::TCP:{
        if(open_tcp_){
          socket_tcp_->write(buffer);
        }        
      }break;
      case CommunicationConfiguration::CommType::SERIAL:{
        // nullptr;
      }break;
      case CommunicationConfiguration::CommType::BLUETOOTH:{
        // nullptr;
      }break;
    }
  }

private:
  void setupSignalConnection(){
    if(socket_udp_){
      socket_udp_->disconnect();
      QObject::connect(socket_udp_, &QUdpSocket::readyRead, this, &Communicator::readyReadUdp);
      QObject::connect(socket_udp_, &QUdpSocket::disconnected, [this](){
        open_udp_ = false;
      });
    }
    if(socket_tcp_){
      socket_tcp_->disconnect();
      QObject::connect(socket_tcp_, &QTcpSocket::readyRead, this, &Communicator::readyReadTcp);
      QObject::connect(socket_tcp_, &QUdpSocket::disconnected, [this](){
        open_tcp_ = false;
      });
    }

  }
  void readyReadUdp(){
    QByteArray datagram;
    QHostAddress sender;
    quint16 senderPort; 
    
    datagram.resize(int(socket_udp_->pendingDatagramSize()));
    socket_udp_->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

    readyRead(datagram);
  }
  void readyReadTcp(){
    readyRead(socket_tcp_->readAll());
  }

signals:
  void openResult(bool result);
  
  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

  void readyRead(const QByteArray& buffer);

private:

  
private:
  QThread *thread_;

  CommunicationConfiguration config_;

  QUdpSocket *socket_udp_ = nullptr;
  QTcpSocket *socket_tcp_ = nullptr;
  QTcpServer *server_tcp_ = nullptr;

  bool open_udp_ = false;
  bool open_tcp_ = false;
  bool open_serial_ = false;
  bool open_bluetooth_ = false;

};
