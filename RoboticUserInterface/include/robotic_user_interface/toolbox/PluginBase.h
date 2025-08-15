#pragma once 

#include <functional>

#include <QWidget>
#include "robotic_user_interface/core/Types.h"

class PluginBase : public QWidget {

  Q_OBJECT
public:
  enum NotifyType {
    Info = 0x00,    // 提示信息
    Success,        // 操作成功
    Debug,          // 调试信息
    Warning,        // 警告信息
    Error           // 错误信息
  };

  using NotifyCallback = std::function<void(NotifyType, const QString&, const QString&)>;

public:
  explicit PluginBase(QWidget *parent = nullptr) : QWidget(parent) {  }

  virtual ~PluginBase() = default;

  virtual void setActivate(bool ok){ activate_ = ok;  }

  void setConfiguration(const std::shared_ptr<Configuration>& config){ config_ = config;  }

  void setObservations(const std::shared_ptr<ObservationsBase>& obs){ obs_ = obs;  }

  void setNotifyCallback(NotifyCallback callback){ notifyCallback_ = callback; }

  virtual bool initialize() = 0;

  virtual QIcon pluginIcon() const = 0;

  virtual QString pluginName() const = 0;

  virtual QString pluginVersion() const = 0;

  virtual QString pluginDescription() const = 0;


protected:
  bool activate_ = false;
  std::shared_ptr<Configuration> config_;
  std::shared_ptr<ObservationsBase> obs_;
  
  NotifyCallback notifyCallback_ = nullptr;
};