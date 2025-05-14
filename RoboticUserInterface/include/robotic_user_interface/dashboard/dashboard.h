#pragma once

#include <QResizeEvent>
#include <QScrollArea>
#include <QKeyEvent>

#include "qt_gcw/QFlowLayout.h"
#include "qwool/qwwindowwidget.h"

#include "robotic_user_interface/form/ActuatorDisplay.h"
#include "robotic_user_interface/core/Types.h"

#include "ui_BMS.h"
#include "ui_IMU.h"
#include "ui_Host.h"
#include "ui_Odom.h"
#include "ui_ActionCommand.h"
#include "ui_Actuator.h"

class Dashboard : public QScrollArea{
public:
  Dashboard(QWidget *parent = nullptr);

  virtual ~Dashboard();

  void setConfiguration(std::shared_ptr<Configuration> config);

  void setObservations(std::shared_ptr<ObservationsBase> obs);

  void setCommand(std::shared_ptr<CommandBase> cmd);
  
  void flushConfiguration();

  void setActuators(const QStringList &title);

  void addChild(QWidget *child);

  void addChild(QLayoutItem* child);

  void addChilds(const QList<QWidget *> &list);

  void init();
  
  void keyPressEventGlobal(QKeyEvent *event) ;

  void keyReleaseEventGlobal(QKeyEvent *event) ;
protected:

  void initLabel();

  void resizeEvent(QResizeEvent* event) override;

  void setupWidgetsControls();

  void setupSignalConnection();

  void flush();

protected:
  Ui::Host ui_host;
  Ui::BMS ui_bms;
  Ui::IMU ui_imu;
  Ui::Odom ui_odom;
  Ui::ActionCommand ui_actionCommand;
  
  QWWindowWidget *widget_host;
  QWWindowWidget *widget_bms;
  QWWindowWidget *widget_imu;
  QWWindowWidget *widget_odom;
  QWWindowWidget *widget_actionCommand;

  ActuatorDisplay *widget_actuator;

private:
  QFlowLayout *layout_;
  QTimer timer_flush_;

  std::shared_ptr<Configuration> config_;
  std::shared_ptr<ObservationsBase> observations_;
  std::shared_ptr<CommandBase> command_;
};
