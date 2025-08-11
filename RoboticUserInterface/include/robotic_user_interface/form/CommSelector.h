#pragma once

#include "ui_CommSelector.h"

#include "qwool/qwwindowwidget.h"
#include "qt_gcw/QSnackbarManager.h"
#include "FluControls/FluMenu.h"

#include "robotic_user_interface/core/Types.h"

#include <QTimer>
#include <QPointer>

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

  void switchPageType(unsigned int index);

  void switchPageProtocol(unsigned int index);

private:
  std::shared_ptr<Configuration> config_;

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
