#pragma once

#include <QList>
#include <QString>
#include <QJsonArray>
#include <QFont>

#include <memory>
#include "Types.h"

struct InitialConfiguration {
  int fontSize = 10;
  QString configDir;
  QString configFile;
  QString pluginName = "DefaultRobot";
  QString language = "zh_CN";
};

class ConfigManager {
public:
    ConfigManager(const QString &configFile, 
                  std::shared_ptr<Configuration> configuration);

    ~ConfigManager();

    // 读取配置文件到结构体
    void readConfig();

    // 写入配置到文件
    void writeConfig();

    // 写入默认配置（仅在第一次启动时）
    void writeDefaultConfig();

    // 获取插件名称：不存在则默认为 DefaultRobot
    static QString getPluginName();

    // 返回配置文件目录、配置文件路径、插件名称、字体
    static InitialConfiguration getInitialConfiguration();

private:

  template <typename T>
  QJsonArray toJsonArray(const QList<T>&vec) {
    QJsonArray array;
    for (const auto& item : vec) {
      array.append(item);
    }
    return array;
  }

  template <typename T>
  QList<T> toList(const QJsonArray& array) {
    QList<T> list;
    for (const auto& value : array) {
      list.append(static_cast<T>(value.toVariant().value<T>()));
    }
    return list;
  }

private:
    std::shared_ptr<Configuration> config_;
    QString configFile_;
};
