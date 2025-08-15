#pragma once

#include <QTimer>
#include <QStackedWidget>

#include "qt_gcw/QSnackbarManager.h"
#include "qt_gcw/QFlowLayout.h"
#include "qt_material_widgets/qtmaterialraisedbutton.h"

#include "robotic_user_interface/core/Types.h"


class ToolsBox : public QWidget
{
  Q_OBJECT
    friend class RobotUserInterface;
public:
  ToolsBox(QWidget* parent = nullptr);

  ~ToolsBox();

  void init();

  void setConfiguration(std::shared_ptr<Configuration> config);

  void setObservations(std::shared_ptr<ObservationsBase> obs);

signals:
  void publishNotify(GCW::NotifyType type, const QString& title, const QString& text);

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  void makeCard();

private:
  QMap<QString, QWidget*> card_;

  std::shared_ptr<Configuration> config_;

  std::shared_ptr<ObservationsBase> observations_;

  QFlowLayout *layout;

  QStackedWidget *stackwidget;
  QWidget *mainPage;
  QtMaterialRaisedButton* homeButton;
};
