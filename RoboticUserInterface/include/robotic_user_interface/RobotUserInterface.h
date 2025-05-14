#pragma once

#include "ui_RobotUserInterface.h"

#include <QWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QPointer>

#include <mutex>

#include "robotic_user_interface/core/Communicator.h"
#include "robotic_user_interface/core/ConfigManager.h"
#include "robotic_user_interface/core/Types.h"
#include "robotic_user_interface/form/FocusStatus.h"
#include "robotic_user_interface/form/CommSelector.h"
#include "robotic_user_interface/form/SettingsDisplay.h"
#include "robotic_user_interface/plot/CurveDisplay.h"
#include "robotic_user_interface/form/NavigationView.h"
// #include "robotic_user_interface/form/FileCatcher.h"

#include "robotic_user_interface/dashboard/dashboard.h"
#include "robotic_user_interface/custom/RobotBase.h"

#include "qt_gcw/QSnackbarManager.h"
//#include "qt_material_widgets/qtmaterialdrawer.h"
#include "qt_material_widgets/qtmaterialdialog.h"
#include "qt_material_widgets/qtmaterialraisedbutton.h"

#include "FluControls/FluWindowKitWidget.h"


class RobotUserInterface : public FluWindowKitWidget
{
  Q_OBJECT

public:
  RobotUserInterface(int argc, char **argv,QWidget *parent = nullptr);
  ~RobotUserInterface();

  void setRobotBase(RobotBase* robotBase);

  void init();
  // notify
  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

private:
  // // config
  // void saveConfig();
  // void loadConfig();
  
  // signal & connection
  void setupSignalConnection();

  // ui initialization
  void setupWidgetsControls();

  void makeNav();

signals:

protected:
  void keyPressEvent(QKeyEvent *event) override ;
  void keyReleaseEvent(QKeyEvent *event) override ;
  
  void resizeEvent(QResizeEvent* event) override;
private:
  // ui
  Ui::RobotUserInterface ui;

  QWidget* desktopWidget = nullptr;

  // library
  QPointer<GCW::QSnackbarManager> snackbar_;
  
  // core
  QPointer<Communicator> communicator_;
  QPointer<RobotBase> robotBase_;

  // form
  QPointer<FocusStatus> topStatus_;
  QPointer<CommSelector> commSelector_;
  QPointer<SettingsDisplay> settingsDisplay_;
  QPointer<CurveDisplay> curveDisplay_;
  // QPointer<FileCatcher> fileCatcher_;
  
  // dashboard
  QPointer<Dashboard> dashboard_base_;

  // library qt_widget
  QPointer<QtMaterialDialog> commSelectorDialog_;
  QPointer<NavigationView> navView_;
  //QPointer<QtMaterialRaisedButton> button_operation_;
  //QPointer<QtMaterialRaisedButton> button_info_;
  //QPointer<QtMaterialRaisedButton> button_curve_;
  //QPointer<QtMaterialRaisedButton> button_comm_;
  //QPointer<QtMaterialRaisedButton> button_settings_;

  // bottom button
  //QPointer<QWidget> buttom_button_widget;
  //QPointer<QWWindowWidget> buttom_button_widget_bg;
  //QPointer<QtMaterialRaisedButton> button_nav_;
  //QPointer<SwitcherButtonItem> button_com_;
  //QPointer<SwitcherButtonItem> button_record_;
  // QPointer<QtMaterialRaisedButton> button_fileCatch_;

  
  // timer
  QTimer timer_flushData;

  // thread
  std::mutex net_mutex;
  
  //settings
  // std::unique_ptr<ConfigManager> configManager_;

  //data
//  SequenceData<MotionQuantity> SequenceData_;

private:
  int argc = 0;
  char **argv = 0;
};
