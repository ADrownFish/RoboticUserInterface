#pragma once

#include "ui_CommSelector.h"

#include "qwool/qwwindowwidget.h"

#include "qt_gcw/QSnackbarManager.h"

#include <QTimer>

#include "robotic_user_interface/core/Types.h"

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

private:
  std::shared_ptr<Configuration> config_;

  Ui::CommSelector ui;
};
