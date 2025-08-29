#pragma once

#include "ui_CommSelector.h"

#include "qwool/qwwindowwidget.h"
#include "qt_gcw/QSnackbarManager.h"
#include "FluControls/FluMenu.h"
#include "FluControls/FluRadioButton.h"

#include "robotic_user_interface/core/Types.h"

#include <QTimer>
#include <QBluetoothAddress>
#include <QPointer>

class QBluetoothLocalDevice;
class BluetoothConfigurator;


class BluetoothDevice {
  using BluetoothDeviceType = CommunicationConfiguration::BTDeviceType;
public:

  BluetoothDeviceType type = BluetoothDeviceType::Classic;
  QBluetoothDeviceInfo deviceInfo;
  QList<QBluetoothServiceInfo> ServiceInfos;

  QString typeString() const {
    if (type == BluetoothDeviceType::BLE) {
      return "BLE";
    }
    else if (type == BluetoothDeviceType::Classic) {
      return "Classic";
    } else if (type == BluetoothDeviceType::DualMode) {
      return "Dual Mode";
    }
    else {
      return "Unknown";
    }
  }

  BluetoothDevice() {  }
  ~BluetoothDevice() {  }
};

class CommSelector : public QWWindowWidget
{
  Q_OBJECT



public:
  CommSelector(QWidget *parent = nullptr);
  
  ~CommSelector();

  void init();

  void setConfiguration(std::shared_ptr<Configuration> config);

  void flushConfiguration();

signals:
  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);
  
  void ok();

  void cancel();

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  void pushParameters();

  void pullParameters();

  void scanSerialPort();

  void scanBluetoothDevice();

  void BluetoothDeviceFound(const QBluetoothDeviceInfo& info);

  void BluetoothServiceFound(const QBluetoothServiceInfo& info);

  QString resolveUuid(const QBluetoothUuid& uuid);

  void switchPageType(unsigned int index);

  void switchPageProtocol(unsigned int index);

  void switchPageTcp(unsigned int index);
  
  void updateBTServiceFromDevice(int serviceIndex);

private:
  std::shared_ptr<Configuration> config_;
  std::unique_ptr<BluetoothConfigurator> btc_;
  // std::unique_ptr<QBluetoothLocalDevice> btld_;
  
  std::map<int32_t, BluetoothDevice> BTDeviceMap_;
  QMap<QUuid, QString> uuid_map_;

  QPointer<FluMenu> menu_serialPort;
  QPointer<FluMenu> menu_baudRate;
  QPointer<FluMenu> menu_udp_ip;
  QPointer<FluMenu> menu_udp_listen;
  QPointer<FluMenu> menu_udp_port;
  QPointer<FluMenu> menu_tcp_ip;
  QPointer<FluMenu> menu_tcp_listen;
  QPointer<FluMenu> menu_tcp_port;

  Ui::CommSelector ui;
};
