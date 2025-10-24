#pragma once

#include <QDialog>

#include "ui_CurveEditor.h"

#include "robotic_user_interface/core/DataSource.h"
#include "robotic_user_interface/plot/CustomPlotMap.h"

#include "qt_gcw/QSnackbarManager.h"

class CurveEditor : public QDialog {

  Q_OBJECT
public:
  CurveEditor(QWidget *parent = nullptr);
  ~CurveEditor();

  void init();

  void setConfiguration(const std::shared_ptr<Configuration>& config);

  void setObservations(const std::shared_ptr<ObservationsBase>& obs);

  void setDataSource(const std::shared_ptr<DataSource>& ds);

  void execEditor(CustomPlotLayer *layer);

signals:
  void publishNotify(GCW::NotifyType type, const QString &title, const QString &text);

private:
  void setupSignalConnection();

  void setupWidgetsControls();


private:
  Ui::CurveEditor ui;

  std::shared_ptr<Configuration>    config_;
  std::shared_ptr<ObservationsBase> observations_;
  std::shared_ptr<DataSource>       dataSource_;

  CustomPlotLayer* layer_ = nullptr;
};
