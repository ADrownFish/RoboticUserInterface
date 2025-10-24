#include "robotic_user_interface/custom/RobotBase.h"
#include <QCoreApplication>
#include <QDir>
#include <QDate>
#include <QPluginLoader>

#include "FluControls/FluMessageBox.h"

void RobotBase::setDataAllocator(const QPointer<DataAllocator>& p){
  dataAllocator_ = p;
}

void RobotBase::init(int numberOfActuator, int numberOfEndEffector){
  numberOfActuator_ = numberOfActuator;
  numberOfEndEffector_ = numberOfEndEffector;
  
  auto cfg = ConfigManager::getInitialConfiguration();
  configFilePath = cfg.configFile;
  
  QString recordFileDir = cfg.configDir + "/data/";
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
    readyRead();
    displayData();
  });

  timer_flush.start(config_->display.getDt());
  dataSource_->topNode()->setTimeWindow(config_->plot.cacheDuration);
}

void RobotBase::saveConfiguration() const{
  configManager_->writeConfig();
}

void RobotBase::readConfiguration() const{
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

const QString& RobotBase::getRecordFilePath()
{
  return recordFilePath;
}

QDialog::DialogCode RobotBase::displayMessageDialog(const QString& title, const QString text)
{
  FluMessageBox messageBox(title, text, topWidget_);
  return (QDialog::DialogCode)messageBox.exec();
}

void RobotBase::updateDataSource(scalar_t time) {
  time = time - dataSource_->startTime();

  // 获取顶层节点引用
  const ObjectNode::Ptr& top = dataSource_->topNode();
  const ObjectNode::Ptr& base = top->findObjectNode("Plugin");

  const ObjectNode::Ptr& odom = base->findObjectNode("Odom");
  const ObjectNode::Ptr& position = odom->findObjectNode("position");
  const ObjectNode::Ptr& velocity = odom->findObjectNode("velocity");
  const ObjectData::Ptr& position_x = position->findObjectData("x");
  const ObjectData::Ptr& position_y = position->findObjectData("y");
  const ObjectData::Ptr& position_z = position->findObjectData("z");
  const ObjectData::Ptr& velocity_x = velocity->findObjectData("x");
  const ObjectData::Ptr& velocity_y = velocity->findObjectData("y");
  const ObjectData::Ptr& velocity_z = velocity->findObjectData("z");
  auto& obs_odom = observations_->odom;
  position_x->appendData(time, obs_odom.position[0]);
  position_y->appendData(time, obs_odom.position[1]);
  position_z->appendData(time, obs_odom.position[2]);
  velocity_x->appendData(time, obs_odom.velocity[0]);
  velocity_y->appendData(time, obs_odom.velocity[1]);
  velocity_z->appendData(time, obs_odom.velocity[2]);

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

  auto& quat_buffer = obs_imu.quat;
  quat_w->appendData(time, quat_buffer[3]);
  quat_x->appendData(time, quat_buffer[0]);
  quat_y->appendData(time, quat_buffer[1]);
  quat_z->appendData(time, quat_buffer[2]);

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
  battery_voltage->appendData(time, obs_battery.voltage);
  battery_current->appendData(time, obs_battery.current);
  battery_soc->appendData(time, obs_battery.soc);
  battery_temp->appendData(time, obs_battery.temp);

  // 处理 System 数据
  const ObjectNode::Ptr& system = base->findObjectNode("System");
  const ObjectData::Ptr& system_cpuUsage = system->findObjectData("cpuUsage");
  const ObjectData::Ptr& system_memoryUsage = system->findObjectData("memoryUsage");
  const ObjectData::Ptr& system_diskUsage = system->findObjectData("diskUsage");
  const ObjectData::Ptr& system_cpuTemp = system->findObjectData("cpuTemp");
  const ObjectData::Ptr& system_cpuFerq = system->findObjectData("cpuFerq");

  // 获取观测值
  auto& obs_system = observations_->system;
  system_cpuUsage->appendData(time, obs_system.cpuUsage);
  system_memoryUsage->appendData(time, obs_system.memoryUsage);
  system_diskUsage->appendData(time, obs_system.diskUsage);
  system_cpuTemp->appendData(time, obs_system.cpuTemp);
  system_cpuFerq->appendData(time, obs_system.cpuFerq);

  // 处理 System 数据
  const ObjectNode::Ptr& sensor = base->findObjectNode("Sensor");
  const ObjectData::Ptr& sensor_temp = sensor->findObjectData("temp");
  const ObjectData::Ptr& sensor_humidity = sensor->findObjectData("humidity");

  // 获取观测值
  auto& obs_sensor = observations_->sensor;
  sensor_temp->appendData(time, obs_sensor.temp);
  sensor_humidity->appendData(time, obs_sensor.humidity);

  auto &obs_act = observations_->actuator;
  const ObjectNode::Ptr& actuators = base->findObjectNode("Actuators");

  for (int i = 0; i < numberOfActuator_; i++) {
    const ObjectNode::Ptr& actuators_x = actuators->findObjectNode(QString("[%1]").arg(i));
    actuators_x->findObjectData("state")         ->appendData(time, obs_act[i].state);
    actuators_x->findObjectData("pos")           ->appendData(time, obs_act[i].pos);
    actuators_x->findObjectData("vel")            ->appendData(time, obs_act[i].vel);
    actuators_x->findObjectData("torque")      ->appendData(time, obs_act[i].torque);
    actuators_x->findObjectData("power")       ->appendData(time, obs_act[i].power);
    actuators_x->findObjectData("temp_m")    ->appendData(time, obs_act[i].temperature);
    actuators_x->findObjectData("Temp_d")    ->appendData(time, obs_act[i].driverTemperature);
  }

}

bool RobotBase::writeData(const QByteArray& data){
  return dataAllocator_->write(CommunicationConfiguration::CommProtocol::Plugin, data);
}

bool RobotBase::readData(DataPktBufferTimePtrVec& vec){
  return  dataAllocator_->read(CommunicationConfiguration::CommProtocol::Plugin, vec);
}

void RobotBase::setTopWidget(QWidget *topWidget){
  topWidget_ = topWidget;
}
