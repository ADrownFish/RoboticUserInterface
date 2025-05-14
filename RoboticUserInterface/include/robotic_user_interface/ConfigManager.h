#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QSettings>

class ConfigManager {
public:
  struct Config {
    int listen = 26666;
    QString targetIP = QString("127.0.0.1");
    int port = 25555;

    int listen_repost = 26667;
    QString targetIP_repost = QString("127.0.0.1");
    int port_repost = 26668;

    bool rad = false;
    int keepPrecision = 2;
    int flushFps = 10;
    int history_length = 2000;

    bool showPosition = true;
    bool showTorque = true;
    bool showVelocity = true;
    bool showTemp = true;

    bool showOdom = true;
    bool showIMU = true;
    bool showBMS = true;
    bool showView = true;
    bool showVz = true;

    float speed_x = 1.0;
    float speed_y = 0.5;
    float speed_yaw = 1.5;
  };

public:
  ConfigManager(const QString &configFile)
      : settings(configFile, QSettings::IniFormat) {
    qDebug() << " config File path: " << configFile;
    if (!QFile::exists(configFile)) {
      writeDefaultConfig();
    }
    readConfig();
  }

  // 读取配置文件到结构体
  void readConfig() {
    config.listen = settings.value("Network/listen", 8080).toInt();
    config.targetIP =
        settings.value("Network/targetIP", "192.168.1.100").toString();
    config.port = settings.value("Network/port", 8080).toInt();

    config.showPosition = settings.value("Display/showPosition", true).toBool();
    config.showTorque = settings.value("Display/showTorque", true).toBool();
    config.showVelocity = settings.value("Display/showVelocity", true).toBool();
    config.showTemp = settings.value("Display/showTemp", true).toBool();
    config.showOdom = settings.value("Display/showOdom", true).toBool();
    config.showIMU = settings.value("Display/showIMU", true).toBool();
    config.showBMS = settings.value("Display/showBMS", true).toBool();

    config.showView = settings.value("Display/showView", true).toBool();
    config.showVz = settings.value("Display/showVz", true).toBool();

    config.rad = settings.value("System/rad", config.rad).toBool();
    config.keepPrecision =
        settings.value("System/keepPrecision", config.keepPrecision).toInt();
    config.flushFps =
        settings.value("System/flushFps", config.flushFps).toInt();
    config.history_length =
        settings.value("System/history_length", config.history_length).toInt();

    config.speed_x =
        settings.value("Control/speed_x", config.speed_x).toFloat();
    config.speed_y =
        settings.value("Control/speed_y", config.speed_y).toFloat();
    config.speed_yaw =
        settings.value("Control/speed_yaw", config.speed_yaw).toFloat();

    qDebug() << "Config Read:";
    qDebug() << "listen =" << config.listen << ", targetIP =" << config.targetIP
             << ", port =" << config.port
             << ", showPosition =" << config.showPosition
             << ", showTorque =" << config.showTorque
             << ", showVelocity =" << config.showVelocity
             << ", showTemp =" << config.showTemp
             << ", showOdom =" << config.showOdom
             << ", showIMU =" << config.showIMU
             << ", showBMS =" << config.showBMS
             << ", showView =" << config.showView
             << ", showVz =" << config.showVz << ", rad =" << config.rad
             << ", keepPrecision =" << config.keepPrecision
             << ", flushFps =" << config.flushFps
             << ", history_length =" << config.history_length
             << ", speed_x =" << config.speed_x
             << ", speed_y =" << config.speed_y
             << ", speed_yaw =" << config.speed_yaw;
  }

  // 写入配置到文件
  void writeConfig() {
    qDebug() << "Writing config " << config.showPosition;
    settings.setValue("Network/listen", config.listen);
    settings.setValue("Network/targetIP", config.targetIP);
    settings.setValue("Network/port", config.port);

    settings.setValue("Display/showPosition", config.showPosition);
    settings.setValue("Display/showTorque", config.showTorque);
    settings.setValue("Display/showVelocity", config.showVelocity);
    settings.setValue("Display/showTemp", config.showTemp);
    settings.setValue("Display/showOdom", config.showOdom);
    settings.setValue("Display/showIMU", config.showIMU);
    settings.setValue("Display/showBMS", config.showBMS);

    settings.setValue("Display/showView", config.showView);
    settings.setValue("Display/showVz", config.showVz);

    settings.setValue("System/rad", config.rad);
    settings.setValue("System/keepPrecision", config.keepPrecision);
    settings.setValue("System/flushFps", config.flushFps);
    settings.setValue("System/history_length", config.history_length);

    settings.setValue("Control/speed_x", config.speed_x);
    settings.setValue("Control/speed_y", config.speed_y);
    settings.setValue("Control/speed_yaw", config.speed_yaw);
  }

  // 写入默认配置（仅在第一次启动时）
  void writeDefaultConfig() {
    writeConfig(); // 写入默认配置到文件
    qDebug() << "Default config written.";
  }

public:
  Config config;
  QSettings settings;
};
