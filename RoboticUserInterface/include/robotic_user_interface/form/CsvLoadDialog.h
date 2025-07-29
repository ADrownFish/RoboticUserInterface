#pragma once

#include "ui_CsvLoadDialog.h"

#include "robotic_user_interface/core/DataStreamSolver.h"

#include "qt_gcw/QSnackbarManager.h"
#include "robotic_user_interface/core/Types.h"
#include "qwool/qwwindowwidget.h"

class CsvLoadDialog : public QWWindowWidget
{
  Q_OBJECT

public:
  CsvLoadDialog(QWidget *parent = nullptr);
  
  ~CsvLoadDialog();

  void init();

  void setConfiguration(std::shared_ptr<Configuration> config);

  void loadFile(const QString& path);

  void setSteamSolver(DataStreamSolver* ss);

signals:
  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

  void ok();

  void cancel();

private:
  void setupSignalConnection();

  void setupWidgetsControls();

  void loadit();

  void switchType(int index);

private:
  std::shared_ptr<Configuration> config_;
  QList<FluRadioButton* > radioButtons_;

  // 数据解析器
  QPointer<DataStreamSolver> dataStreamSolver_ = nullptr;

  QString filePath;

  Ui::CsvLoadDialog ui;
};
