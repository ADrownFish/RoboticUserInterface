#include "robotic_user_interface/core/DataSource.h"

DataSource::DataSource(int num_actuator){
	qRegisterMetaType<ObjectNode::Ptr>("ObjectNode::Ptr");

  topNode_ = std::make_shared<ObjectNode>();
	topNode_->name = "_Top_Node";
  topNode_->id = 0;
	topNode_->addNode(createBaseNode(num_actuator));
  //topNode_->addNode(createTestNode());

  topNode_->resetID(topNode_->id);

	std::cout << topNode_->toString().toLocal8Bit().data() << std::endl;
  
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

ObjectNode::Ptr DataSource::createBaseNode(int num_actuator){
	ObjectNode::Ptr node = std::make_shared<ObjectNode>();
	node->name = "Base";

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
  system->addObject(std::make_shared<ObjectData>("cpuCoreTemp"));
  system->addObject(std::make_shared<ObjectData>("cpuPackageTemp"));

  auto actuators = std::make_shared<ObjectNode>();
  actuators->name = "Actuators";
  for (int i = 0; i < num_actuator; ++i) {
    auto actuator = std::make_shared<ObjectNode>();
    actuator->name = QString("Actuator_%1").arg(i);
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
  node->addNode(actuators);

  return node;
}


void DataSource::clearData(){
	topNode_->clearData();
}

void DataSource::resetTime(){
  startTime = timestamp_ns();
}

scalar_t DataSource::time(){
  uint64_t retTime = timestamp_ns() - startTime;
  return scalar_t(retTime) / scalar_t(1000000000);
}
