#pragma once

#include "PluginBase.h"

#include "Eigen/Dense"

#include <QPointer>
#include <QWidget>
#include <QSplitter>
#include <QTimer>

#include "qt_gcw/QSnackbarManager.h"
#include "robotic_user_interface/core/Types.h"

#include "ui_IMUEllipsoidFit.h"

class Q3DScatter;
class QScatterDataProxy;

class IMUEllipsoidFit : public PluginBase {

  Q_OBJECT
public:
  IMUEllipsoidFit(QWidget *parent = nullptr);
  
  ~IMUEllipsoidFit();

  void setConfiguration(const std::shared_ptr<Configuration>& config);

  void setObservations(const std::shared_ptr<ObservationsBase>& obs);

  void setActivate(bool ok);

  bool initialize();

  QIcon pluginIcon() const override;

  QString pluginName() const override;

  QString pluginVersion() const override;

  QString pluginDescription() const override;

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  void fit();

  void clearData();

  void cachedData();

  void load();

  void display3D();

  void exportData();

private:
  Ui::IMUEllipsoidFit ui;

  QTimer timer_;
  std::vector<Eigen::Matrix<scalar_t, 3, 1>> dataVector;
  Eigen::Matrix<scalar_t, 3, 1> lastAccel;

  Q3DScatter *scatter_;
  QScatterDataProxy *proxy_;
  QWidget* container_;
};
