#include "robotic_user_interface/custom/RobotBase.h"
#include <QCoreApplication>
#include <QDir>
#include <QDate>
#include <QPluginLoader>

#include "FluControls/FluMessageBox.h"

void RobotBase::setDataAllocator(const QPointer<DataAllocator>& p)
{
  dataAllocator_ = p;

  // 数据传递
  QObject::connect(dataAllocator_, &DataAllocator::readyRead, [this]() {

    // on signal thread
    
    if (config_->comm.commProtocol == CommunicationConfiguration::CommProtocol::Plugin) {
      QByteArray buffer;
      dataAllocator_->read(CommunicationConfiguration::CommProtocol::Plugin, buffer);
      
      recvMutex_.lock();
      recvbuffer_.append(buffer);
      recvMutex_.unlock();

      emit dataReaches();
    }
  });
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
    displayData();
  });

  QObject::connect(this, &RobotBase::dataReaches, this, &RobotBase::readyRead, Qt::QueuedConnection);

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

  // scalar_t tttt =  DataSource::timestamp_ms_f() * 1000.;
  // 获取顶层节点引用
  const ObjectNode::Ptr& top = dataSource_->topNode();

  // 查找并缓存各个节点
  const ObjectNode::Ptr& base = top->findObjectNode("Plugin");
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

  auto& quat_buffer = obs_imu.quat;

  // 直接使用缓存的节点引用进行数据追加
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
    const ObjectNode::Ptr& actuators_x = actuators->findObjectNode(QString("[%1]").arg(i));
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

  // qDebug() << "RobotBase::updateData() " << DataSource::timestamp_ms_f()*1000. - tttt;;

}

void RobotBase::writeData(const QByteArray& data){
  dataAllocator_->write(CommunicationConfiguration::CommProtocol::Plugin, data);
}

void RobotBase::readData(QByteArray& data){

  recvMutex_.lock();
  if (!recvbuffer_.isEmpty()) {
    data = std::move(recvbuffer_);
  }
  recvMutex_.unlock();
}

void RobotBase::setTopWidget(QWidget *topWidget){
  topWidget_ = topWidget;
}
