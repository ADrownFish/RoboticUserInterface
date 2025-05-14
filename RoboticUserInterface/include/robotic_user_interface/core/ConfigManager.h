#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QDir>

#include <memory>
#include "Types.h"

class ConfigManager {
public:
    ConfigManager(const QString &configFile, std::shared_ptr<Configuration> configuration)
        : config_(configuration) {
        qDebug() << "Config file path: " << configFile;
        configFile_ = configFile;

        // 检查文件是否存在，若不存在则创建默认配置
        if (!QFile::exists(configFile)) {
          writeDefaultConfig();
        }
        readConfig();
    }

    ~ConfigManager() {
        writeConfig();
    }

    // 读取配置文件到结构体
    void readConfig() {
      QFile file(configFile_);
      if (!file.open(QIODevice::ReadOnly)) {
          qWarning() << "Failed to open config file for reading";
          return;
      }

      QByteArray fileData = file.readAll();
      file.close();

      QJsonDocument doc = QJsonDocument::fromJson(fileData);
      if (doc.isNull()) {
          qWarning() << "Invalid JSON format";
          return;
      }

      QJsonObject json = doc.object();
      // 读取 DisplayConfiguration
      QJsonObject displayObj = json.value("Display").toObject();
//      config_->display.highTempAlarm_motor = displayObj.value("highTempAlarm_motor").toInt(config_->display.highTempAlarm_motor);
//      config_->display.highTempAlarm_driver = displayObj.value("highTempAlarm_driver").toInt(config_->display.highTempAlarm_driver);
      config_->display.spacing = displayObj.value("spacing").toInt(config_->display.spacing);
      config_->display.farmRate = displayObj.value("farmRate").toInt(config_->display.farmRate);
      config_->display.precision = displayObj.value("precision").toInt(config_->display.precision);
      config_->display.contentsMargins = displayObj.value("contentsMargins").toInt(config_->display.contentsMargins);
      config_->display.setAngleUnit(static_cast<AngleUnit>(displayObj.value("angleUnit").toInt(static_cast<int>(config_->display.angleUnit))));

      // 读取 CardConfiguration
      QJsonObject cardObj = json.value("Card").toObject();
      config_->card.odom = cardObj.value("odom").toBool(config_->card.odom);
      config_->card.bms = cardObj.value("bms").toBool(config_->card.bms);
      config_->card.imu = cardObj.value("imu").toBool(config_->card.imu);
      config_->card.system = cardObj.value("system").toBool(config_->card.system);
      config_->card.command = cardObj.value("command").toBool(config_->card.command);
      config_->card.windowsWidth = cardObj.value("windowsWidth").toInt(config_->card.windowsWidth);
      config_->card.windowsHeight = cardObj.value("windowsHeight").toInt(config_->card.windowsHeight);

      // 读取 CommunicationConfiguration
      QJsonObject commObj = json.value("Communication").toObject();
      config_->comm.commType = static_cast<CommunicationConfiguration::CommType>(
          commObj.value("commType").toInt(static_cast<int>(config_->comm.commType)));

      QJsonObject udpObj = commObj.value("UDP").toObject();
      config_->comm.udp.listen = udpObj.value("listen").toInt(config_->comm.udp.listen);
      config_->comm.udp.ip = udpObj.value("ip").toString(config_->comm.udp.ip);
      config_->comm.udp.port = udpObj.value("port").toInt(config_->comm.udp.port);

      QJsonObject tcpObj = commObj.value("TCP").toObject();
      config_->comm.tcp.listen = tcpObj.value("listen").toInt(config_->comm.tcp.listen);
      config_->comm.tcp.ip = tcpObj.value("ip").toString(config_->comm.tcp.ip);
      config_->comm.tcp.port = tcpObj.value("port").toInt(config_->comm.tcp.port);

      QJsonObject serialObj = commObj.value("Serial").toObject();
      config_->comm.serial.serialName = serialObj.value("serialName").toString(config_->comm.serial.serialName);
      config_->comm.serial.baudRate = serialObj.value("baudRate").toInt(config_->comm.serial.baudRate);
      config_->comm.serial.parity = static_cast<CommunicationConfiguration::Parity>(
          serialObj.value("parity").toInt(static_cast<int>(config_->comm.serial.parity)));
      config_->comm.serial.flowControl = static_cast<CommunicationConfiguration::FlowControl>(
          serialObj.value("flowControl").toInt(static_cast<int>(config_->comm.serial.flowControl)));
      config_->comm.serial.stopBits = static_cast<CommunicationConfiguration::StopBits>(
          serialObj.value("stopBits").toInt(static_cast<int>(config_->comm.serial.stopBits)));
      config_->comm.serial.dataBits = static_cast<CommunicationConfiguration::DataBits>(
          serialObj.value("dataBits").toInt(static_cast<int>(config_->comm.serial.dataBits)));

      // 读取 ActionConfiguration
      QJsonObject actionObj = commObj.value("Action").toObject();
      for (int i = 0; i < 3; ++i) {
        // 读取 pos_limit
        QJsonObject posLimitObj = actionObj["pos_limit"].toArray().at(i).toObject();
        config_->action.pos_limit[i].lower = posLimitObj["lower"].toDouble();
        config_->action.pos_limit[i].upper = posLimitObj["upper"].toDouble();
      }

      for (int i = 0; i < 3; ++i) {
        // 读取 vel_limit
        QJsonObject velLimitObj = actionObj["vel_limit"].toArray().at(i).toObject();
        config_->action.vel_limit[i].lower = velLimitObj["lower"].toDouble();
        config_->action.vel_limit[i].upper = velLimitObj["upper"].toDouble();
      }

      for (int i = 0; i < 3; ++i) {
        // 读取 euler_limit
        QJsonObject eulerLimitObj = actionObj["euler_limit"].toArray().at(i).toObject();
        config_->action.euler_limit[i].lower = eulerLimitObj["lower"].toDouble();
        config_->action.euler_limit[i].upper = eulerLimitObj["upper"].toDouble();
      }

      for (int i = 0; i < 3; ++i) {
        // 读取 omega_limit
        QJsonObject omegaLimitObj = actionObj["omega_limit"].toArray().at(i).toObject();
        config_->action.omega_limit[i].lower = omegaLimitObj["lower"].toDouble();
        config_->action.omega_limit[i].upper = omegaLimitObj["upper"].toDouble();
      }

      // 读取其他值
      config_->action.pos_key[0] = actionObj["pos_key"].toArray().at(0).toDouble();
      config_->action.pos_key[1] = actionObj["pos_key"].toArray().at(1).toDouble();
      config_->action.pos_key[2] = actionObj["pos_key"].toArray().at(2).toDouble();

      config_->action.vel_key[0] = actionObj["vel_key"].toArray().at(0).toDouble();
      config_->action.vel_key[1] = actionObj["vel_key"].toArray().at(1).toDouble();
      config_->action.vel_key[2] = actionObj["vel_key"].toArray().at(2).toDouble();

      config_->action.euler_key[0] = actionObj["euler_key"].toArray().at(0).toDouble();
      config_->action.euler_key[1] = actionObj["euler_key"].toArray().at(1).toDouble();
      config_->action.euler_key[2] = actionObj["euler_key"].toArray().at(2).toDouble();

      config_->action.omega_key[0] = actionObj["omega_key"].toArray().at(0).toDouble();
      config_->action.omega_key[1] = actionObj["omega_key"].toArray().at(1).toDouble();
      config_->action.omega_key[2] = actionObj["omega_key"].toArray().at(2).toDouble();

      // 读取 File Path 
      QJsonArray filePathArray = json.value("FilePaths").toArray();
      config_->filePaths.clear();
      for (int i = 0; i < filePathArray.size(); i++){
        std::shared_ptr<FilePathConfiguration> fc = std::make_shared<FilePathConfiguration>();
        QJsonObject filePathObj = filePathArray.at(i).toObject();
        fc->host = filePathObj.value("host").toString();
        fc->username = filePathObj.value("username").toString();
        fc->password = filePathObj.value("password").toString();
        fc->path = filePathObj.value("path").toString();
        config_->filePaths.append(fc);
      }

      qDebug() << "Config read from JSON.";
    }

    // 写入配置到文件
    void writeConfig() {
      QJsonObject json;

      // 写入 DisplayConfiguration
      QJsonObject displayObj;
//      displayObj["highTempAlarm_motor"] = config_->display.highTempAlarm_motor;
//      displayObj["highTempAlarm_driver"] = config_->display.highTempAlarm_driver;
      displayObj["spacing"] = config_->display.spacing;
      displayObj["farmRate"] = config_->display.farmRate;
      displayObj["precision"] = config_->display.precision;
      displayObj["contentsMargins"] = config_->display.contentsMargins;
      displayObj["angleUnit"] = static_cast<int>(config_->display.angleUnit);
      json["Display"] = displayObj;

      // 写入 CardConfiguration
      QJsonObject cardObj;
      cardObj["odom"] = config_->card.odom;
      cardObj["bms"] = config_->card.bms;
      cardObj["imu"] = config_->card.imu;
      cardObj["system"] = config_->card.system;
      cardObj["command"] = config_->card.command;
      cardObj["windowsWidth"] = config_->card.windowsWidth;
      cardObj["windowsHeight"] = config_->card.windowsHeight;
      json["Card"] = cardObj;

      // 写入 CommunicationConfiguration
      QJsonObject commObj;
      commObj["commType"] = static_cast<int>(config_->comm.commType);

      QJsonObject udpObj;
      udpObj["listen"] = config_->comm.udp.listen;
      udpObj["ip"] = config_->comm.udp.ip;
      udpObj["port"] = config_->comm.udp.port;
      commObj["UDP"] = udpObj;

      QJsonObject tcpObj;
      tcpObj["listen"] = config_->comm.tcp.listen;
      tcpObj["ip"] = config_->comm.tcp.ip;
      tcpObj["port"] = config_->comm.tcp.port;
      commObj["TCP"] = tcpObj;

      QJsonObject serialObj;
      serialObj["serialName"] = config_->comm.serial.serialName;
      serialObj["baudRate"] = config_->comm.serial.baudRate;
      serialObj["parity"] = static_cast<int>(config_->comm.serial.parity);
      serialObj["flowControl"] = static_cast<int>(config_->comm.serial.flowControl);
      serialObj["stopBits"] = static_cast<int>(config_->comm.serial.stopBits);
      serialObj["dataBits"] = static_cast<int>(config_->comm.serial.dataBits);
      commObj["Serial"] = serialObj;
      json["Communication"] = commObj;

      // 写入 ActionConfiguration
      QJsonObject actionObj;
      QJsonArray posLimitArray;
      for (int i = 0; i < 3; ++i) {
        QJsonObject posLimitObj;
        posLimitObj["lower"] = config_->action.pos_limit[i].lower;
        posLimitObj["upper"] = config_->action.pos_limit[i].upper;
        posLimitArray.append(posLimitObj);
      }
      actionObj["pos_limit"] = posLimitArray;

      QJsonArray velLimitArray;
      for (int i = 0; i < 3; ++i) {
        QJsonObject velLimitObj;
        velLimitObj["lower"] = config_->action.vel_limit[i].lower;
        velLimitObj["upper"] = config_->action.vel_limit[i].upper;
        velLimitArray.append(velLimitObj);
      }
      actionObj["vel_limit"] = velLimitArray;

      QJsonArray eulerLimitArray;
      for (int i = 0; i < 3; ++i) {
        QJsonObject eulerLimitObj;
        eulerLimitObj["lower"] = config_->action.euler_limit[i].lower;
        eulerLimitObj["upper"] = config_->action.euler_limit[i].upper;
        eulerLimitArray.append(eulerLimitObj);
      }
      actionObj["euler_limit"] = eulerLimitArray;

      QJsonArray omegaLimitArray;
      for (int i = 0; i < 3; ++i) {
        QJsonObject omegaLimitObj;
        omegaLimitObj["lower"] = config_->action.omega_limit[i].lower;
        omegaLimitObj["upper"] = config_->action.omega_limit[i].upper;
        omegaLimitArray.append(omegaLimitObj);
      }
      QJsonArray posKeyArray;
      posKeyArray.append(config_->action.pos_key[0]);
      posKeyArray.append(config_->action.pos_key[1]);
      posKeyArray.append(config_->action.pos_key[2]);
      actionObj["pos_key"] = posKeyArray;

      QJsonArray velKeyArray;
      velKeyArray.append(config_->action.vel_key[0]);
      velKeyArray.append(config_->action.vel_key[1]);
      velKeyArray.append(config_->action.vel_key[2]);
      actionObj["vel_key"] = velKeyArray;

      QJsonArray eulerKeyArray;
      eulerKeyArray.append(config_->action.euler_key[0]);
      eulerKeyArray.append(config_->action.euler_key[1]);
      eulerKeyArray.append(config_->action.euler_key[2]);
      actionObj["euler_key"] = eulerKeyArray;

      QJsonArray omegaKeyArray;
      omegaKeyArray.append(config_->action.omega_key[0]);
      omegaKeyArray.append(config_->action.omega_key[1]);
      omegaKeyArray.append(config_->action.omega_key[2]);
      actionObj["omega_key"] = omegaKeyArray;
      json["Action"] = actionObj;

      // file path
      QJsonArray filePathArray;
      for (int i = 0; i < config_->filePaths.size(); i++){
        auto &fileCfg = config_->filePaths.at(i);
        QJsonObject filePathObj;
        filePathObj["host"]     = fileCfg->host;
        filePathObj["username"] = fileCfg->username;
        filePathObj["password"] = fileCfg->password;
        filePathObj["path"]     = fileCfg->path;
        filePathArray.append(filePathObj);
      }
      json["FilePaths"] = filePathArray;

      // 写入到文件
      QFile file(configFile_);
      if (!file.open(QIODevice::WriteOnly)) {
          qWarning() << "Failed to open config file for writing";
          return;
      }

      QJsonDocument doc(json);
      file.write(doc.toJson());
      file.close();

      qDebug() << "Config written to JSON.";
    }

    // 写入默认配置（仅在第一次启动时）
    void writeDefaultConfig() {
        writeConfig();  // 写入默认配置到文件
        qDebug() << "Default config written.";
    }

private:
    std::shared_ptr<Configuration> config_;
    QString configFile_;
};
