#pragma once

#include "ui_Actuator.h"

#include "robotic_user_interface/core/Types.h"

#include "qt_material_widgets/qtmaterialtextfield.h"
#include "qt_gcw/QFlowLayout.h"
#include "qt_gcw/QSnackbarManager.h"
#include "qwool/qwwindowwidget.h"

#include <QGridLayout>

class ActuatorUnit : public QWWindowWidget
{
  Q_OBJECT
public:

public:
  ActuatorUnit(QWidget *parent = nullptr);
  ~ActuatorUnit();

  void setTitle(const QString &title);

  void setActuator(const ObservationsBase::Actuator &actuator);

  bool isDishplay() { return ui_actuator.toggle->isToggled();};


  void setConfiguration(std::shared_ptr<Configuration> config);

  void init();

  void setupWidgetsControls();

  void setupSignalConnection();
public:
  // ui
  Ui::Actuator ui_actuator;

private:
  ObservationsBase::Actuator actuator_;
  std::shared_ptr<Configuration> config_;
};

#include "ui_ActuatorDisplay.h"

class ActuatorDisplay : public QWWindowWidget
{
  Q_OBJECT
public:

public:
  ActuatorDisplay(QWidget *parent = nullptr);
  ~ActuatorDisplay();

  void setActuators(const QStringList &title);

  void setConfiguration(std::shared_ptr<Configuration> config);

  void init();

  void flushConfiguration();

  void updateActuators(const std::vector<ObservationsBase::Actuator> &actuators);

private:
  void setupSignalConnection();

  void setupWidgetsControls();

protected:
  void resizeEvent(QResizeEvent* event){
    int maxHeight = 0;
    for (ActuatorUnit *unit : actuator_){
      maxHeight = qMax(maxHeight, unit->height() + unit->y());
    }
    ui_display_.widget->setMaximumHeight(maxHeight);

    QWWindowWidget::resizeEvent(event);
  }

private:
  Ui::ActuatorDisplay ui_display_;

  std::vector<ActuatorUnit *> actuator_;
  QFlowLayout *layout_;

  std::shared_ptr<Configuration> config_;
};
