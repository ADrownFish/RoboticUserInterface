#include "robotic_user_interface/dashboard/dashboard.h"

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
  ui_actionCommand.lineEdit_pos_x->setLabel("x");
  ui_actionCommand.lineEdit_pos_y->setLabel("y");
  ui_actionCommand.lineEdit_pos_z->setLabel("yaw");
  ui_actionCommand.lineEdit_vel_x->setLabel("x");
  ui_actionCommand.lineEdit_vel_y->setLabel("y");
  ui_actionCommand.lineEdit_vel_z->setLabel("omega");

  ui_host.lineEdit_cpu_package_temp->setLabel("CPU-Pack ℃");
  ui_host.lineEdit_cpu_max_temp->setLabel("CPU-Max ℃");
  ui_host.lineEdit_cpu->setLabel("CPU %");
  ui_host.lineEdit_mem->setLabel("Memory %");
  ui_host.lineEdit_disk->setLabel("Disk %");
  ui_host.lineEdit_bodyTemp->setLabel("BodyTemp ℃");
  ui_host.lineEdit_bodyHumidity->setLabel("Humidity %");

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
  ui_imu.lineEdit_gyro_x->setLabel("x");
  ui_imu.lineEdit_gyro_y->setLabel("y");
  ui_imu.lineEdit_gyro_z->setLabel("z");
  ui_imu.lineEdit_accel_x->setLabel("x");
  ui_imu.lineEdit_accel_y->setLabel("y");
  ui_imu.lineEdit_accel_z->setLabel("z");
  ui_imu.lineEdit_euler_x->setLabel("roll");
  ui_imu.lineEdit_euler_y->setLabel("pitch");
  ui_imu.lineEdit_euler_z->setLabel("yaw");

  ui_odom.label_linear_pos->setText("Position m");
  ui_odom.label_linear_vel->setText("Velocity m/s");

  ui_odom.lineEdit_linear_pos_x->setLabel("x");
  ui_odom.lineEdit_linear_pos_y->setLabel("y");
  ui_odom.lineEdit_linear_pos_z->setLabel("z");
  ui_odom.lineEdit_linear_vel_x->setLabel("x");
  ui_odom.lineEdit_linear_vel_y->setLabel("y");
  ui_odom.lineEdit_linear_vel_z->setLabel("z");

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

  widget_host = new QWWindowWidget(this);
  widget_bms  = new QWWindowWidget(this);
  widget_imu  = new QWWindowWidget(this);
  widget_odom = new QWWindowWidget(this);
  widget_actionCommand = new QWWindowWidget(this);

  widget_host->setBorderRadius(8);
  widget_bms->setBorderRadius(8);
  widget_imu->setBorderRadius(8);
  widget_odom->setBorderRadius(8);
  widget_actionCommand->setBorderRadius(8);

  QColor bgColor(50, 50, 50);
  widget_host->setBackgroundColor(bgColor);
  widget_bms->setBackgroundColor(bgColor);
  widget_imu->setBackgroundColor(bgColor);
  widget_odom->setBackgroundColor(bgColor);
  widget_actionCommand->setBackgroundColor(bgColor);

  ui_host.setupUi(widget_host);
  ui_bms.setupUi (widget_bms);
  ui_imu.setupUi (widget_imu);
  ui_odom.setupUi(widget_odom);
  ui_actionCommand.setupUi(widget_actionCommand);

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

  ui_actionCommand.toggle->setPenWidth(2);
  ui_actionCommand.toggle->setBackgroundColor(QColor(0,0,0,0));
  ui_actionCommand.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

  addChild(widget_imu);
  addChild(widget_bms);
  addChild(widget_host);
  addChild(widget_odom);
  addChild(widget_actionCommand);

  ui_host.toggle->setToggle(true);
  ui_bms.toggle->setToggle(true);
  ui_imu.toggle->setToggle(true);
  ui_odom.toggle->setToggle(true);
  ui_actionCommand.toggle->setToggle(true);

  ui_actionCommand.lineEdit_pos_x->setText(QString::number(0,'f',config_->display.precision));
  ui_actionCommand.lineEdit_pos_y->setText(QString::number(0,'f',config_->display.precision));
  ui_actionCommand.lineEdit_pos_z->setText(QString::number(0,'f',config_->display.precision));

  ui_actionCommand.lineEdit_vel_x->setText(QString::number(config_->action.vel_key[0],  'f',config_->display.precision));
  ui_actionCommand.lineEdit_vel_y->setText(QString::number(config_->action.vel_key[1],  'f',config_->display.precision));
  ui_actionCommand.lineEdit_vel_z->setText(QString::number(config_->action.omega_key[2],'f',config_->display.precision));

}

void Dashboard::setupSignalConnection() {
  QObject::connect(ui_host.toggle,         &QWSwitcher::toggled, ui_host.widget,         &QWidget::setVisible);
  QObject::connect(ui_bms.toggle,          &QWSwitcher::toggled, ui_bms.widget,          &QWidget::setVisible);
  QObject::connect(ui_imu.toggle,          &QWSwitcher::toggled, ui_imu.widget,          &QWidget::setVisible);
  QObject::connect(ui_odom.toggle,         &QWSwitcher::toggled, ui_odom.widget,         &QWidget::setVisible);
  QObject::connect(ui_actionCommand.toggle,&QWSwitcher::toggled, ui_actionCommand.widget,&QWidget::setVisible);

  QObject::connect(ui_actionCommand.button_reset_pos, &QPushButton::clicked, [this]() {
    ui_actionCommand.lineEdit_pos_x->setText(QString::number(observations_->odom.position[0],'f',config_->display.precision));
    ui_actionCommand.lineEdit_pos_y->setText(QString::number(observations_->odom.position[1],'f',config_->display.precision));
    ui_actionCommand.lineEdit_pos_z->setText(QString::number(observations_->imu.eulerAngles[2],'f',config_->display.precision));
  });
  QObject::connect(ui_actionCommand.button_reset_vel, &QPushButton::clicked, [this]() {
    ui_actionCommand.lineEdit_vel_x->setText(QString::number(0.f,'f',config_->display.precision));
    ui_actionCommand.lineEdit_vel_y->setText(QString::number(0.f,'f',config_->display.precision));
    ui_actionCommand.lineEdit_vel_z->setText(QString::number(0.f,'f',config_->display.precision));
  });
  QObject::connect(ui_actionCommand.button_apply_pos, &QPushButton::clicked, [this]() {
    // command_->input.position[0] = ui_actionCommand.lineEdit_pos_x->text().toFloat();
    // command_->input.position[1] = ui_actionCommand.lineEdit_pos_y->text().toFloat();
    // command_->input.eulerAngles[2] = ui_actionCommand.lineEdit_pos_z->text().toFloat();
    command_->input.position[0] = ui_actionCommand.lineEdit_pos_x->text().toFloat();
    command_->input.position[1] = ui_actionCommand.lineEdit_pos_y->text().toFloat();
    command_->input.eulerAngles[2] = ui_actionCommand.lineEdit_pos_z->text().toFloat();
  });
  QObject::connect(ui_actionCommand.button_apply_vel, &QPushButton::clicked, [this]() {
    // command_->input.velocity[0] = ui_actionCommand.lineEdit_vel_x->text().toFloat();
    // command_->input.velocity[1] = ui_actionCommand.lineEdit_vel_y->text().toFloat();
    // command_->input.angularVelocity[2] = ui_actionCommand.lineEdit_vel_z->text().toFloat();
    config_->action.vel_key[0] = ui_actionCommand.lineEdit_vel_x->text().toFloat();
    config_->action.vel_key[1] = ui_actionCommand.lineEdit_vel_y->text().toFloat();
    config_->action.omega_key[2] = ui_actionCommand.lineEdit_vel_z->text().toFloat();
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

  // system
  const auto& system = observations_->system;
  ui_host.lineEdit_cpu_max_temp        ->setText(QString::number(system.cpuCoreMaxTemp));
  ui_host.lineEdit_cpu_package_temp    ->setText(QString::number(system.cpuPackageTemp));
  ui_host.lineEdit_cpu                 ->setText(QString::number(system.cpuUsage));
  ui_host.lineEdit_mem                 ->setText(QString::number(system.memoryUsage));
  ui_host.lineEdit_disk                ->setText(QString::number(system.diskUsage));
  ui_host.lineEdit_bodyTemp            ->setText(QString::number(system.bodyTemp));
  ui_host.lineEdit_bodyHumidity        ->setText(QString::number(system.bodyHumidity));

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
    ui_actionCommand.button_up->setDown(true);
    break;
  case Qt::Key_A:
    command_->input.velocity[1] = config_->action.vel_key[1];
    // cmd.velocity.yVel = configManager_->config.speed_y;
    ui_actionCommand.button_left->setDown(true);
    break;
  case Qt::Key_S:
    command_->input.velocity[0] = -config_->action.vel_key[0];
    // cmd.velocity.xVel = -configManager_->config.speed_x;
    ui_actionCommand.button_down->setDown(true);
    break;
  case Qt::Key_D:
    command_->input.velocity[1] = -config_->action.vel_key[1];
    // cmd.velocity.yVel = -configManager_->config.speed_y;
    ui_actionCommand.button_right->setDown(true);
    break;
  case Qt::Key_I:
    command_->input.eulerAngles[1] = config_->action.euler_key[1];
    ui_actionCommand.button_turn_up->setDown(true);
    break;
  case Qt::Key_K:
    command_->input.eulerAngles[1] = -config_->action.euler_key[1];
    ui_actionCommand.button_turn_down->setDown(true);
    break;
  case Qt::Key_J:
    command_->input.angularVelocity[2] = config_->action.omega_key[2];
    // cmd.velocity.yawVel = configManager_->config.speed_yaw;
    ui_actionCommand.button_turn_left->setDown(true);
    break;
  case Qt::Key_L:
    command_->input.angularVelocity[2] = -config_->action.omega_key[2];
    // cmd.velocity.yawVel = -configManager_->config.speed_yaw;
    ui_actionCommand.button_turn_right->setDown(true);
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
    ui_actionCommand.button_up->setDown(false);
    break;
  case Qt::Key_A:
    command_->input.velocity[1] = 0;
    // cmd.velocity.yVel = configManager_->config.speed_y;
    ui_actionCommand.button_left->setDown(false);
    break;
  case Qt::Key_S:
    command_->input.velocity[0] = 0;
    // cmd.velocity.xVel = -configManager_->config.speed_x;
    ui_actionCommand.button_down->setDown(false);
    break;
  case Qt::Key_D:
    command_->input.velocity[1] = 0;
    // cmd.velocity.yVel = -configManager_->config.speed_y;
    ui_actionCommand.button_right->setDown(false);
    break;
  case Qt::Key_I:
    command_->input.eulerAngles[1] = 0;
    ui_actionCommand.button_turn_up->setDown(false);
    break;
  case Qt::Key_K:
    command_->input.eulerAngles[1] = 0;
    ui_actionCommand.button_turn_down->setDown(false);
    break;
  case Qt::Key_J:
    command_->input.angularVelocity[2] = 0;
    // cmd.velocity.yawVel = configManager_->config.speed_yaw;
    ui_actionCommand.button_turn_left->setDown(false);
    break;
  case Qt::Key_L:
    command_->input.angularVelocity[2] = 0;
    // cmd.velocity.yawVel = -configManager_->config.speed_yaw;
    ui_actionCommand.button_turn_right->setDown(false);
    break;
  default:
    
    break;
  }

  QScrollArea::keyReleaseEvent(event); // 调用父类事件处理
}