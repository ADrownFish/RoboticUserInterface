#include "robotic_user_interface/core/DataSource.h"
#include "robotic_user_interface/core/FunctionUtils.h"

DataSource::DataSource(int num_actuator){
	qRegisterMetaType<ObjectNode::Ptr>("ObjectNode::Ptr");

  topNode_ = std::make_shared<ObjectNode>();
	topNode_->name = "_Top_Node";
	topNode_->addNode(createPluginNode(num_actuator));
  topNode_->resetID(-1);

  resetTime();
}

ObjectNode::Ptr DataSource::createAxis3Node(const QString& nodeName){
	auto node = std::make_shared<ObjectNode>();
  node->name = nodeName;
  node->addObject(std::make_shared<ObjectData>("x"));
  node->addObject(std::make_shared<ObjectData>("y"));
  node->addObject(std::make_shared<ObjectData>("z"));

	return node;
}
ObjectNode::Ptr DataSource::createAxisQuatNode(const QString& nodeName){
	auto node = std::make_shared<ObjectNode>();
  node->name = nodeName;
  node->addObject(std::make_shared<ObjectData>("w"));
  node->addObject(std::make_shared<ObjectData>("x"));
  node->addObject(std::make_shared<ObjectData>("y"));
  node->addObject(std::make_shared<ObjectData>("z"));
	return node;
}


ObjectNode::Ptr DataSource::createTestNode() {
  ObjectNode::Ptr node = std::make_shared<ObjectNode>();
  node->name = "Sensor Test";
  node->addNode(createAxis3Node("temperature"));
  return node;
}

ObjectNode::Ptr DataSource::createPluginNode(int num_actuator){
	ObjectNode::Ptr node = std::make_shared<ObjectNode>();
	node->name = "Plugin";

  auto odom = std::make_shared<ObjectNode>();
  odom->name = "Odom";
  odom->addNode(createAxis3Node("position"));
  odom->addNode(createAxis3Node("velocity"));

  auto battery = std::make_shared<ObjectNode>();
  battery->name = "Battery";
  battery->addObject(std::make_shared<ObjectData>("voltage"));
  battery->addObject(std::make_shared<ObjectData>("current"));
  battery->addObject(std::make_shared<ObjectData>("soc"));
  battery->addObject(std::make_shared<ObjectData>("temp"));

  auto imu = std::make_shared<ObjectNode>();
  imu->name = "IMU";
  imu->addNode(createAxisQuatNode("quat"));
  imu->addNode(createAxis3Node("euler"));
  imu->addNode(createAxis3Node("acc"));
  imu->addNode(createAxis3Node("angVel"));
  imu->addNode(createAxis3Node("angAcc"));

  auto system = std::make_shared<ObjectNode>();
  system->name = "System";
  system->addObject(std::make_shared<ObjectData>("cpuUsage"));
  system->addObject(std::make_shared<ObjectData>("memoryUsage"));
  system->addObject(std::make_shared<ObjectData>("diskUsage"));
  system->addObject(std::make_shared<ObjectData>("cpuTemp"));
  system->addObject(std::make_shared<ObjectData>("cpuFerq"));

  auto sensor = std::make_shared<ObjectNode>();
  sensor->name = "Sensor";
  sensor->addObject(std::make_shared<ObjectData>("temp"));
  sensor->addObject(std::make_shared<ObjectData>("humidity"));

  auto actuators = std::make_shared<ObjectNode>();
  actuators->name = "Actuators";
  for (int i = 0; i < num_actuator; ++i) {
    auto actuator = std::make_shared<ObjectNode>();
    actuator->name = QString("[%1]").arg(i);
    actuator->addObject(std::make_shared<ObjectData>("state"));
    actuator->addObject(std::make_shared<ObjectData>("pos"));
    actuator->addObject(std::make_shared<ObjectData>("vel"));
    actuator->addObject(std::make_shared<ObjectData>("torque"));
    actuator->addObject(std::make_shared<ObjectData>("voltage"));
    actuator->addObject(std::make_shared<ObjectData>("current"));
    actuator->addObject(std::make_shared<ObjectData>("power"));
    actuator->addObject(std::make_shared<ObjectData>("temp_m"));
    actuator->addObject(std::make_shared<ObjectData>("Temp_d"));

    actuators->addNode(actuator);
  }

  // 添加所有节点到 Base
  node->addNode(odom);
  node->addNode(battery);
  node->addNode(imu);
  node->addNode(system);
  node->addNode(sensor);
  node->addNode(actuators);

  node->resetID(0);

  return node;
}

void DataSource::resetDataSource(){
  for(auto &it : topNode_->children){
    if(it->name == "Plugin"){
      it->clearData();
      continue;
    }

    it->clear();
  }
}

void DataSource::clearData(){
	topNode_->clearData();
  resetTime();
}

void DataSource::resetTime(){
  startTime_ = timestamp_ms_f();
}

scalar_t DataSource::time() const {
  return  timestamp_ms_f() - startTime_;
}

scalar_t DataSource::startTime() const {

  return  startTime_;
}