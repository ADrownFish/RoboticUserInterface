#include "bluetoothmanager.h"
#include <QDebug>
#include <QBluetoothUuid>

BluetoothManager::BluetoothManager(QObject *parent) : QObject(parent)
{
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
            this, &BluetoothManager::onDeviceDiscovered);
    connect(discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished,
            this, &BluetoothManager::onDiscoveryFinished);
    
    classicSocket = nullptr;
    bleController = nullptr;
    bleService = nullptr;
}

BluetoothManager::~BluetoothManager()
{
    disconnect();
}

void BluetoothManager::startDeviceDiscovery()
{
    emit statusMessage("开始扫描设备...");
    discoveryAgent->start();
}

void BluetoothManager::onDeviceDiscovered(const QBluetoothDeviceInfo &device)
{
    DeviceType type = determineDeviceType(device);
    emit deviceDiscovered(device.name(), device.address().toString(), type);
}

void BluetoothManager::onDiscoveryFinished()
{
    emit statusMessage("设备扫描完成");
}

void BluetoothManager::connectToDevice(const QString &address)
{
    // 这里简化处理，实际应该根据设备类型选择连接方式
    // 这里以经典蓝牙为例
    currentDeviceAddress = address;
    currentDeviceType = Classic; // 简化处理，实际应该根据扫描结果判断
    
    if (currentDeviceType == Classic) {
        setupClassicBluetooth();
    } else {
        setupBLE();
    }
}

void BluetoothManager::setupClassicBluetooth()
{
    if (classicSocket) {
        classicSocket->deleteLater();
    }
    
    classicSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(classicSocket, &QBluetoothSocket::connected, this, &BluetoothManager::onSocketConnected);
    connect(classicSocket, &QBluetoothSocket::disconnected, this, &BluetoothManager::onSocketDisconnected);
    connect(classicSocket, &QBluetoothSocket::readyRead, this, &BluetoothManager::onSocketReadyRead);
    connect(classicSocket, &QBluetoothSocket::errorOccurred, this, &BluetoothManager::onSocketError);
    
    // 使用标准串口服务UUID
    QBluetoothUuid serviceUuid(QBluetoothUuid::ServiceClassUuid::SerialPort);
    classicSocket->connectToService(QBluetoothAddress(currentDeviceAddress), serviceUuid);
    
    emit statusMessage("正在连接经典蓝牙设备...");
}

void BluetoothManager::setupBLE()
{
    emit statusMessage("BLE连接功能待实现...");
    // BLE连接逻辑较为复杂，需要具体的服务UUID和特性UUID
}

void BluetoothManager::disconnect()
{
    if (classicSocket && classicSocket->state() == QBluetoothSocket::SocketState::ConnectedState) {
        classicSocket->disconnectFromService();
    }
    
    if (bleController) {
        bleController->disconnectFromDevice();
    }
}

void BluetoothManager::sendData(const QByteArray &data)
{
    if (classicSocket && classicSocket->state() == QBluetoothSocket::SocketState::ConnectedState) {
        classicSocket->write(data);
        emit statusMessage("数据已发送: " + data);
    } else {
        emit statusMessage("未连接，无法发送数据");
    }
}

void BluetoothManager::onSocketConnected()
{
    emit statusMessage("蓝牙连接成功");
    emit connected();
}

void BluetoothManager::onSocketDisconnected()
{
    emit statusMessage("蓝牙连接已断开");
    emit disconnected();
}

void BluetoothManager::onSocketReadyRead()
{
    QByteArray data = classicSocket->readAll();
    emit messageReceived(data);
}

void BluetoothManager::onSocketError(QBluetoothSocket::SocketError error)
{
    emit statusMessage("Socket错误: " + QString::number((int)error));
}

BluetoothManager::DeviceType BluetoothManager::determineDeviceType(const QBluetoothDeviceInfo &device)
{
    // 简单判断设备类型：如果支持LE就是BLE，否则是经典蓝牙
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        return BLE;
    }
    return Classic;
}