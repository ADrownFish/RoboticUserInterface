#include "robotic_user_interface/dashboard/dashboard.h"

#include <QSvgRenderer>
#include <QtCore>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QSysInfo>
#include <QtSerialPort/QSerialPortInfo>

static QPair<int, QString> detectFirewallStatus() {
#if defined(Q_OS_WIN)
    QProcess p;
    p.start("netsh", QStringList() << "advfirewall" << "show" << "allprofiles" << "state");
    if (!p.waitForFinished(1500)) return {0, "netsh timeout/permission"};
    QString out = QString::fromLocal8Bit(p.readAllStandardOutput());
    if (out.contains("State ON", Qt::CaseInsensitive)) return {1, "enabled"};
    if (out.contains("State OFF", Qt::CaseInsensitive)) return {2, "disabled"};
    return {0, "unknown"};
#elif defined(Q_OS_MAC)
    QProcess p;
    p.start("defaults", QStringList() << "read" << "/Library/Preferences/com.apple.alf" << "globalstate");
    if (!p.waitForFinished(1000)) return {0, "permission/timeout"};
    bool ok = false;
    int v = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed().toInt(&ok);
    if (!ok) return {0, "unknown"};
    return v == 0 ? QPair<int,QString>{2,"disabled"} : QPair<int,QString>{1, "enabled"};
#else
    QProcess p;
    p.start("ufw", QStringList() << "status");
    if (p.waitForFinished(1000)) {
        QString out = QString::fromLocal8Bit(p.readAllStandardOutput() + p.readAllStandardError());
        if (out.contains("Status: active", Qt::CaseInsensitive)) return {1,"ufw active"};
        if (out.contains("Status: inactive", Qt::CaseInsensitive)) return {2,"ufw inactive"};
    }
    QProcess p2;
    p2.start("systemctl", QStringList() << "is-active" << "firewalld");
    if (p2.waitForFinished(800)) {
        QString s = QString::fromLocal8Bit(p2.readAllStandardOutput()).trimmed();
        if (s == "active") return {1,"firewalld active"};
        if (s == "inactive") return {2,"firewalld inactive"};
    }
    return {0,"unknown"};
#endif
}

QString getSystemInfo() {
    QString out;
    // Basic
    out += QString("********** System **********\n");
    out += QString("Hostname: %1\n").arg(QHostInfo::localHostName());
    out += QString("OS: %1 %2\n").arg(QSysInfo::productType(), QSysInfo::productVersion());
    out += QString("Kernel/Version: %1 / %2\n").arg(QSysInfo::kernelType(), QSysInfo::kernelVersion());
    out += QString("CPU Arch: %1\n").arg(QSysInfo::currentCpuArchitecture());
    out += QString("Logical Cores: %1\n").arg(QThread::idealThreadCount());

    // Network: IPv4 + MAC (only UP, non-loopback)
    out += QString("\n********** Network **********\n");
    bool anyNet = false;
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (!iface.isValid()) continue;
        if (!iface.flags().testFlag(QNetworkInterface::IsUp)) continue;
        if (iface.flags().testFlag(QNetworkInterface::IsLoopBack)) continue;
        QStringList ipv4s;
        for (const QNetworkAddressEntry &e : iface.addressEntries()) {
            if (e.ip().protocol() == QAbstractSocket::IPv4Protocol && !e.ip().isNull())
                ipv4s << e.ip().toString();
        }
        if (!ipv4s.isEmpty()) {
            anyNet = true;
            out += QString("  %1 (MAC=%2)\n").arg(iface.humanReadableName(), iface.hardwareAddress());
            for (const QString &ip : ipv4s) out += QString("    - %1\n").arg(ip);
        }
    }
    if (!anyNet) out += "  None\n";

    // Serial ports
    out += QString("\n********** Serial Ports **********\n");
    auto ports = QSerialPortInfo::availablePorts();
    if (ports.isEmpty()) {
        out += "  None\n";
    } else {
        for (const QSerialPortInfo &p : ports) {
            out += QString("  %1 %2").arg(p.portName(), p.description());
            if (!p.manufacturer().isEmpty()) out += QString(" (%1)").arg(p.manufacturer());
            out += "\n";
        }
    }

    // Firewall
    out += QString("\n********** Firewall **********\n");
    auto fw = detectFirewallStatus();
    out += fw.second;

    return out;
}


void setSvgToLabel(QLabel* label, const QString& svgPath){
  QSvgRenderer renderer(svgPath);
  QPixmap pixmap(label->size());
  pixmap.fill(Qt::transparent);
  
  QPainter painter(&pixmap);
  renderer.render(&painter);
  
  label->setPixmap(pixmap);
}

Dashboard::Dashboard(QWidget *parent)
  : QScrollArea(parent) {
}

Dashboard::~Dashboard() {

}

void Dashboard::addChild(QWidget *child) {
  layout_->addWidget(child);
}

void Dashboard::addChild(QLayoutItem* child){
  layout_->addItem(child);
}

void Dashboard::initLabel(){
  ui_virtualJoystick.lineEdit_pos_x->setLabel("x");
  ui_virtualJoystick.lineEdit_pos_y->setLabel("y");
  ui_virtualJoystick.lineEdit_pos_z->setLabel("yaw");
  ui_virtualJoystick.lineEdit_vel_x->setLabel("x");
  ui_virtualJoystick.lineEdit_vel_y->setLabel("y");
  ui_virtualJoystick.lineEdit_vel_z->setLabel("omega");
  ui_virtualJoystick.button_mode->setIcon(QIcon(":/svg/svg/arrow-down2.svg"));

  ui_host.lineEdit_cpu_temp->setLabel("CPU Temp ℃");
  ui_host.lineEdit_cpu_ferq->setLabel("CPU Freq GHz");
  ui_host.lineEdit_cpu->setLabel("CPU %");
  ui_host.lineEdit_mem->setLabel("Memory %");
  ui_host.lineEdit_disk->setLabel("Disk %");

  ui_sensor.lineEdit_temp->setLabel("Temp ℃");
  ui_sensor.lineEdit_humidity->setLabel("Humidity %");

  ui_bms.lineEdit_vol->setLabel("Voltage V");
  ui_bms.lineEdit_current->setLabel("Current A"); 
  ui_bms.lineEdit_power->setLabel("Power W");
  ui_bms.lineEdit_soc->setLabel("Level %");
  ui_bms.lineEdit_temp->setLabel("Temp ℃");
  ui_bms.lineEdit_state->setLabel("State");
  ui_bms.lineEdit_cycel->setLabel("Cycel");

  ui_imu.label_gyro ->setText(QString("Gyro %1/s").arg(config_->display.getDegRad()));
  ui_imu.label_accel->setText("Accel m/s2");
  ui_imu.label_euler->setText(QString("Euler %1").arg(config_->display.getDegRad()));
  ui_imu.label_quat->setText(QString("Quat"));
  ui_imu.lineEdit_gyro_x->setLabel("x");
  ui_imu.lineEdit_gyro_y->setLabel("y");
  ui_imu.lineEdit_gyro_z->setLabel("z");
  ui_imu.lineEdit_accel_x->setLabel("x");
  ui_imu.lineEdit_accel_y->setLabel("y");
  ui_imu.lineEdit_accel_z->setLabel("z");
  ui_imu.lineEdit_euler_x->setLabel("roll");
  ui_imu.lineEdit_euler_y->setLabel("pitch");
  ui_imu.lineEdit_euler_z->setLabel("yaw");
  ui_imu.lineEdit_quat_w->setLabel("w");
  ui_imu.lineEdit_quat_x->setLabel("x");
  ui_imu.lineEdit_quat_y->setLabel("y");
  ui_imu.lineEdit_quat_z->setLabel("z");

  ui_odom.label_linear_pos->setText(tr("Position m"));
  ui_odom.label_linear_vel->setText(tr("Velocity m/s"));
  ui_odom.label_linear_mileage->setText(tr("Mileage m"));

  ui_odom.lineEdit_linear_pos_x->setLabel("x");
  ui_odom.lineEdit_linear_pos_y->setLabel("y");
  ui_odom.lineEdit_linear_pos_z->setLabel("z");
  ui_odom.lineEdit_linear_vel_x->setLabel("x");
  ui_odom.lineEdit_linear_vel_y->setLabel("y");
  ui_odom.lineEdit_linear_vel_z->setLabel("z");
  ui_odom.lineEdit_linear_mileage->setLabel("Total");

  timer_flush_.start(config_->display.getDt());
} 

void Dashboard::setConfiguration(std::shared_ptr<Configuration> config){
  config_ = config;
}

void Dashboard::setObservations(std::shared_ptr<ObservationsBase> obs){
  observations_ = obs;
}

void Dashboard::setCommand(std::shared_ptr<CommandBase> cmd){
  command_ = cmd;
}

void Dashboard::setActuators(const QStringList &title){

  widget_actuator = new ActuatorDisplay(this);
  widget_actuator->setConfiguration(config_);
  widget_actuator->init();
  widget_actuator->setActuators(title);
  auto acts = widget_actuator->actuatorUnits();
  for(auto &it : acts){
    setSvgToLabel(it->ui_actuator.label_res, ":/svg/svg/servo.svg");
  }
  addChild(widget_actuator);
}

void Dashboard::resizeEvent(QResizeEvent* event){
  widget_actuator -> setMinimumWidth(
    event->size().width() - 2 * config_->display.contentsMargins);
  // 调用基类的 resizeEvent，确保正常处理事件
  QWidget::resizeEvent(event);
}

void Dashboard::setupWidgetsControls(){
  layout_ = new QFlowLayout(this);
  layout_->setContentsMargins(
  config_->display.contentsMargins,
  config_->display.contentsMargins,
  config_->display.contentsMargins,
  config_->display.contentsMargins);
  layout_->setHorizontalSpacing(config_->display.spacing);
  layout_->setVerticalSpacing(config_->display.spacing);

  QWidget *layoutWidget = new QWidget(this);
  layoutWidget->setLayout(layout_);
  setWidget(layoutWidget);  // 设置 QScrollArea 的滚动区域
  setWidgetResizable(true);  // 内容区域大小可随滚动区域调整

  widget_sysInfo = new QWWindowWidget(this);
  widget_sensor  = new QWWindowWidget(this);
  widget_host = new QWWindowWidget(this);
  widget_bms  = new QWWindowWidget(this);
  widget_imu  = new QWWindowWidget(this);
  widget_odom = new QWWindowWidget(this);
  widget_joy = new QWWindowWidget(this);

  widget_sysInfo->setBorderRadius(8);
  widget_sensor->setBorderRadius(8);
  widget_host->setBorderRadius(8);
  widget_bms->setBorderRadius(8);
  widget_imu->setBorderRadius(8);
  widget_odom->setBorderRadius(8);
  widget_joy->setBorderRadius(8);

  QColor bgColor(50, 50, 50);
  widget_sysInfo->setBackgroundColor(bgColor);
  widget_sensor->setBackgroundColor(bgColor);
  widget_host->setBackgroundColor(bgColor);
  widget_bms->setBackgroundColor(bgColor);
  widget_imu->setBackgroundColor(bgColor);
  widget_odom->setBackgroundColor(bgColor);
  widget_joy->setBackgroundColor(bgColor);


  ui_systemInfo.setupUi(widget_sysInfo);
  ui_sensor.setupUi(widget_sensor);
  ui_host.setupUi(widget_host);
  ui_bms.setupUi (widget_bms);
  ui_imu.setupUi (widget_imu);
  ui_odom.setupUi(widget_odom);
  ui_virtualJoystick.setupUi(widget_joy);

  ui_systemInfo.toggle->setPenWidth(2);
  ui_systemInfo.toggle->setBackgroundColor(QColor(0,0,0,0));
  ui_systemInfo.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

  ui_sensor.toggle->setPenWidth(2);
  ui_sensor.toggle->setBackgroundColor(QColor(0,0,0,0));
  ui_sensor.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

  ui_host.toggle->setPenWidth(2);
  ui_host.toggle->setBackgroundColor(QColor(0,0,0,0));
  ui_host.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

  ui_bms.toggle->setPenWidth(2);
  ui_bms.toggle->setBackgroundColor(QColor(0,0,0,0));
  ui_bms.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

  ui_imu.toggle->setPenWidth(2);
  ui_imu.toggle->setBackgroundColor(QColor(0,0,0,0));
  ui_imu.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

  ui_odom.toggle->setPenWidth(2);
  ui_odom.toggle->setBackgroundColor(QColor(0,0,0,0));
  ui_odom.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

  ui_odom.toggle->setPenWidth(2);
  ui_odom.toggle->setBackgroundColor(QColor(0,0,0,0));
  ui_odom.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

  ui_virtualJoystick.toggle->setPenWidth(2);
  ui_virtualJoystick.toggle->setBackgroundColor(QColor(0,0,0,0));
  ui_virtualJoystick.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

  addChild(widget_sysInfo);
  addChild(widget_sensor);
  addChild(widget_imu);
  addChild(widget_bms);
  addChild(widget_host);
  addChild(widget_odom);
  addChild(widget_joy);

  ui_systemInfo.toggle->setToggle(true);
  ui_sensor.toggle->setToggle(true);
  ui_host.toggle->setToggle(true);
  ui_bms.toggle->setToggle(true);
  ui_imu.toggle->setToggle(true);
  ui_odom.toggle->setToggle(true);
  ui_virtualJoystick.toggle->setToggle(true);

  // resource
  
  setSvgToLabel(ui_systemInfo.label_res,      ":/svg/svg/debug.svg");
  setSvgToLabel(ui_sensor.label_res,          ":/svg/svg/temperature.svg");  
  setSvgToLabel(ui_host.label_res,            ":/svg/svg/operation.svg");
  setSvgToLabel(ui_bms.label_res,             ":/svg/svg/bms.svg");
  setSvgToLabel(ui_imu.label_res,             ":/svg/svg/gyro.svg");
  setSvgToLabel(ui_odom.label_res,            ":/svg/svg/odom.svg");
  setSvgToLabel(ui_virtualJoystick.label_res, ":/svg/svg/joy.svg");

  ui_virtualJoystick.lineEdit_pos_x->setText(QString::number(config_->action.pos_key[0],'f',config_->display.precision));
  ui_virtualJoystick.lineEdit_pos_y->setText(QString::number(config_->action.pos_key[1],'f',config_->display.precision));
  ui_virtualJoystick.lineEdit_pos_z->setText(QString::number(config_->action.pos_key[2],'f',config_->display.precision));

  ui_virtualJoystick.lineEdit_vel_x->setText(QString::number(config_->action.vel_key[0],  'f',config_->display.precision));
  ui_virtualJoystick.lineEdit_vel_y->setText(QString::number(config_->action.vel_key[1],  'f',config_->display.precision));
  ui_virtualJoystick.lineEdit_vel_z->setText(QString::number(config_->action.omega_key[2],'f',config_->display.precision));

}

void Dashboard::setupSignalConnection() {
  QObject::connect(ui_systemInfo.toggle,   &QWSwitcher::toggled, ui_systemInfo.widget,   &QWidget::setVisible);
  QObject::connect(ui_sensor.toggle,       &QWSwitcher::toggled, ui_sensor.widget,          &QWidget::setVisible);
  QObject::connect(ui_host.toggle,         &QWSwitcher::toggled, ui_host.widget,         &QWidget::setVisible);
  QObject::connect(ui_bms.toggle,          &QWSwitcher::toggled, ui_bms.widget,          &QWidget::setVisible);
  QObject::connect(ui_imu.toggle,          &QWSwitcher::toggled, ui_imu.widget,          &QWidget::setVisible);
  QObject::connect(ui_odom.toggle,         &QWSwitcher::toggled, ui_odom.widget,         &QWidget::setVisible);

  QObject::connect(ui_virtualJoystick.toggle,&QWSwitcher::toggled, ui_virtualJoystick.stackedWidget,&QWidget::setVisible);
  QObject::connect(ui_virtualJoystick.button_mode,&QPushButton::clicked, this, [this](){
    auto &sw = ui_virtualJoystick.stackedWidget;
    uint32_t index  = (sw->currentIndex() + 1) % sw->count();
    sw->setCurrentIndex(index);
  });

  QObject::connect(ui_systemInfo.button_flush,&QPushButton::clicked, this, [this](){
    auto &st = ui_systemInfo.textEdit;

    QString text = QString("<span style=\"color:%1;\">%2</span>").arg("#DDF0FF", getSystemInfo().toHtmlEscaped());
    text.replace("\n", "<br>");  // 将换行符转换为 HTML 换行
    st->append(text);
  });

  QObject::connect(ui_virtualJoystick.button_reset_pos, &QPushButton::clicked, [this]() {
    ui_virtualJoystick.lineEdit_pos_x->setText(QString::number(observations_->odom.position[0],'f',config_->display.precision));
    ui_virtualJoystick.lineEdit_pos_y->setText(QString::number(observations_->odom.position[1],'f',config_->display.precision));
    ui_virtualJoystick.lineEdit_pos_z->setText(QString::number(observations_->imu.eulerAngles[2],'f',config_->display.precision));
  });
  QObject::connect(ui_virtualJoystick.button_reset_vel, &QPushButton::clicked, [this]() {
    ui_virtualJoystick.lineEdit_vel_x->setText(QString::number(0.f,'f',config_->display.precision));
    ui_virtualJoystick.lineEdit_vel_y->setText(QString::number(0.f,'f',config_->display.precision));
    ui_virtualJoystick.lineEdit_vel_z->setText(QString::number(0.f,'f',config_->display.precision));
  });
  QObject::connect(ui_virtualJoystick.button_apply_pos, &QPushButton::clicked, [this]() {
    // command_->input.position[0] = ui_virtualJoystick.lineEdit_pos_x->text().toFloat();
    // command_->input.position[1] = ui_virtualJoystick.lineEdit_pos_y->text().toFloat();
    // command_->input.eulerAngles[2] = ui_virtualJoystick.lineEdit_pos_z->text().toFloat();
    command_->input.position[0] = ui_virtualJoystick.lineEdit_pos_x->text().toFloat();
    command_->input.position[1] = ui_virtualJoystick.lineEdit_pos_y->text().toFloat();
    command_->input.eulerAngles[2] = ui_virtualJoystick.lineEdit_pos_z->text().toFloat();
  });
  QObject::connect(ui_virtualJoystick.button_apply_vel, &QPushButton::clicked, [this]() {
    // command_->input.velocity[0] = ui_virtualJoystick.lineEdit_vel_x->text().toFloat();
    // command_->input.velocity[1] = ui_virtualJoystick.lineEdit_vel_y->text().toFloat();
    // command_->input.angularVelocity[2] = ui_virtualJoystick.lineEdit_vel_z->text().toFloat();
    config_->action.vel_key[0] = ui_virtualJoystick.lineEdit_vel_x->text().toFloat();
    config_->action.vel_key[1] = ui_virtualJoystick.lineEdit_vel_y->text().toFloat();
    config_->action.omega_key[2] = ui_virtualJoystick.lineEdit_vel_z->text().toFloat();
  });

  QObject::connect(&timer_flush_, &QTimer::timeout, this, &Dashboard::flush);
}

void Dashboard::addChilds(const QList<QWidget *> &list){
  for(QWidget *widget : list){
    addChild(widget);
  }
}

void Dashboard::init(){

  setupWidgetsControls();
  setupSignalConnection();
}

void Dashboard::flushConfiguration(){
  initLabel();
  widget_actuator->flushConfiguration();
}

void Dashboard::flush(){

  const auto& precision       = config_->display.precision;
  const auto& angleScale     = config_->display.getAngleScale();
  // const auto& degRad          = config_->display.getDegRad();
  // const auto& degRadPerSecond = config_->display.getDegRadPerSecond();

  // odom
  const auto& odom = observations_->odom;
  ui_odom.lineEdit_linear_pos_x->setText(QString::number(odom.position[0],'f',precision));
  ui_odom.lineEdit_linear_pos_y->setText(QString::number(odom.position[1],'f',precision));
  ui_odom.lineEdit_linear_pos_z->setText(QString::number(odom.position[2],'f',precision));

  ui_odom.lineEdit_linear_vel_x->setText(QString::number(odom.velocity[0],'f',precision));
  ui_odom.lineEdit_linear_vel_y->setText(QString::number(odom.velocity[1],'f',precision));
  ui_odom.lineEdit_linear_vel_z->setText(QString::number(odom.velocity[2],'f',precision));
  ui_odom.lineEdit_linear_mileage->setText(QString::number(odom.mileage,  'f',precision));

  // imu
  const auto& imu = observations_->imu;
  ui_imu.lineEdit_euler_x->setText(QString::number(imu.eulerAngles[0] * angleScale,'f',precision));
  ui_imu.lineEdit_euler_y->setText(QString::number(imu.eulerAngles[1] * angleScale,'f',precision));
  ui_imu.lineEdit_euler_z->setText(QString::number(imu.eulerAngles[2] * angleScale,'f',precision));

  ui_imu.lineEdit_gyro_x->setText(QString::number(imu.angularVelocity[0] * angleScale,'f',precision));
  ui_imu.lineEdit_gyro_y->setText(QString::number(imu.angularVelocity[1] * angleScale,'f',precision));
  ui_imu.lineEdit_gyro_z->setText(QString::number(imu.angularVelocity[2] * angleScale,'f',precision));
  
  ui_imu.lineEdit_accel_x->setText(QString::number(imu.acceleration[0],'f',precision));
  ui_imu.lineEdit_accel_y->setText(QString::number(imu.acceleration[1],'f',precision));
  ui_imu.lineEdit_accel_z->setText(QString::number(imu.acceleration[2],'f',precision));

  ui_imu.lineEdit_quat_w->setText(QString::number(imu.quat[0],'f',precision));
  ui_imu.lineEdit_quat_x->setText(QString::number(imu.quat[1],'f',precision));
  ui_imu.lineEdit_quat_y->setText(QString::number(imu.quat[2],'f',precision));
  ui_imu.lineEdit_quat_z->setText(QString::number(imu.quat[3],'f',precision));

  // system
  const auto& system = observations_->system;
  ui_host.lineEdit_cpu_temp ->setText(QString::number(system.cpuTemp));
  ui_host.lineEdit_cpu_ferq ->setText(QString::number(system.cpuFerq));
  ui_host.lineEdit_cpu      ->setText(QString::number(system.cpuUsage));
  ui_host.lineEdit_mem      ->setText(QString::number(system.memoryUsage));
  ui_host.lineEdit_disk     ->setText(QString::number(system.diskUsage));

  // sensor
  const auto& sensor = observations_->sensor;
  ui_sensor.lineEdit_temp     ->setText(QString::number(sensor.temp));
  ui_sensor.lineEdit_humidity ->setText(QString::number(sensor.humidity));

  // battery
  const auto& battery = observations_->battery;
  ui_bms.lineEdit_state   ->setText(QString::number(battery.status));
  ui_bms.lineEdit_soc     ->setText(QString::number(battery.soc));
  ui_bms.lineEdit_current ->setText(QString::number(battery.current,'f',precision));
  ui_bms.lineEdit_vol     ->setText(QString::number(battery.voltage,'f',precision));
  ui_bms.lineEdit_temp    ->setText(QString::number(battery.temp));
  ui_bms.lineEdit_cycel   ->setText(QString::number(battery.cycle));
  ui_bms.lineEdit_power   ->setText(QString::number(battery.current * observations_->battery.voltage));

  widget_actuator->updateActuators(observations_->actuator);
}

void Dashboard::keyPressEventGlobal(QKeyEvent *event) {
    // 监听按下的按键
  switch (event->key()) {
  case Qt::Key_W:
    command_->input.velocity[0] = config_->action.vel_key[0];
    ui_virtualJoystick.button_up->setDown(true);
    break;
  case Qt::Key_A:
    command_->input.velocity[1] = config_->action.vel_key[1];
    // cmd.velocity.yVel = configManager_->config.speed_y;
    ui_virtualJoystick.button_left->setDown(true);
    break;
  case Qt::Key_S:
    command_->input.velocity[0] = -config_->action.vel_key[0];
    // cmd.velocity.xVel = -configManager_->config.speed_x;
    ui_virtualJoystick.button_down->setDown(true);
    break;
  case Qt::Key_D:
    command_->input.velocity[1] = -config_->action.vel_key[1];
    // cmd.velocity.yVel = -configManager_->config.speed_y;
    ui_virtualJoystick.button_right->setDown(true);
    break;
  case Qt::Key_I:
    command_->input.eulerAngles[1] = config_->action.euler_key[1];
    ui_virtualJoystick.button_turn_up->setDown(true);
    break;
  case Qt::Key_K:
    command_->input.eulerAngles[1] = -config_->action.euler_key[1];
    ui_virtualJoystick.button_turn_down->setDown(true);
    break;
  case Qt::Key_J:
    command_->input.angularVelocity[2] = config_->action.omega_key[2];
    // cmd.velocity.yawVel = configManager_->config.speed_yaw;
    ui_virtualJoystick.button_turn_left->setDown(true);
    break;
  case Qt::Key_L:
    command_->input.angularVelocity[2] = -config_->action.omega_key[2];
    // cmd.velocity.yawVel = -configManager_->config.speed_yaw;
    ui_virtualJoystick.button_turn_right->setDown(true);
    break;
  default:
    
    break;
  }
  QScrollArea::keyPressEvent(event); // 调用父类事件处理
}

void Dashboard::keyReleaseEventGlobal(QKeyEvent *event) {

  // 监听按键释放
  switch (event->key()) {
  case Qt::Key_W:
    command_->input.velocity[0] = 0;
    ui_virtualJoystick.button_up->setDown(false);
    break;
  case Qt::Key_A:
    command_->input.velocity[1] = 0;
    // cmd.velocity.yVel = configManager_->config.speed_y;
    ui_virtualJoystick.button_left->setDown(false);
    break;
  case Qt::Key_S:
    command_->input.velocity[0] = 0;
    // cmd.velocity.xVel = -configManager_->config.speed_x;
    ui_virtualJoystick.button_down->setDown(false);
    break;
  case Qt::Key_D:
    command_->input.velocity[1] = 0;
    // cmd.velocity.yVel = -configManager_->config.speed_y;
    ui_virtualJoystick.button_right->setDown(false);
    break;
  case Qt::Key_I:
    command_->input.eulerAngles[1] = 0;
    ui_virtualJoystick.button_turn_up->setDown(false);
    break;
  case Qt::Key_K:
    command_->input.eulerAngles[1] = 0;
    ui_virtualJoystick.button_turn_down->setDown(false);
    break;
  case Qt::Key_J:
    command_->input.angularVelocity[2] = 0;
    // cmd.velocity.yawVel = configManager_->config.speed_yaw;
    ui_virtualJoystick.button_turn_left->setDown(false);
    break;
  case Qt::Key_L:
    command_->input.angularVelocity[2] = 0;
    // cmd.velocity.yawVel = -configManager_->config.speed_yaw;
    ui_virtualJoystick.button_turn_right->setDown(false);
    break;
  default:
    
    break;
  }

  QScrollArea::keyReleaseEvent(event); // 调用父类事件处理
}
