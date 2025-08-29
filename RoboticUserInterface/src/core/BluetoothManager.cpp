#include "robotic_user_interface/core/BluetoothManager.h"
#include <QDebug>
#include <QMetaEnum>
#include <QTimer>
#include <QBluetoothLocalDevice>

BluetoothConfigurator::BluetoothConfigurator(QObject* parent)
  : QObject(parent),
  m_deviceAgent(nullptr),
  m_serviceAgent(nullptr) {
}

void BluetoothConfigurator::startDeviceScan() {
  if (m_deviceAgent) {
    m_deviceAgent->deleteLater();
  }

  m_deviceAgent = new QBluetoothDeviceDiscoveryAgent(this);
  // m_deviceAgent->

  connect(m_deviceAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
    this, &BluetoothConfigurator::deviceFound);
  connect(m_deviceAgent, &QBluetoothDeviceDiscoveryAgent::finished,
    this, &BluetoothConfigurator::deviceScanFinished);

  m_deviceAgent->start();
}

void BluetoothConfigurator::startServiceScan(const QString& deviceAddress) {
  if (m_serviceAgent) {
    m_serviceAgent->deleteLater();
  }

  m_serviceAgent = new QBluetoothServiceDiscoveryAgent(this);
  connect(m_serviceAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
    this, &BluetoothConfigurator::serviceFound);
  connect(m_serviceAgent, &QBluetoothServiceDiscoveryAgent::finished,
    this, &BluetoothConfigurator::serviceScanFinished);

  m_serviceAgent->setRemoteAddress(QBluetoothAddress(deviceAddress));
  m_serviceAgent->start();
}
void BluetoothConfigurator::stopDeviceScan(){
  if(m_deviceAgent){
    m_deviceAgent->stop();
    m_deviceAgent->deleteLater();
    m_deviceAgent = nullptr;
  }
}
void BluetoothConfigurator::stopServiceScan(){
  if(m_serviceAgent){
    m_serviceAgent->stop();
    m_serviceAgent->deleteLater();
    m_serviceAgent = nullptr;
  }
}

BluetoothConnectionManager::BluetoothConnectionManager(QObject* parent)
  : QObject(parent)
{
}

void BluetoothConnectionManager::setConfig(const BluetoothConfig& config) {
  m_config = config;
}

void BluetoothConnectionManager::open() {

  // std::unique_ptr<QBluetoothLocalDevice> btld_;
  if(BluetoothHelper::checkBluetoothAvailability() != BluetoothAvailability::Available){
    publishNotify(GCW::Info, tr("Bluetooth") ,tr("Please turn on Bluetooth"));
    return;
  }
  if(BluetoothHelper::checkDevicePairingStatus(m_config.address) == DevicePairingStatus::NotPaired){
    publishNotify(GCW::Info, tr("Bluetooth") ,tr("Please pair with the device"));
    return;
  }
  if(m_config.address.isNull() || m_config.type == BluetoothDeviceType::Unknown){
    publishNotify(GCW::Info, tr("Bluetooth") ,tr("Please select a Bluetooth device"));
    return;
  }

  if (m_config.type == BluetoothDeviceType::Classic || m_config.type == BluetoothDeviceType::DualMode) {
    if (m_socket) { m_socket->deleteLater(); m_socket = nullptr; }

    qDebug() << "Classic >>  select service: " << m_config.address << m_config.uuid;
    
    m_socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(m_socket, &QBluetoothSocket::connected, this, &BluetoothConnectionManager::onSocketConnected);
    connect(m_socket, &QBluetoothSocket::disconnected, this, &BluetoothConnectionManager::onSocketDisconnected);
    connect(m_socket, &QBluetoothSocket::readyRead, this, &BluetoothConnectionManager::onSocketReadyRead);
    connect(m_socket, qOverload<QBluetoothSocket::SocketError>(&QBluetoothSocket::errorOccurred),
      this, &BluetoothConnectionManager::onSocketError);

    if (m_socket->state() != QBluetoothSocket::SocketState::UnconnectedState) {
      m_socket->disconnectFromService();
      m_socket->close();
    }
    m_socket->connectToService(m_config.address, m_config.uuid);
  }
  else if(m_config.type == BluetoothDeviceType::BLE){
    if (m_controller) { m_controller->deleteLater(); m_controller = nullptr; }
    m_controller = QLowEnergyController::createCentral(m_config.deviceInfo, this);

    qDebug() << "BLE: select device name: " << m_config.deviceInfo.name() << "device address" << m_config.deviceInfo.address();

    connect(m_controller, &QLowEnergyController::connected, this, &BluetoothConnectionManager::onBleConnected);
    connect(m_controller, &QLowEnergyController::disconnected, this, &BluetoothConnectionManager::onBleDisconnected);
    connect(m_controller, &QLowEnergyController::stateChanged, this, [this]( QLowEnergyController::ControllerState state){
      qDebug() << "BLE: state changed: " << m_controller->state();
    });
    connect(m_controller, &QLowEnergyController::serviceDiscovered, this, &BluetoothConnectionManager::onBleServiceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished, this, &BluetoothConnectionManager::onBleServiceScanDone);
    connect(m_controller, &QLowEnergyController::errorOccurred, this, &BluetoothConnectionManager::onBleControllerError);

    m_controller->connectToDevice();
  }
}

void BluetoothConnectionManager::close() {
  if (m_config.type == BluetoothDeviceType::Classic && m_socket) {
    m_socket->disconnectFromService();
    m_socket->deleteLater();
    m_socket = nullptr;
  }
  else if (m_config.type == BluetoothDeviceType::BLE && m_controller) {
    m_controller->disconnectFromDevice();
    // if (m_service) { m_service->deleteLater(); m_service = nullptr; }
    m_controller->deleteLater();
    m_controller = nullptr;
  }
  m_connected = false;
}

bool BluetoothConnectionManager::isOpen() const {
  return m_connected;
}

// ---------------- Classic ----------------
bool BluetoothConnectionManager::write(const QByteArray& data) {
  if (m_config.type == BluetoothDeviceType::Classic && m_socket && m_connected) {
    qint64 written = m_socket->write(data);
    return written == data.size();
  }
  else if (m_config.type == BluetoothDeviceType::BLE && m_connected) {
    // QLowEnergyCharacteristic c = m_service->characteristic(m_writeChar.uuid());
    // if (!c.isValid()) return false;

    // m_service->writeCharacteristic(c, data, QLowEnergyService::WriteWithResponse);
    // return true;
  }
  return false;
}

void BluetoothConnectionManager::onSocketConnected() {
  m_connected = true;
  emit connected();
}

void BluetoothConnectionManager::onSocketDisconnected() {
  m_connected = false;
  emit disconnected();
}

void BluetoothConnectionManager::onSocketError(QBluetoothSocket::SocketError error) {
  publishNotify(GCW::Info, tr("Bluetooth") ,tr("Classic socket error: %1").arg(BluetoothHelper::toString(error)));
  emit disconnected();
}

void BluetoothConnectionManager::onSocketReadyRead() {
  QByteArray data = m_socket->readAll();
  emit readyRead(data);
}

// ---------------- BLE ----------------
void BluetoothConnectionManager::onBleConnected() {
  m_connected = true;
  emit connected();

  // // 请求更稳定、功耗稍高的连接参数
  // QLowEnergyConnectionParameters params;
  // params.setIntervalRange(40, 80);   // 连接间隔：50ms - 100ms
  // params.setLatency(0);              // 从机延迟
  // params.setSupervisionTimeout(300); // 监控超时：3000ms
  // m_controller->requestConnectionUpdate(params);

  qDebug() << " BLE connectied, start to discoverServices " << m_controller->state();
  m_discoveredServiceUuids.clear();

  QTimer::singleShot(100, [this](){
    m_controller->discoverServices();
  });
}

void BluetoothConnectionManager::onBleDisconnected() {
  m_connected = false;
  emit disconnected();
}

void BluetoothConnectionManager::onBleServiceDiscovered(const QBluetoothUuid& uuid) {
  qDebug() << "Service found: " << uuid;
  m_discoveredServiceUuids.append(uuid);

  // if(m_BLERxTxService != nullptr && (m_BLETxService != nullptr || m_currBTArgument.RxServiceUUID == m_currBTArgument.))
  //   return;
}

void BluetoothConnectionManager::onBleServiceScanDone() {
  for (const auto &uuid : m_discoveredServiceUuids) {
    QLowEnergyService *service = m_controller->createServiceObject(uuid, this);
    m_services.append(service);
    if (service) {
      connect(service, &QLowEnergyService::stateChanged, this, [this,service, uuid](QLowEnergyService::ServiceState s) {
        if (s == QLowEnergyService::RemoteServiceDiscovered) {
          qDebug() << "Service state changed:" << uuid << s;

          QTimer::singleShot(200, [this, service]() {
            
          int index = 0;
          auto chars = service->characteristics();
          for (auto &c : chars) {
            auto props = c.properties();
            // qDebug() << "Characteristic UUID:" << c.uuid().toString() << " properties:" << props;

            // 写特征
            if (props & QLowEnergyCharacteristic::Write || props & QLowEnergyCharacteristic::WriteNoResponse) {
              // qDebug() << "Potential write characteristic:" << c.uuid().toString();
              m_writeChar = c;
            }

            // 读/通知特征
            if (props & QLowEnergyCharacteristic::Read || props & QLowEnergyCharacteristic::Notify) {
              // qDebug() << "Potential read/notify characteristic:" << c.uuid().toString();
              m_readChar = c;

              if (m_controller && m_controller->state() == QLowEnergyController::ConnectedState) {
                // 订阅通知
                QLowEnergyDescriptor notificationDesc = c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
                if (notificationDesc.isValid()) {
                  QTimer::singleShot(150 * (index + 1), [service, c, notificationDesc](){
                    service->writeDescriptor(notificationDesc, QByteArray::fromHex("0100"));
                  });
                  qDebug() << "Subscribed to notifications for" << c.uuid().toString();
                }
              } else {
                qDebug() << "QLowEnergyController :: UnconnectedState" << m_controller->state() ;
              }
            }

            index++;
          }
          });
        }
      });

      connect(service, &QLowEnergyService::characteristicChanged, this, [this](const QLowEnergyCharacteristic &c, const QByteArray &value) {
        if (c == m_readChar) {
          emit readyRead(value);
        }
        qDebug() << "Notification from" << c.uuid().toString() << "value:" << value.toHex();
      });

      connect(service, &QLowEnergyService::errorOccurred, this, [this](QLowEnergyService::ServiceError error) {
        publishNotify(GCW::Info, tr("Bluetooth") ,tr("BLE error: %1").arg(BluetoothHelper::toString(error)));
        emit disconnected();
      });

      QTimer::singleShot(100 * m_services.size(), [this, service](){
        service->discoverDetails(); // 动态发现所有特征
      });
    } else {
      qDebug() << "creating service failed  " << uuid.toString();
    }
  }
}

void BluetoothConnectionManager::onBleControllerError(QLowEnergyController::Error error) {
  publishNotify(GCW::Info, tr("Bluetooth") ,tr("BLE controller error: %1").arg(BluetoothHelper::toString(error)));
  emit disconnected();
}




namespace BluetoothHelper { 
QString toString(QLowEnergyService::ServiceError error) {
  QString errorString;
  switch (error)
  {
    case QLowEnergyService::ServiceError::NoError:
    errorString = "No Error";
    break;
    case QLowEnergyService::ServiceError::OperationError:
    errorString = "Operation";
    break;
    case QLowEnergyService::ServiceError::CharacteristicWriteError:
    errorString = "Characteristic Write";
    break;
    case QLowEnergyService::ServiceError::DescriptorWriteError:
    errorString = "Descriptor Write";
    break;
    case QLowEnergyService::ServiceError::UnknownError:
    errorString = "Unknown";
    break;
    case QLowEnergyService::ServiceError::CharacteristicReadError:
    errorString = "Characteristic Read";
    break;
    case QLowEnergyService::ServiceError::DescriptorReadError:
    errorString = "Descriptor Read";
    break;
  }
  return errorString;
}
QString toString(QLowEnergyController::Error error) {
  QString errorString;
  switch (error)
  {
    case QLowEnergyController::Error::NoError:
    errorString = "No Error";
    break;
    case QLowEnergyController::Error::UnknownError:
    errorString = "Unknown Error";
    break;
    case QLowEnergyController::Error::UnknownRemoteDeviceError:
    errorString = "Unknown Remote Device";
    break;
    case QLowEnergyController::Error::NetworkError:
    errorString = "Network";
    break;
    case QLowEnergyController::Error::InvalidBluetoothAdapterError:
    errorString = "Invalid Bluetooth Adapter";
    break;
    case QLowEnergyController::Error::ConnectionError:
    errorString = "Connection";
    break;
    case QLowEnergyController::Error::AdvertisingError:
    errorString = "Advertising";
    break;
    case QLowEnergyController::Error::RemoteHostClosedError:
    errorString = "Remote Host Closed";
    break;
    case QLowEnergyController::Error::AuthorizationError:
    errorString = "Authorization";
    break;
    case QLowEnergyController::Error::MissingPermissionsError:
    errorString = "Missing Permissions";
    break;
    case QLowEnergyController::Error::RssiReadError:
    errorString = "RSSI Read";
    break;
  }
  return errorString;
}

QString toString(QBluetoothSocket::SocketError error){
  QString errorString;
  switch (error)
  {
    case QBluetoothSocket::SocketError::NoSocketError:
    errorString = "No Socket";
    break;
    case QBluetoothSocket::SocketError::UnknownSocketError:
    errorString = "Unknown Socket";
    break;
    case QBluetoothSocket::SocketError::RemoteHostClosedError:
    errorString = "Remote Host Closed";
    break;
    case QBluetoothSocket::SocketError::HostNotFoundError:
    errorString = "Host Not Found , May Be Device Is Poweroff";
    break;
    case QBluetoothSocket::SocketError::ServiceNotFoundError:
    errorString = "Service Not Found, Thinking of trying BLE mode?";
    break;
    case QBluetoothSocket::SocketError::NetworkError:
    errorString = "Network";
    break;
    case QBluetoothSocket::SocketError::UnsupportedProtocolError:
    errorString = "Unsupported Protocol";
    break;
    case QBluetoothSocket::SocketError::OperationError:
    errorString = "Operation";
    break;
    case QBluetoothSocket::SocketError::MissingPermissionsError:
    errorString = "Missing Permissions";
    break;
  }
  return errorString;
}

BluetoothAvailability checkBluetoothAvailability() {
  QBluetoothLocalDevice localDevice;

  // 检查本地设备是否支持蓝牙
  if (!localDevice.isValid()) {
    return BluetoothAvailability::NotSupported;
  }

  // 检查蓝牙是否开启
  if (localDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
    return BluetoothAvailability::PoweredOff;
  }

  // 检查是否有其他错误
  if (localDevice.hostMode() == QBluetoothLocalDevice::HostConnectable) {
    return BluetoothAvailability::Available;
  } else if (localDevice.hostMode() ==
             QBluetoothLocalDevice::HostDiscoverable) {
    return BluetoothAvailability::Available;
  } else if (localDevice.hostMode() ==
             QBluetoothLocalDevice::HostDiscoverableLimitedInquiry) {
    return BluetoothAvailability::Available;
  }

  return BluetoothAvailability::UnknownError;
}

// 检查指定MAC地址的设备是否已配对
DevicePairingStatus checkDevicePairingStatus(const QBluetoothAddress &deviceAddress) {

  QBluetoothLocalDevice localDevice;
  bool unpaired = (localDevice.pairingStatus(deviceAddress) == QBluetoothLocalDevice::Unpaired);
  if (unpaired) {
    return DevicePairingStatus::NotPaired;
  }
  return DevicePairingStatus::Paired;
}
}