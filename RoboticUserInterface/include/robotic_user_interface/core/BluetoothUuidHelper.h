#pragma once
#include <QBluetoothUuid>
#include <QString>
#include <QSet>

namespace BluetoothUuidHelper {

  // 串口标准 UUID
  static const QBluetoothUuid SerialPortUuid(
    QStringLiteral("00001101-0000-1000-8000-00805F9B34FB"));

  // 可以扩展更多已知 profile 的 UUID
  static const QSet<QBluetoothUuid> KnownSerialUuids = {
      SerialPortUuid
  };

  // 判断是否是串口服务
  inline bool isSerialPort(const QBluetoothUuid& uuid) {
    return KnownSerialUuids.contains(uuid);
  }

  // 可选: 判断是否是已知的标准 profile（比如 Handsfree, A2DP, etc.）
  inline QString profileName(const QBluetoothUuid& uuid) {
    if (uuid == SerialPortUuid) {
      return "Serial Port Profile (SPP)";
    }
    else if (uuid == QBluetoothUuid(QStringLiteral("0000111e-0000-1000-8000-00805f9b34fb"))) {
      return "Handsfree Audio Gateway";
    }
    else if (uuid == QBluetoothUuid(QStringLiteral("0000fd2d-0000-1000-8000-00805f9b34fb"))) {
      return "Assigned Number (FD2D) - Special profile";
    }
    return "Unknown / Vendor Specific";
  }

} // namespace BluetoothUuidHelper