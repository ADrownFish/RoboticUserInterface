#pragma once

#include "ui_Settings.h"

#include "qwool/qwwindowwidget.h"

#include "qt_gcw/QSnackbarManager.h"

#include <QTimer>

#include "robotic_user_interface/core/Types.h"

class SettingsDisplay : public QWWindowWidget
{
  Q_OBJECT
  friend class RobotUserInterface;
public:
  SettingsDisplay(QWidget *parent = nullptr);
  
  ~SettingsDisplay();

  void init();

  void setConfiguration(std::shared_ptr<Configuration> config);

signals:
  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  void pushParameters();

  void pullParameters();

private:
  std::shared_ptr<Configuration> config_;

  Ui::SettingsDisplay ui;
};
