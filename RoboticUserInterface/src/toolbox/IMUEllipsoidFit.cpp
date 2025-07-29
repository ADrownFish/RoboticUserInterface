#include "robotic_user_interface/toolbox/IMUEllipsoidFit.h"

#include "ellipsoid/fit.h"
#include "ellipsoid/generate.h"

#include <QFileDialog>
#include <QtDataVisualization/Q3DScatter>
#include <QtDataVisualization/QScatter3DSeries>
#include <QtDataVisualization/QScatterDataProxy>

#include <FluControls/FluConfirmFlyout.h>

IMUEllipsoidFit::IMUEllipsoidFit(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
}

IMUEllipsoidFit::~IMUEllipsoidFit() {
  container_->deleteLater();
}

void IMUEllipsoidFit::init() {
  setupWidgetsControls();
  setupSignalConnection();

  lastAccel.setZero();

  timer_.setInterval(10);
}

void IMUEllipsoidFit::setConfiguration(const std::shared_ptr<Configuration> &config) {
  config_ = config;
}

void IMUEllipsoidFit::setObservations(const std::shared_ptr<ObservationsBase> &obs) {
  observations_ = obs;
}

void IMUEllipsoidFit::setupSignalConnection() {
  QObject::connect(ui.back, &QPushButton::clicked, [this](){
    emit back();
  });

  QObject::connect(&timer_, &QTimer::timeout, this, &IMUEllipsoidFit::cachedData);
  QObject::connect(ui.button_clear, &QPushButton::clicked, this, &IMUEllipsoidFit::clearData);
  QObject::connect(ui.button_fit, &QPushButton::clicked, this, &IMUEllipsoidFit::fit);
  QObject::connect(ui.button_load, &QPushButton::clicked, this, &IMUEllipsoidFit::load);
  QObject::connect(ui.button_3d, &QPushButton::clicked, this, &IMUEllipsoidFit::display3D);
  QObject::connect(ui.button_export, &QPushButton::clicked, this, &IMUEllipsoidFit::exportData);

  QObject::connect(ui.ModeSelector, &QWWindowButton::selectUnitIndexChanged, [this](unsigned int index){
    ui.stackedWidget->setCurrentIndex(index);
  });

  QObject::connect(ui.HowToUse, &QPushButton::clicked, [this](bool clicked){
    auto flyout = new FluConfirmFlyout(ui.HowToUse, FluFlyoutPosition::Right);
    flyout->setTitle(tr("IMU Ellipsoid Fit"));
    flyout->setInfo(tr("IMU data ellipsoid fitting tool, of course, any ellipsoid fitting needs can be used"));
    flyout->show();
  });

  QObject::connect(ui.help_center, &QPushButton::clicked, [this](bool clicked){
    auto flyout = new FluConfirmFlyout(ui.help_center, FluFlyoutPosition::Right);
    flyout->setTitle(tr("Ellipsoid center"));
    flyout->setInfo(tr("also known as bias"));
    flyout->show();
  });

  QObject::connect(ui.help_radii, &QPushButton::clicked, [this](bool clicked){
    auto flyout = new FluConfirmFlyout(ui.help_radii, FluFlyoutPosition::Right);
    flyout->setTitle(tr("Ellipsoid semi-axis length"));
    flyout->setInfo(tr("i.e. scaling factor. Unitless (normalized quantity)"));
    flyout->show();
  });

  QObject::connect(ui.help_evecColumn, &QPushButton::clicked, [this](bool clicked){
    auto flyout = new FluConfirmFlyout(ui.help_evecColumn, FluFlyoutPosition::Right);
    flyout->setTitle(tr("Spindle direction rotation matrix"));
    flyout->setInfo(tr("Transformation between ellipsoid and reference coordinates, unitless"));
    flyout->show();
  });

  QObject::connect(ui.help_eval, &QPushButton::clicked, [this](bool clicked){
    auto flyout = new FluConfirmFlyout(ui.help_eval, FluFlyoutPosition::Right);
    flyout->setTitle(tr("Ellipsoid eigenvalues"));
    flyout->setInfo(tr("Related to the scaling factor"));
    flyout->show();
  });

  QObject::connect(ui.help_coefficients, &QPushButton::clicked, [this](bool clicked){
    auto flyout = new FluConfirmFlyout(ui.help_coefficients, FluFlyoutPosition::Right);
    flyout->setTitle(tr("Ellipsoid Algebraic Equation Coefficients"));
    flyout->setInfo(tr("Show as formula"));
    flyout->show();
  });

}

void IMUEllipsoidFit::setupWidgetsControls() {

  ui.back->setText(tr("Back"));
  ui.HowToUse->setIcon(QIcon(":/svg/svg/question.svg"));

  ui.button_clear->setIcon(QIcon(":/svg/svg/clear.svg"));
  ui.button_fit->setIcon(QIcon(":/svg/svg/fit.svg"));
  ui.button_3d->setIcon(QIcon(":/svg/svg/show.svg"));
  ui.button_export->setIcon(QIcon(":/svg/svg/export.svg"));

  ui.button_clear->setText(tr("Clear"));
  ui.button_fit->setText(tr("Fit"));
  ui.button_load->setText(tr("Load"));
  ui.button_3d->setText(tr("Display"));
  ui.button_export->setText(tr("Export"));

  ui.help_center->setIcon(QIcon(":/svg/svg/question.svg"));
  ui.help_coefficients->setIcon(QIcon(":/svg/svg/question.svg"));
  ui.help_eval->setIcon(QIcon(":/svg/svg/question.svg"));
  ui.help_evecColumn->setIcon(QIcon(":/svg/svg/question.svg"));
  ui.help_radii->setIcon(QIcon(":/svg/svg/question.svg"));

  ui.lineEdit_linear_acc_x->setLabel("x");
  ui.lineEdit_linear_acc_y->setLabel("y");
  ui.lineEdit_linear_acc_z->setLabel("z");

  ui.lineEdit_linear_cached->setLabel(tr("The current number of caches"));
  ui.lineEdit_linear_cached->setText(QString::number(0));

  ui.lineEdit_center_x->setLabel("x");
  ui.lineEdit_center_y->setLabel("y");
  ui.lineEdit_center_z->setLabel("z");

  ui.lineEdit_radii_x->setLabel("x");
  ui.lineEdit_radii_y->setLabel("y");
  ui.lineEdit_radii_z->setLabel("z");

  ui.lineEdit_evecColumn_00->setLabel("0,0");
  ui.lineEdit_evecColumn_01->setLabel("0,1");
  ui.lineEdit_evecColumn_02->setLabel("0,2");
  ui.lineEdit_evecColumn_10->setLabel("1,0");
  ui.lineEdit_evecColumn_11->setLabel("1,1");
  ui.lineEdit_evecColumn_12->setLabel("1,2");
  ui.lineEdit_evecColumn_20->setLabel("2,0");
  ui.lineEdit_evecColumn_21->setLabel("2,1");
  ui.lineEdit_evecColumn_22->setLabel("2,2");

  ui.lineEdit_eval_x->setLabel("x");
  ui.lineEdit_eval_y->setLabel("y");
  ui.lineEdit_eval_z->setLabel("z");

  ui.lineEdit_linear_loadName_x->setLabel("CSV x");
  ui.lineEdit_linear_loadName_y->setLabel("CSV y");
  ui.lineEdit_linear_loadName_z->setLabel("CSV z");

  ui.lineEdit_linear_loadName_x->setText("ax");
  ui.lineEdit_linear_loadName_y->setText("ay");
  ui.lineEdit_linear_loadName_z->setText("az");

  ui.coefficients->setLabel(tr("Ellipsoid Algebraic Equation Coefficients"));
  
  ui.toggle->setToggle(false);

  ui.ModeSelector->addUnit(tr("Load File"));
  ui.ModeSelector->addUnit(tr("Online Cache"));
  // ui.ModeSelector->setSelectdColor(QColor(220, 120, 120));
  ui.ModeSelector->setBackgroundColor(QColor(100,100,100,50));

  // 创建 3d 数据
  scatter_ = new Q3DScatter();
  container_ = QWidget::createWindowContainer(scatter_);
  container_->setAttribute(Qt::WA_TransparentForMouseEvents, false);
  container_->setFocusPolicy(Qt::NoFocus);

  proxy_ = new QScatterDataProxy();
  auto *series = new QScatter3DSeries(proxy_);
  scatter_->addSeries(series);

  // 设置点大小
  series->setItemSize(0.15f);  // 放大点


  // 设置主题、取消阴影
  scatter_->activeTheme()->setType(Q3DTheme::ThemeStoneMoss);
  scatter_->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
  scatter_->axisX()->setTitle(tr("X Axis"));
  scatter_->axisY()->setTitle(tr("Y Axis"));
  scatter_->axisZ()->setTitle(tr("Z Axis"));
  scatter_->axisX()->setTitleVisible(true);
  scatter_->axisY()->setTitleVisible(true);
  scatter_->axisZ()->setTitleVisible(true);
}


void IMUEllipsoidFit::fit(){
  Eigen::Matrix<double, 10, 1> coefficients; // 10 coefficients
  Eigen::Vector3d eval;                      // eigenvalues
  Eigen::Matrix3d evec_column;               // eigenvectors in column

  Eigen::Matrix<double, Eigen::Dynamic, 3> dataMat(dataVector.size(), 3);
  for (size_t i = 0; i < dataVector.size(); ++i) {
    dataMat.row(i) = dataVector[i].transpose();  // 每个 vector3_t 是列向量，转置成行
  }

  auto identified_parameters =
    ellipsoid::fit(dataMat, &coefficients, &eval, &evec_column,
        ellipsoid::EllipsoidType::Arbitrary);

  ui.lineEdit_center_x->setText(QString::number(identified_parameters.center(0)));
  ui.lineEdit_center_y->setText(QString::number(identified_parameters.center(1)));
  ui.lineEdit_center_z->setText(QString::number(identified_parameters.center(2)));

  ui.lineEdit_radii_x->setText(QString::number(identified_parameters.radii(0)));
  ui.lineEdit_radii_y->setText(QString::number(identified_parameters.radii(1)));
  ui.lineEdit_radii_z->setText(QString::number(identified_parameters.radii(2)));

  ui.lineEdit_evecColumn_00->setText(QString::number(evec_column(0,0)));
  ui.lineEdit_evecColumn_01->setText(QString::number(evec_column(0,1)));
  ui.lineEdit_evecColumn_02->setText(QString::number(evec_column(0,2)));
  ui.lineEdit_evecColumn_10->setText(QString::number(evec_column(1,0)));
  ui.lineEdit_evecColumn_11->setText(QString::number(evec_column(1,1)));
  ui.lineEdit_evecColumn_12->setText(QString::number(evec_column(1,2)));
  ui.lineEdit_evecColumn_20->setText(QString::number(evec_column(2,0)));
  ui.lineEdit_evecColumn_21->setText(QString::number(evec_column(2,1)));
  ui.lineEdit_evecColumn_22->setText(QString::number(evec_column(2,2)));

  ui.lineEdit_eval_x->setText(QString::number(eval(0)));
  ui.lineEdit_eval_y->setText(QString::number(eval(1)));
  ui.lineEdit_eval_z->setText(QString::number(eval(2)));

  QString eqn;
  QStringList vars = {
      "x^2", "y^2", "z^2", "xy", "xz", "yz", "x", "y", "z", ""
  };

  for (int i = 0; i < 10; ++i) {
      double val = coefficients(i);
      if (qFuzzyIsNull(val)) continue;  // 跳过0系数

      QString term;
      if (val > 0 && !eqn.isEmpty())
          term += "+ ";

      if (val < 0)
          term += "- ";

      term += QString::number(std::abs(val), 'g', 6);
      if (!vars[i].isEmpty())
          term += vars[i];

      eqn += term + " ";
  }

  eqn += "= 0";
  ui.coefficients->setText(eqn);

}

void IMUEllipsoidFit::clearData(){
  dataVector.clear();
  ui.lineEdit_linear_cached->setText(QString::number(0));
}

void IMUEllipsoidFit::cachedData(){
  ui.lineEdit_linear_acc_x->setText(QString::number(observations_->imu.acceleration[0], 'f', config_->display.precision));
  ui.lineEdit_linear_acc_y->setText(QString::number(observations_->imu.acceleration[1], 'f', config_->display.precision));
  ui.lineEdit_linear_acc_z->setText(QString::number(observations_->imu.acceleration[2], 'f', config_->display.precision));

  if(ui.toggle->isToggled()){
    vector3_t data;
    data[0] = observations_->imu.acceleration[0];
    data[1] = observations_->imu.acceleration[1];
    data[2] = observations_->imu.acceleration[2];

    if(lastAccel == data){
      return;
    }
    lastAccel = data;

    dataVector.push_back(data);
    ui.lineEdit_linear_cached->setText(QString::number(dataVector.size()));
  }
}

void IMUEllipsoidFit::setActivate(bool ok) {
  if(ok){
    timer_.start();
  } else {
    timer_.stop();
  }
}

void IMUEllipsoidFit::load() {
  QStringList filenames = QFileDialog::getOpenFileNames(
      nullptr, tr("Select CSV Files"), "", "CSV Files (*.csv);;All Files (*)");

  if (filenames.isEmpty()) {
    emit publishNotify(GCW::NotifyType::Warning, tr("No File Selected"),
                       tr("No file was selected for loading."));
    return;
  }

  QString ax_name = ui.lineEdit_linear_loadName_x->text();
  QString ay_name = ui.lineEdit_linear_loadName_y->text();
  QString az_name = ui.lineEdit_linear_loadName_z->text();

  std::vector<vector3_t> newAccelData;
  int totalValidSamples = 0;

  for (const QString &filename : filenames) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      emit publishNotify(GCW::NotifyType::Warning, tr("File Open Error"),
                         QString(tr("Failed to open file: %1")).arg(filename));
      continue;
    }

    QTextStream in(&file);
    int lineNum = 0;
    int axIdx = -1, ayIdx = -1, azIdx = -1;

    // 读取首行标题
    QString headerLine = in.readLine().trimmed();
    lineNum++;
    QStringList headers =
        headerLine.split(QRegularExpression("[,;\\s]"), Qt::SkipEmptyParts);

    for (int i = 0; i < headers.size(); ++i) {
      QString h = headers[i].trimmed().toLower();
      if (h == ax_name)
        axIdx = i;
      else if (h == ay_name)
        ayIdx = i;
      else if (h == az_name)
        azIdx = i;
    }

    if (axIdx == -1 || ayIdx == -1 || azIdx == -1) {
      emit publishNotify(
          GCW::NotifyType::Warning, tr("Invalid Header"),
          QString(tr("File '%1' is missing required columns: %2, %3, %4. Skipped."))
              .arg(filename,ax_name,ay_name,az_name));
      continue;
    }

    // 读取数据
    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      lineNum++;

      if (line.isEmpty() || line.startsWith("#"))
        continue;

      QStringList parts =
          line.split(QRegularExpression("[,;\\s]"), Qt::SkipEmptyParts);
      if (parts.size() <= std::max({axIdx, ayIdx, azIdx})) {
        emit publishNotify(
            GCW::NotifyType::Warning, tr("Incomplete Line"),
            QString(tr("File '%1', line %2 has insufficient columns. Skipped."))
                .arg(filename)
                .arg(lineNum));
        continue;
      }

      bool okX = false, okY = false, okZ = false;
      double ax = parts[axIdx].toDouble(&okX);
      double ay = parts[ayIdx].toDouble(&okY);
      double az = parts[azIdx].toDouble(&okZ);

      if (!okX || !okY || !okZ) {
        emit publishNotify(
            GCW::NotifyType::Warning, tr("Parse Error"),
            QString(
                tr("File '%1', line %2 contains invalid float values. Skipped."))
                .arg(filename)
                .arg(lineNum));
        continue;
      }

      newAccelData.emplace_back(ax, ay, az);
    }

    file.close();
  }

  if (newAccelData.empty()) {
    emit publishNotify(
        GCW::NotifyType::Error, tr("No Valid Data"),
        tr("No valid IMU acceleration data found in the selected files."));
    return;
  }

  dataVector.insert(dataVector.end(), newAccelData.begin(),
                     newAccelData.end());

  emit publishNotify(
      GCW::NotifyType::Success, tr("Data Loaded"),
      QString(
          tr("Successfully loaded %1 accelerometer data samples from %2 files."))
          .arg(newAccelData.size())
          .arg(filenames.size()));

  ui.lineEdit_linear_cached->setText(QString::number(dataVector.size()));
}

void IMUEllipsoidFit::display3D(){
  // 准备 3D 点数据
  QScatterDataArray *dataArray = new QScatterDataArray;
  dataArray->resize(dataVector.size());

  for (int i = 0; i < dataArray->size(); ++i) {
    auto &d = dataVector[i];
    (*dataArray)[i].setPosition(QVector3D(d[0], d[1], d[2]));
  }
  proxy_->resetArray(dataArray);

  // 视角设置
  scatter_->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);

  container_->setMinimumSize(600,800);
  container_->show();
}

void IMUEllipsoidFit::exportData()
{
    if (dataVector.empty()) {
        emit publishNotify(
            GCW::NotifyType::Error,
            tr("No Valid Data"),
            tr("There is no data in the cache."));
        return;
    }

    // 获取 xyz 名称
    QString ax_name = ui.lineEdit_linear_loadName_x->text();
    QString ay_name = ui.lineEdit_linear_loadName_y->text();
    QString az_name = ui.lineEdit_linear_loadName_z->text();

    // 默认保存路径
    QString defaultFileName = QDir(QCoreApplication::applicationDirPath()).filePath("EllipsoidFit.csv");

    // 弹出文件保存对话框
    QString fileName = QFileDialog::getSaveFileName(
        nullptr,
        "Save as CSV",
        defaultFileName,
        "CSV Files (*.csv);;All Files (*)");

    if (fileName.isEmpty())
        return; // 用户取消

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit publishNotify(
            GCW::NotifyType::Error,
            tr("File Write Error"),
            tr("Failed to write to the selected file."));
        return;
    }

    QTextStream out(&file);
    out.setRealNumberPrecision(9); // 可根据 scalar_t 精度选择

    // 写表头
    out << ax_name << "," << ay_name << "," << az_name << "\n";

    // 写数据
    for (const auto& vec : dataVector) {
        out << vec.x() << "," << vec.y() << "," << vec.z() << "\n";
    }

    file.close();
    emit publishNotify(
            GCW::NotifyType::Info,
            "Data written",
            fileName);
        return;
}

/*
参数名	数据类型	含义	单位	UI 展示建议
center	Eigen::Vector3d	椭球中心，即偏置（bias）	m/s²	显示为 3 个数值框（X, Y, Z），表示 IMU 静止时的非零偏移
radii	Eigen::Vector3d	椭球半轴长度，即比例缩放系数	无单位（归一化量）	显示为 3 个进度条或百分比，范围大致在 [0.5 ~ 2.0]
evec_column	Eigen::Matrix3d	主轴方向旋转矩阵（椭球与参考坐标的变换）	无单位	显示为 3x3 矩阵，或3个欧拉角（更直观）
coefficients（可选）	Eigen::Matrix<double, 10, 1>	椭球代数方程系数	-	可选项，仅用于公式展示或高级调试
eval（可选）	Eigen::Vector3d	椭球特征值	-	一般与 radii 相关，不建议独立展示
 */
