#pragma once

#include <QObject>
#include <QBluetoothSocket>
#include <QLowEnergyController>
#include <QLowEnergyService>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceInfo>

#include "qt_gcw/QSnackbarManager.h"
#include "robotic_user_interface/core/Types.h"

using BluetoothConfig = CommunicationConfiguration::BluetoothConfig;
using BluetoothDeviceType = CommunicationConfiguration::BTDeviceType;

// 蓝牙可用性枚举
enum class BluetoothAvailability {
  Available,          // 蓝牙可用且已开启
  NotSupported,       // 设备不支持蓝牙
  PoweredOff,         // 蓝牙已关闭
  UnknownError        // 未知错误
};

// 设备配对状态枚举
enum class DevicePairingStatus {
  Paired,             // 设备已配对
  NotPaired,          // 设备未配对
};

namespace BluetoothHelper{

  QString toString(QLowEnergyService::ServiceError error);
  QString toString(QLowEnergyController::Error error);
  QString toString(QBluetoothSocket::SocketError error);

  BluetoothAvailability checkBluetoothAvailability();
  DevicePairingStatus checkDevicePairingStatus(const QBluetoothAddress &deviceAddress);
}

class BluetoothConfigurator : public QObject {
  Q_OBJECT
public:
  explicit BluetoothConfigurator(QObject* parent = nullptr);
  void startDeviceScan();
  void startServiceScan(const QString& deviceAddress);
  void stopDeviceScan();
  void stopServiceScan();

  BluetoothConfig currentConfig() const { return m_config; }
  void setConfig(const BluetoothConfig& cfg) { m_config = cfg; }

signals:
  void deviceFound(const QBluetoothDeviceInfo& info);
  void deviceScanFinished();
  void serviceFound(const QBluetoothServiceInfo& info);
  void serviceScanFinished();

private:
  QBluetoothDeviceDiscoveryAgent* m_deviceAgent;
  QBluetoothServiceDiscoveryAgent* m_serviceAgent;
  BluetoothConfig m_config;
};


class BluetoothConnectionManager : public QObject
{
  Q_OBJECT
public:
  explicit BluetoothConnectionManager(QObject* parent = nullptr);

  void setConfig(const BluetoothConfig& config);
  void open();
  void close();
  bool isOpen() const;
  bool write(const QByteArray& data); 

signals:
  void readyRead(const QByteArray& data); // 经典蓝牙数据或 BLE 特征变化
  void connected();
  void disconnected();
  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

private slots:
  // Classic
  void onSocketConnected();
  void onSocketDisconnected();
  void onSocketError(QBluetoothSocket::SocketError error);
  void onSocketReadyRead();

  // BLE
  void onBleConnected();
  void onBleDisconnected();
  void onBleServiceDiscovered(const QBluetoothUuid& uuid);
  void onBleServiceScanDone();
  void onBleControllerError(QLowEnergyController::Error error);

private:
  BluetoothConfig m_config;
  bool m_connected = false;

  
  // Classic
  QBluetoothSocket* m_socket = nullptr;
  
  // BLE
  QLowEnergyController* m_controller = nullptr;
  QList<QBluetoothUuid> m_discoveredServiceUuids;
  QLowEnergyService* m_BLERxTxService = nullptr; // Rx of all mode, Tx of BLE_1S1C and BLE 1S2C mode
  QLowEnergyService* m_BLETxService = nullptr;   // Tx of BLE_2S2C mode
  QLowEnergyCharacteristic m_writeChar;
  QLowEnergyCharacteristic m_readChar;
  QList<QLowEnergyService* > m_services;
};