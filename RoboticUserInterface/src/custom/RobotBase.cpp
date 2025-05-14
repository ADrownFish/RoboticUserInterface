#include "robotic_user_interface/custom/RobotBase.h"
#include <QCoreApplication>
#include <QDir>
#include <QDate>

void RobotBase::init(int numberOfActuator, int numberOfEndEffector){
  numberOfActuator_ = numberOfActuator;
  numberOfEndEffector_ = numberOfEndEffector;
  
  QString appDir = QCoreApplication::applicationDirPath();
  QString normalizedPath = QDir::cleanPath(appDir);  // 正规化路径
  QString appName = QCoreApplication::applicationName();

  configFilePath = normalizedPath + "/" + appName + "_config.json";
  QString recordFileDir = normalizedPath + "/data/";
  if(!QDir(recordFileDir).exists()){
    QDir().mkdir(recordFileDir);
  }
  QDateTime now = QDateTime::currentDateTime();
  QString timestamp = now.toString("yy-MM-dd_hh-mm-ss-zzz");  // zzz=毫秒 
  recordFilePath = QDir(recordFileDir).filePath(timestamp + ".csv");

  config_ = std::make_unique<Configuration>();
  config_->filePaths.append(std::make_shared<FilePathConfiguration>());
  configManager_ = std::make_shared<ConfigManager>(configFilePath,config_);

  dataSource_ = std::make_shared<DataSource>(numberOfActuator);

  observations_ = std::make_unique<ObservationsBase>(numberOfActuator_, numberOfEndEffector_);
  command_ = std::make_unique<CommandBase>();
  

  QObject::connect(&timer_flush, &QTimer::timeout, [this](){
    catchData();
    displayData();
  });
  timer_flush.start(config_->display.getDt());
}

void RobotBase::saveConfiguration(){
  configManager_->writeConfig();
}

void RobotBase::readConfiguration(){
  configManager_->readConfig();
}

const std::shared_ptr<Configuration> RobotBase::configuration() const{
  return config_;
}

const std::shared_ptr<ObservationsBase> & RobotBase::observations() const{
  return observations_;
}

const std::shared_ptr<CommandBase>& RobotBase::command() const{
  return command_;
}

const std::shared_ptr<DataSource>& RobotBase::dataSource() const {
  return dataSource_;
}

void RobotBase::appendBaseDataSource() {
  scalar_t time = dataSource_->time();

  // 获取顶层节点引用
  const ObjectNode::Ptr& top = dataSource_->topNode();

  // 查找并缓存各个节点
  const ObjectNode::Ptr& base = top->findObjectNode("Base");
  const ObjectNode::Ptr& odom = base->findObjectNode("Odom");

  const ObjectNode::Ptr& position = odom->findObjectNode("position");
  const ObjectNode::Ptr& velocity = odom->findObjectNode("velocity");

  // 获取对应的数据项
  const ObjectData::Ptr& position_x = position->findObjectData("x");
  const ObjectData::Ptr& position_y = position->findObjectData("y");
  const ObjectData::Ptr& position_z = position->findObjectData("z");

  const ObjectData::Ptr& velocity_x = velocity->findObjectData("x");
  const ObjectData::Ptr& velocity_y = velocity->findObjectData("y");
  const ObjectData::Ptr& velocity_z = velocity->findObjectData("z");

  // 获取观测值
  auto& obs_odom = observations_->odom;

  // 直接使用缓存的节点引用进行数据追加
  position_x->appendData(time, obs_odom.position[0]);
  position_y->appendData(time, obs_odom.position[1]);
  position_z->appendData(time, obs_odom.position[2]);

  velocity_x->appendData(time, obs_odom.velocity[0]);
  velocity_y->appendData(time, obs_odom.velocity[1]);
  velocity_z->appendData(time, obs_odom.velocity[2]);

  // 处理 IMU 数据
  const ObjectNode::Ptr& imu = base->findObjectNode("IMU");
  const ObjectNode::Ptr& quat = imu->findObjectNode("quat");
  const ObjectNode::Ptr& euler = imu->findObjectNode("euler");
  const ObjectNode::Ptr& acc = imu->findObjectNode("acc");
  const ObjectNode::Ptr& angVel = imu->findObjectNode("angVel");
  const ObjectNode::Ptr& angAcc = imu->findObjectNode("angAcc");

  const ObjectData::Ptr& quat_w = quat->findObjectData("w");
  const ObjectData::Ptr& quat_x = quat->findObjectData("x");
  const ObjectData::Ptr& quat_y = quat->findObjectData("y");
  const ObjectData::Ptr& quat_z = quat->findObjectData("z");

  const ObjectData::Ptr& euler_x = euler->findObjectData("x");
  const ObjectData::Ptr& euler_y = euler->findObjectData("y");
  const ObjectData::Ptr& euler_z = euler->findObjectData("z");

  const ObjectData::Ptr& acc_x = acc->findObjectData("x");
  const ObjectData::Ptr& acc_y = acc->findObjectData("y");
  const ObjectData::Ptr& acc_z = acc->findObjectData("z");

  const ObjectData::Ptr& angVel_x = angVel->findObjectData("x");
  const ObjectData::Ptr& angVel_y = angVel->findObjectData("y");
  const ObjectData::Ptr& angVel_z = angVel->findObjectData("z");

  const ObjectData::Ptr& angAcc_x = angAcc->findObjectData("x");
  const ObjectData::Ptr& angAcc_y = angAcc->findObjectData("y");
  const ObjectData::Ptr& angAcc_z = angAcc->findObjectData("z");

  // 获取观测值
  auto& obs_imu = observations_->imu;

  // 直接使用缓存的节点引用进行数据追加
  quat_w->appendData(time, obs_imu.quat[0]);
  quat_x->appendData(time, obs_imu.quat[1]);
  quat_y->appendData(time, obs_imu.quat[2]);
  quat_z->appendData(time, obs_imu.quat[3]);

  euler_x->appendData(time, obs_imu.eulerAngles[0]);
  euler_y->appendData(time, obs_imu.eulerAngles[1]);
  euler_z->appendData(time, obs_imu.eulerAngles[2]);

  acc_x->appendData(time, obs_imu.acceleration[0]);
  acc_y->appendData(time, obs_imu.acceleration[1]);
  acc_z->appendData(time, obs_imu.acceleration[2]);

  angVel_x->appendData(time, obs_imu.angularVelocity[0]);
  angVel_y->appendData(time, obs_imu.angularVelocity[1]);
  angVel_z->appendData(time, obs_imu.angularVelocity[2]);

  angAcc_x->appendData(time, obs_imu.angularAcceleration[0]);
  angAcc_y->appendData(time, obs_imu.angularAcceleration[1]);
  angAcc_z->appendData(time, obs_imu.angularAcceleration[2]);


  // 处理 Battery 数据
  const ObjectNode::Ptr& battery = base->findObjectNode("Battery");

  const ObjectData::Ptr& battery_voltage = battery->findObjectData("voltage");
  const ObjectData::Ptr& battery_current = battery->findObjectData("current");
  const ObjectData::Ptr& battery_soc = battery->findObjectData("soc");
  const ObjectData::Ptr& battery_temp = battery->findObjectData("temp");

  // 获取观测值
  auto& obs_battery = observations_->battery;

  // 直接使用缓存的节点引用进行数据追加
  battery_voltage->appendData(time, obs_battery.voltage);
  battery_current->appendData(time, obs_battery.current);
  battery_soc->appendData(time, obs_battery.soc);
  battery_temp->appendData(time, obs_battery.temp);

  // 处理 System 数据
  const ObjectNode::Ptr& system = base->findObjectNode("System");

  const ObjectData::Ptr& system_cpuUsage = system->findObjectData("cpuUsage");
  const ObjectData::Ptr& system_memoryUsage = system->findObjectData("memoryUsage");
  const ObjectData::Ptr& system_diskUsage = system->findObjectData("diskUsage");
  const ObjectData::Ptr& system_cpuCoreTemp = system->findObjectData("cpuCoreTemp");
  const ObjectData::Ptr& system_cpuPackageTemp = system->findObjectData("cpuPackageTemp");

  // 获取观测值
  auto& obs_system = observations_->system;

  // 直接使用缓存的节点引用进行数据追加
  system_cpuUsage->appendData(time, obs_system.cpuUsage);
  system_memoryUsage->appendData(time, obs_system.memoryUsage);
  system_diskUsage->appendData(time, obs_system.diskUsage);
  system_cpuCoreTemp->appendData(time, obs_system.cpuCoreMaxTemp);
  system_cpuPackageTemp->appendData(time, obs_system.cpuPackageTemp);

  auto &obs_act = observations_->actuator;
  const ObjectNode::Ptr& actuators = base->findObjectNode("Actuators");

  for (int i = 0; i < numberOfActuator_; i++) {
    const ObjectNode::Ptr& actuators_x = actuators->findObjectNode(QString("Actuator_%1").arg(i));
    actuators_x->findObjectData("state")         ->appendData(time, obs_act[i].state);
    actuators_x->findObjectData("pos")           ->appendData(time, obs_act[i].pos);
    actuators_x->findObjectData("vel")            ->appendData(time, obs_act[i].vel);
    actuators_x->findObjectData("torque")      ->appendData(time, obs_act[i].torque);
    actuators_x->findObjectData("voltage")     ->appendData(time, obs_act[i].voltage);
    actuators_x->findObjectData("current")     ->appendData(time, obs_act[i].current);
    actuators_x->findObjectData("power")       ->appendData(time, obs_act[i].power);
    actuators_x->findObjectData("temp_m")    ->appendData(time, obs_act[i].temperature);
    actuators_x->findObjectData("Temp_d")    ->appendData(time, obs_act[i].driverTemperature);
  }
}


void RobotBase::setEnabledRecord(bool enabled){
  record_ = enabled;
  if(dataRecorder_ == nullptr){
    dataRecorder_ = new robot::AsyncDataRecorder(recordFilePath,this);
    QStringList col;
    col 
    // odom
    << "odom_position_x" << "odom_position_y" << "odom_position_z"
    << "odom_velocity_x" << "odom_velocity_y" << "odom_velocity_z"

    // imu
    << "imu_quat_w" << "imu_quat_x" << "imu_quat_y" << "imu_quat_z"
    << "imu_euler_x" << "imu_euler_y" << "imu_euler_z"
    << "imu_linear_acc_x" << "imu_linear_acc_y" << "imu_linear_acc_z"
    << "imu_angular_vel_x" << "imu_angular_vel_y" << "imu_angular_vel_z"
    << "imu_angular_acc_x" << "imu_angular_acc_y" << "imu_angular_acc_z"

    // system
    << "sys_status" << "sys_cpu_CoreMaxTemp" << "sys_cpuPackageTemp" << "sys_cpuUsage" << "sys_memoryUsage" << "sys_diskUsage"

    // battery
    << "bms_status" << "bms_soc" << "bms_current" << "bms_voltage" << "bms_cycle" << "bms_temp";

    for (size_t i = 0; i < numberOfActuator_; i++){
      col 
      << QString("actuator%1_state").arg(i) 
      << QString("actuator%1_pos").arg(i) 
      << QString("actuator%1_vel").arg(i) 
      << QString("actuator%1_tau").arg(i) 
      << QString("actuator%1_current").arg(i) 
      << QString("actuator%1_voltage").arg(i) 
      << QString("actuator%1_power").arg(i)
      << QString("actuator%1_temp").arg(i)
      << QString("actuator%1_driverTemp").arg(i);
    }
    
    dataRecorder_->init(1000,col);
  }
}

void RobotBase::recordDataOnce(){
  if(dataRecorder_ == nullptr)
    return;
  if(!record_){
    return;
  }

  const auto & odom = observations_->odom;
  const auto & imu = observations_->imu;
  const auto & sys = observations_->system;
  const auto & bat = observations_->battery;
  const auto & act = observations_->actuator;

  QVector<scalar_t> data;
  data 
  << odom.position[0] << odom.position[1] << odom.position[2]
  << odom.velocity[0] << odom.velocity[1] << odom.velocity[2]

  << imu.quat[0] << imu.quat[1] << imu.quat[2] << imu.quat[3]
  << imu.eulerAngles[0] << imu.eulerAngles[1] << imu.eulerAngles[2]
  << imu.acceleration[0] << imu.acceleration[1] << imu.acceleration[2]
  << imu.angularVelocity[0] << imu.angularVelocity[1] << imu.angularVelocity[2]
  << imu.angularAcceleration[0] << imu.angularAcceleration[1] << imu.angularAcceleration[2]

  << (int)sys.status << (int)sys.cpuCoreMaxTemp << (int)sys.cpuPackageTemp << (int)sys.cpuUsage << (int)sys.memoryUsage << (int)sys.diskUsage

  << (int)bat.status << (int)bat.soc << bat.current << bat.voltage << bat.cycle << bat.temp;


  for (size_t i = 0; i < numberOfActuator_; i++){
    data 
    << act[i].state
    << act[i].pos
    << act[i].vel
    << act[i].torque
    << act[i].current
    << act[i].voltage
    << act[i].power
    << act[i].temperature
    << act[i].driverTemperature;
  }

  dataRecorder_->submitRecord(data);
}