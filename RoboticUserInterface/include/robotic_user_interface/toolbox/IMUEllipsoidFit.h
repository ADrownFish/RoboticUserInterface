#pragma once

#include <QPointer>
#include <QWidget>
#include <QSplitter>
#include <QTimer>

#include "qt_gcw/QSnackbarManager.h"
#include "robotic_user_interface/core/Types.h"

#include "ui_IMUEllipsoidFit.h"

class Q3DScatter;
class QScatterDataProxy;

class IMUEllipsoidFit : public QWidget {

  Q_OBJECT
public:
  IMUEllipsoidFit(QWidget *parent = nullptr);
  ~IMUEllipsoidFit();

  void init();

  void setConfiguration(const std::shared_ptr<Configuration>& config);

  void setObservations(const std::shared_ptr<ObservationsBase>& obs);

  void setActivate(bool ok);

signals:
  void publishNotify(GCW::NotifyType type, const QString &title, const QString &text);
  void back();

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

  std::shared_ptr<Configuration> config_;
  std::shared_ptr<ObservationsBase> observations_;

  QTimer timer_;
  std::vector<vector3_t> dataVector;
  vector3_t lastAccel;

  Q3DScatter *scatter_;
  QScatterDataProxy *proxy_;
  QWidget* container_;
};
