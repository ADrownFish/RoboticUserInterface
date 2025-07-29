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
#include "robotic_user_interface/core/DataAllocator.h"
#include "robotic_user_interface/core/Types.h"
#include "robotic_user_interface/core/DataStreamSolver.h"

#include "robotic_user_interface/plot/CurveDisplay.h"
#include "robotic_user_interface/form/FocusStatus.h"
#include "robotic_user_interface/form/CommSelector.h"
#include "robotic_user_interface/form/SettingsDisplay.h"
#include "robotic_user_interface/form/NavigationView.h"
#include "robotic_user_interface/form/ToolsBox.h"
#include "robotic_user_interface/form/CommTerminal.h"
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

  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

  void shutdown();

private:

  // signal & connection
  void setupSignalConnection();

  // ui initialization
  void setupWidgetsControls();

  // navigation
  void makeNav();

signals:

protected:
  void keyPressEvent(QKeyEvent *event) override;

  void keyReleaseEvent(QKeyEvent *event) override ;
  
  void resizeEvent(QResizeEvent* event) override;

  void closeEvent(QCloseEvent* event) override;

private:
  // ui
  Ui::RobotUserInterface ui;

  QWidget* desktopWidget = nullptr;

  // library
  QPointer<GCW::QSnackbarManager> snackbar_;
  
  // core
  QPointer<Communicator> communicator_;
  QPointer<RobotBase>        robotBase_;
  QPointer<DataAllocator>    dataAllocator_;
  QPointer<DataStreamSolver> dataStreamSolver_;

  // form
  QPointer<FocusStatus>      topStatus_;
  QPointer<CommSelector>  commSelector_;
  QPointer<SettingsDisplay>  settingsDisplay_;
  QPointer<CurveDisplay>     curveDisplay_;
  QPointer<ToolsBox>           toolsBox_;
  QPointer<CommTerminal>  commTerminal_;
  

  // dashboard
  QPointer<Dashboard> dashboard_base_;

  // library qt_widget
  QPointer<QtMaterialDialog> commSelectorDialog_;
  QPointer<NavigationView>   navView_;

  // timer
  QTimer timer_flushData;

  // thread
  std::mutex net_mutex;
  
private:
  int argc = 0;
  char **argv = 0;
};
