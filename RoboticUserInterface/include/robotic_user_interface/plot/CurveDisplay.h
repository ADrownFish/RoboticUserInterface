#pragma once

#include <QPointer>
#include <QWidget>
#include <QSplitter>

#include "qt_gcw/QSnackbarManager.h"
#include "robotic_user_interface/plot/DataSourceViewer.h"
#include "robotic_user_interface/plot/CustomPlotMap.h"
#include "robotic_user_interface/core/DataSource.h"

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

signals:
  void publishNotify(GCW::NotifyType type, const QString &title, const QString &text);

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  void appendCustomPlot();

  void clearAllDataSource();

  void settingsVisible(bool ok);

  void gridLineVisible(bool ok);

  void setPaused(bool ok);

private:
  Ui::Curve ui;

  QTimer flushTimer_;

  QPointer<DataSourceViewer> dataSourceViewer_;

  CustomPlotMap* currentPlotMap = nullptr;

  QSplitter* splitter = nullptr;

  std::shared_ptr<Configuration> config_;
  std::shared_ptr<ObservationsBase> observations_;
  std::shared_ptr<DataSource> dataSource_;
};
