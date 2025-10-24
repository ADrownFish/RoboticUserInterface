#pragma once

#include <QPointer>
#include <QWidget>
#include <QSplitter>

#include "qwool/qwdropwidget.h"
#include "qt_gcw/QSnackbarManager.h"
#include "robotic_user_interface/plot/DataSourceViewer.h"
#include "robotic_user_interface/plot/CustomPlotMap.h"
#include "robotic_user_interface/plot/CurveEditor.h"
#include "robotic_user_interface/core/DataSource.h"
#include "robotic_user_interface/core/DataStreamSolver.h"

#include "ui_Curve.h"

class CurveDisplay : public QWidget {

  Q_OBJECT
public:
  CurveDisplay(QWidget *parent = nullptr);
  ~CurveDisplay();

  void init();

  void setConfiguration(const std::shared_ptr<Configuration>& config);

  void setObservations(const std::shared_ptr<ObservationsBase>& obs);

  void setDataSource(const std::shared_ptr<DataSource>& ds);

  void setSteamSolver(DataStreamSolver* ss);

  void setActivate(bool ok);

signals:
  void publishNotify(GCW::NotifyType type, const QString &title, const QString &text);

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  CustomPlotMap* appendCustomPlot(const QString& name);

  void clearAllDataSource();

  void settingsVisible(bool ok);

  void gridLineVisible(bool ok);

  void setPaused(bool ok);

  void updateOnce();

  void clearData();

  void exportDataImage();

  void exportDataFile();

  void SaveLayout();
  
  void loadLayout();

private:
  Ui::Curve ui;

  QTimer flushTimer_;

  QPointer<DataSourceViewer> dataSourceViewer_;
  QPointer<QWDropWidget>     dropButton_;
  QPointer<CurveEditor>      curveEditor_;
  CustomPlotMap*             currentPlotMap = nullptr;
  QSplitter*                 splitter = nullptr;

  std::shared_ptr<Configuration>    config_;
  std::shared_ptr<ObservationsBase> observations_;
  std::shared_ptr<DataSource>       dataSource_;

  QPointer<DataStreamSolver>        dataStreamSolver_;
};
