#pragma once

#include <QTimer>

#include "qt_gcw/QSnackbarManager.h"
#include "qt_gcw/QFlowLayout.h"

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

signals:
  void publishNotify(GCW::NotifyType type, const QString& title, const QString& text);

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  void makeCard();

private:
  std::shared_ptr<Configuration> config_;

  QFlowLayout *layout;
};
