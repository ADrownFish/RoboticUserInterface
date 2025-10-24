#include "robotic_user_interface/custom/SevnceRobot.h"
#include "robotic_user_interface/core/BinarySerializer.h"
#include "robotic_user_interface/core/FunctionUtils.h"

#include "ui_SevnceRobot_Ctrl.h"
#include "ui_SevnceRobot_State.h"

#include "qwool/qwwindowwidget.h"

#include <QDebug>
#include <QMessageBox>

#include <stdint.h>
#include <cstdint> 
#include <string>
#include <iostream>
#include <iomanip> 
#include <sstream>

template<size_t N>
void printFloatArray(const std::array<float, N>& arr, const std::string& prefix = "") {
    std::cout << prefix << "[" << N << "]: { ";
    
    for (size_t i = 0; i < N; ++i) {
        std::cout << std::fixed << std::setprecision(6) << arr[i];
        if (i < N - 1) {
            std::cout << ", ";
        }
    }
    std::cout << " }" << std::endl;
}

namespace sevnce{


namespace sdk {

  // 返回码
  enum class RetCode {
    Success = 0,           // 成功
    Timeout = 1,           // 超时
    Error   = 2,           // 错误
  };

  // 切换命令
  enum class SwitchCmd {
    None = 0,              // 保持当前模式
    Stop = 1,              // 急停(立即切换到阻尼状态)
    Sit = 2,               // 卧倒(正常切换到阻尼状态)
    Stand = 3,             // 站立
    Pose = 4,              // 调姿
    Walk = 5,              // 行走(传统控制)
    WalkRL1 = 6,           // 行走(强化学习1)
    WalkRL2 = 7,           // 行走(强化学习2)

    Docking = 255          // 对接充电桩(对接完成切换到阻尼状态)
  };

  // 机器人模式
  enum class RobotMode {
    Passive = 1,           // 阻尼状态
    Lying = 2,             // 卧姿状态(中间状态)
    Squat = 3,             // 蹲姿状态(中间状态)
    Stand = 4,             // 站立状态
    Pose = 5,              // 调姿状态
    Walk = 6,              // 行走状态(传统控制)
    WalkRL1 = 7,           // 行走状态(强化学习1)
    WalkRL2 = 8,           // 行走状态(强化学习2)

    Docking = 255          // 对接状态(中间状态)
  };

  // 行走步态(robotMode=Walk时有效)
  enum class WalkGait {
    Walk  = 1,      // 行走
    Run   = 2,      // 跑步
    Climb = 3,      // 攀爬
  };

  // 行走模式(robotMode=Walk时有效)
  enum class WalkMode {
    Velocity = 0,   // 速度模式
    Goal = 1,       // 目标模式
  };  

  // 告警状态
  enum class AlarmStatus {
    PowerCommError        = 0x01 << 0,     // 程控板通讯异常
    IMUCommError          = 0x01 << 1,     // IMU通讯异常
    MotorCommError        = 0x01 << 2,     // 电机通讯异常
    MotorFault            = 0x01 << 3,     // 电机故障
    MotorOverTemp         = 0x01 << 4,     // 电机过热
    DriverOverTemp        = 0x01 << 5,     // 驱动器过热
    SingleOverPower       = 0x01 << 6,     // 单电机功率超限
    TotalOverPower        = 0x01 << 7,     // 总功率超限
    BatteryFault          = 0x01 << 8,     // 电池故障
    EmergencyStop         = 0x01 << 9,     // 紧急停止
    BodyOverTemp          = 0x01 << 10,    // 舱体过热
    SystemWarn            = 0x01 << 11,    // 系统警告
    JointPosAbnormal      = 0x01 << 12,    // 关节位置异常
    JointVelAbnormal      = 0x01 << 13,    // 关节速度异常
    JointTauAbnormal      = 0x01 << 14,    // 关节扭矩异常
    MotorDataAbnormal     = 0x01 << 15,    // 电机数据异常
    JointCmdTimeout       = 0x01 << 16,    // 关节命令超时

    FallOver              = 0x01 << 24,    // 机器人摔倒
    PTZCommError          = 0x01 << 25,    // 云台通讯异常
  };

  // 调整姿态命令
  struct PoseCmd {
    float roll; 
    float pitch;
    float yaw;
  };

  // 速度模式命令
  struct VelocityCmd {
    float xVel;
    float yVel;
    float yawVel;
  };

  // 目标模式命令(Walk时为世界系, Docking时为机身系)
  struct GoalCmd {
    float x;
    float y;
    float yaw;
  };

  struct HighCmd {
    // 切换命令 
    SwitchCmd switchCmd;
    // 是否感知模式(1:感知模式 0:盲踩模式)
    bool isPerceptive;
    // 姿态(mode=Pose时有效)
    PoseCmd pose;
    // 速度(mode=Walk、WalkRL1、WalkRL2时有效)
    VelocityCmd velocity;
    // 目标(mode=Walk,Docking时有效)
    GoalCmd goal;
    // 行走步态(mode=Walk时有效)
    WalkGait walkGait;
    // 行走模式(mode=Walk时有效)
    WalkMode walkMode;  
    // 持续踏步(1:持续踏步 0:自动停止踏步, mode=Walk时有效)
    bool keepWalking;
    // 机身高度偏移(mode=Pose、Walk时有效)
    float deltaBodyHeight;
    // 抬腿高度偏移(mode=Walk时有效)
    float deltaSwingHeight;
  };

  // 关节信息
  struct JointState {
    // 关节位置
    float q;
    // 关节速度
    float dq;
    // 关节扭矩
    float tau;
    // 电机故障码
    uint32_t motorFault;    
    // 电机功率(W)
    float power;
    // 关节电机温度(°C)
    float motorTemp;
    // 驱动器温度(°C)
    float driverTemp;
  };

  // BMS信息
  struct Bms {
    // 状态
    uint32_t status;
    // 剩余电量(%)
    float soc;
    // 电流(A)
    float current;
    // 电压(V)
    float voltage;
    // 温度(°C)
    float temp;
    // 完全充放电次数
    uint16_t cycle;
  };

  // IMU信息
  struct IMU {
    // 姿态四元数(w,x,y,z)
    std::array<float, 4> quat;	
    // 角速度(rad/s) 
    std::array<float, 3> angularVel;		
    // 线加速度(m/s2) 
    std::array<float, 3> linearAcc; 
    // 时间戳
    uint64_t timeStamp;
    // 温度(°C)
    float temp; 
  };

  // 系统信息
  struct SysInfo {
    uint8_t  alarmStatus;
    // CPU温度(°C, Packet 0)
    float    cpuTemp;
    // CPU频率(GHz)
    float    cpuFrequency;
    // CPU使用率(0%~100%)
    uint32_t cpuUsage;
    // 内存使用率(0%~100%)
    uint32_t memoryUsage;
    // 硬盘使用率(0%~100%)
    uint32_t diskUsage;
    // 舱体温度(°C)
    int32_t  bodyTemp;
    // 舱体湿度(%RH)
    uint32_t bodyHumidity;
  };  

  // 位姿信息
  struct Pose {
    // 位置
    float x;
    float y;
    float z;
    // 姿态欧拉角
    float roll; 
    float pitch;
    float yaw;    
  };

  // 速度信息
  struct Velocity {
    // 线速度
    float xVel;
    float yVel;
    float zVel;
    // 角速度
    float rollVel;
    float pitchVel;
    float yawVel;
  };

  struct HighState {
    // 告警状态
    uint32_t alarmStatus;
    // 控制模式(3:遥控器控制 2:APP控制 1:远程控制 0:自动控制)
    uint8_t ctrlMode;
    // 模式
    RobotMode robotMode;
    // 是否感知模式(1:感知模式 0:盲踩模式)
    bool isPerceptive;      
    // 位姿
    Pose pose;
    // 速度
    Velocity velocity;
    // 行走步态(mode=Walk时有效)
    WalkGait walkGait;        
    // 行走模式(mode=Walk时有效)
    WalkMode walkMode;  
    // 持续踏步(1:持续踏步 0:自动停止踏步, mode=Walk时有效)
    bool keepWalking;
    // 机身高度(m, mode=Pose、Walk时有效)
    float bodyHeight;
    // 抬腿高度(m, mode=Walk时有效)
    float swingHeight;
    // 状态切换中(不接收任何命令， 同时pose、velocity、goal全置为零)
    bool isSwitching;     
    // 静止站立(非持续踏步时，速度命令为零或目标达到，同时未受到外力扰动，自动静止站立)
    bool isStance;
    // 电池信息
    Bms bms;
    // IMU信息
    IMU imu;
    // 系统信息
    SysInfo sys;
    // 12个关节信息
    std::array<JointState, 16> joints;
    // 充电接触器状态(按位表示 bit0表示负极 bi1表示正极, 0-断开/1-闭合)
    uint8_t chargeContactorState;
    
    // 行使里程
    float distance;
  };

  struct Connect{
    uint8_t isRequest;
  };

  struct Authority{
    uint8_t isRequest;
  };
}

  // 编码状态 [原始] -> [压缩]
  bool encode(const sdk::HighState &data, std::vector<uint8_t> &buffer){

    constexpr int size_ = 360;
    buffer.resize(size_);

    robot::BinarySerializer bs(buffer.data());

    bs.writeUint32(data.alarmStatus);
    bs.writeUint8(data.ctrlMode);
    bs.writeUint8(static_cast<uint8_t>(data.robotMode));
    bs.writeUint8(data.isPerceptive);

    bs.writeFloat32(data.pose.x);
    bs.writeFloat32(data.pose.y);
    bs.writeFloat32(data.pose.z);
    bs.writeFloat32(data.pose.roll);
    bs.writeFloat32(data.pose.pitch);
    bs.writeFloat32(data.pose.yaw);

    bs.writeFloat32(data.velocity.xVel);
    bs.writeFloat32(data.velocity.yVel);
    bs.writeFloat32(data.velocity.zVel);
    bs.writeFloat32(data.velocity.rollVel);
    bs.writeFloat32(data.velocity.pitchVel);
    bs.writeFloat32(data.velocity.yawVel);

    bs.writeUint8(static_cast<uint8_t>(data.walkGait));
    bs.writeUint8(static_cast<uint8_t>(data.walkMode));
    bs.writeUint8(data.keepWalking);
    bs.writeUint8(data.bodyHeight * 100.f);
    bs.writeUint8(data.swingHeight * 100.f);
    bs.writeUint8(data.isSwitching);
    bs.writeUint8(data.isStance);

    bs.writeUint32(data.bms.status);
    bs.writeUint8(data.bms.soc);
    bs.writeInt16(data.bms.current * 10.f);
    bs.writeInt16(data.bms.voltage * 10.f);
    bs.writeInt8(data.bms.temp);
    bs.writeUint16(data.bms.cycle);

    for (int i = 0; i < 4; ++i)
      bs.writeFloat32(data.imu.quat[i]);
    for (int i = 0; i < 3; ++i)
      bs.writeFloat32(data.imu.angularVel[i]);
    for (int i = 0; i < 3; ++i)
      bs.writeFloat32(data.imu.linearAcc[i]);
    bs.writeUint64(data.imu.timeStamp);
    bs.writeInt8(data.imu.temp);

    bs.writeUint8(data.sys.alarmStatus);
    bs.writeInt8(data.sys.cpuTemp);
    bs.writeUint8(data.sys.cpuFrequency * 10.f);
    bs.writeUint8(data.sys.cpuUsage);
    bs.writeUint8(data.sys.memoryUsage);
    bs.writeUint8(data.sys.diskUsage);
    bs.writeInt8(data.sys.bodyTemp);
    bs.writeUint8(data.sys.bodyHumidity);

    for(int i = 0, count = data.joints.size(); i < count; i++){
      auto &j_dst = data.joints[i];

      bs.writeInt16(j_dst.q * 1000.f);
      bs.writeInt16(j_dst.dq * 1000.f);
      bs.writeInt16(j_dst.tau * 10.f);
      bs.writeUint32(j_dst.motorFault);
      bs.writeInt16(j_dst.power);
      bs.writeInt8(j_dst.motorTemp - 50.f);
      bs.writeInt8(j_dst.driverTemp - 50.f);
    }

    bs.writeUint8(data.chargeContactorState);
    bs.writeFloat32(data.distance);

    std::cerr << "sdk encode state size: " << size_ << " bs.offset: " << bs.offset() << std::endl;
    assert(size_ == bs.offset());
    return true;
  }
  // 编码命令 [原始] -> [压缩]
  bool encode(const sdk::HighCmd &data,   std::vector<uint8_t> &buffer){
    constexpr int size_ = 43;
    buffer.resize(size_);

    robot::BinarySerializer bs(buffer.data());

    bs.writeUint8(static_cast<uint8_t>(data.switchCmd));
    bs.writeUint8(data.isPerceptive);

    bs.writeFloat32(data.pose.roll);
    bs.writeFloat32(data.pose.pitch);
    bs.writeFloat32(data.pose.yaw);

    bs.writeFloat32(data.velocity.xVel);
    bs.writeFloat32(data.velocity.yVel);
    bs.writeFloat32(data.velocity.yawVel);

    bs.writeFloat32(data.goal.x);
    bs.writeFloat32(data.goal.y);
    bs.writeFloat32(data.goal.yaw);

    bs.writeUint8(static_cast<uint8_t>(data.walkGait));
    bs.writeUint8(static_cast<uint8_t>(data.walkMode));

    bs.writeUint8(data.keepWalking);
    bs.writeInt8(data.deltaBodyHeight * 100.f);
    bs.writeInt8(data.deltaSwingHeight * 100.f);

    // std::cerr << "sdk endcode cmd size: " << size_ << " bs.offset: " << bs.offset() << std::endl;
    assert(size_ == bs.offset());
    return true;
  }
  // 解码状态 [压缩] -> [原始]
  bool decode(std::vector<uint8_t> &buffer, sdk::HighState &data){
    constexpr int size_ = 360;
    if(buffer.size() != size_){
      return false;
    }
    assert(size_ == buffer.size());

    robot::BinarySerializer bs(buffer.data());

    data.alarmStatus = bs.readUint32();
    data.ctrlMode = bs.readUint8();
    data.robotMode = static_cast<sdk::RobotMode>(bs.readUint8());
    data.isPerceptive = bs.readUint8();

    data.pose.x = bs.readFloat32();
    data.pose.y = bs.readFloat32();
    data.pose.z = bs.readFloat32();
    data.pose.roll = bs.readFloat32();
    data.pose.pitch = bs.readFloat32();
    data.pose.yaw = bs.readFloat32();

    data.velocity.xVel = bs.readFloat32();
    data.velocity.yVel = bs.readFloat32();
    data.velocity.zVel = bs.readFloat32();
    data.velocity.rollVel = bs.readFloat32();
    data.velocity.pitchVel = bs.readFloat32();
    data.velocity.yawVel = bs.readFloat32();

    data.walkGait = static_cast<sdk::WalkGait>(bs.readUint8());
    data.walkMode = static_cast<sdk::WalkMode>(bs.readUint8());
    data.keepWalking = bs.readUint8();
    data.bodyHeight = bs.readUint8() / 100.f;
    data.swingHeight = bs.readUint8() / 100.f;
    data.isSwitching = bs.readUint8();
    data.isStance = bs.readUint8();

    data.bms.status = bs.readUint32();
    data.bms.soc = bs.readUint8();
    data.bms.current = bs.readInt16() / 10.f;
    data.bms.voltage = bs.readInt16() / 10.f;
    data.bms.temp = bs.readInt8();
    data.bms.cycle = bs.readUint16();

    // imu
    for (int i = 0; i < 4; ++i)
      data.imu.quat[i] = bs.readFloat32();
    for (int i = 0; i < 3; ++i)
      data.imu.angularVel[i] = bs.readFloat32();
    for (int i = 0; i < 3; ++i)
      data.imu.linearAcc[i] = bs.readFloat32();
    data.imu.timeStamp = bs.readUint64();
    data.imu.temp = bs.readInt8();

    // system
    data.sys.alarmStatus = bs.readUint8();
    data.sys.cpuTemp = bs.readInt8();
    data.sys.cpuFrequency = bs.readUint8() / 10.f;
    data.sys.cpuUsage = bs.readUint8();
    data.sys.memoryUsage = bs.readUint8();
    data.sys.diskUsage = bs.readUint8();
    data.sys.bodyTemp = bs.readInt8();
    data.sys.bodyHumidity = bs.readUint8();

    // joints
    for (int i = 0, count = data.joints.size(); i < count; ++i) {
      auto &j = data.joints[i];
      j.q = bs.readInt16() / 1000.f;
      j.dq = bs.readInt16() / 1000.f;
      j.tau = bs.readInt16() / 10.f;
      j.motorFault = bs.readUint32();
      j.power = bs.readInt16();
      j.motorTemp = bs.readInt8() + 50.f;
      j.driverTemp = bs.readInt8() + 50.f;
    }

    data.chargeContactorState = bs.readUint8();
    data.distance = bs.readFloat32();

    // std::cerr << "sdk decode state size: " << size_ << " bs.offset: " << bs.offset() << std::endl;
    assert(size_ == bs.offset());
    return true;
  }
  // 解码命令 [压缩] -> [原始]
  bool decode(std::vector<uint8_t> &buffer,   sdk::HighCmd &data){
    constexpr int size_ = 43;
    if(buffer.size() != size_){
      return false;
    }
    assert(size_ == buffer.size());

    robot::BinarySerializer bs(buffer.data());

    data.switchCmd = static_cast<sdk::SwitchCmd>(bs.readUint8());
    data.isPerceptive = bs.readUint8();

    data.pose.roll = bs.readFloat32();
    data.pose.pitch = bs.readFloat32();
    data.pose.yaw = bs.readFloat32();

    data.velocity.xVel = bs.readFloat32();
    data.velocity.yVel = bs.readFloat32();
    data.velocity.yawVel = bs.readFloat32();

    data.goal.x = bs.readFloat32();
    data.goal.y = bs.readFloat32();
    data.goal.yaw = bs.readFloat32();

    data.walkGait = static_cast<sdk::WalkGait>(bs.readUint8());
    data.walkMode = static_cast<sdk::WalkMode>(bs.readUint8());
    data.keepWalking = bs.readUint8();
    data.deltaBodyHeight = bs.readInt8() / 100.f;
    data.deltaSwingHeight = bs.readInt8() / 100.f;

    std::cerr << "sdk decode state size: " << size_ << " bs.offset: " << bs.offset() << std::endl;
    assert(size_ == bs.offset());
    return true;
  }

class SevnceData{
public:
  SevnceData(){
    reset();
  }
  ~SevnceData(){

  }

  void reset(){
    memset(&cmd, 0, sizeof(sdk::HighCmd));
    memset(&state, 0, sizeof(sdk::HighState));
  }

public:
  sdk::HighCmd cmd;
  sdk::HighState state;
};

class SevnceWidget{
public:
  SevnceWidget(){
    QColor bgColor(50, 50, 50);

    widget_SevnceRobot_Ctrl = new QWWindowWidget();
    widget_SevnceRobot_Ctrl->setBorderRadius(8);
    widget_SevnceRobot_Ctrl->setBackgroundColor(bgColor);
    ui_SevnceRobot_Ctrl.setupUi(widget_SevnceRobot_Ctrl);

    widget_SevnceRobot_State = new QWWindowWidget();
    widget_SevnceRobot_State->setBorderRadius(8);
    widget_SevnceRobot_State->setBackgroundColor(bgColor);
    ui_SevnceRobot_State.setupUi(widget_SevnceRobot_State);

    ui_SevnceRobot_Ctrl.toggle->setPenWidth(2);
    ui_SevnceRobot_Ctrl.toggle->setBackgroundColor(QColor(0,0,0,0));
    ui_SevnceRobot_Ctrl.toggle->setToggle(true);
    ui_SevnceRobot_Ctrl.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));

    ui_SevnceRobot_State.toggle->setPenWidth(2);
    ui_SevnceRobot_State.toggle->setBackgroundColor(QColor(0,0,0,0));
    ui_SevnceRobot_State.toggle->setToggle(true);
    ui_SevnceRobot_State.toggle->setBackCheCkedgroundColor(QColor(0,0,0,0));
  }
  ~SevnceWidget(){

  }

  QList<QWidget *> createCustomInfoWidgets(){
    QList<QWidget *> listWidget;
    listWidget.append(widget_SevnceRobot_Ctrl);
    listWidget.append(widget_SevnceRobot_State);

    return listWidget;
  }

  

public:
  Ui::SevnceRobot_Ctrl ui_SevnceRobot_Ctrl;
  Ui::SevnceRobot_State ui_SevnceRobot_State;
  QWWindowWidget *widget_SevnceRobot_Ctrl;
  QWWindowWidget *widget_SevnceRobot_State;
};



SevnceRobot::SevnceRobot(QObject *parent)
: RobotBase(parent) {

  solver_.setDeviceID(0x01);
  solver_.setEnableFilter(false);
  
  sevnceData = std::make_unique<SevnceData>();
  sevnceWidget = std::make_unique<SevnceWidget>();
}
SevnceRobot::~SevnceRobot(){

}

QStringList SevnceRobot::getActuators() { 
  QStringList list;
  list 
    << "LF HAA   Joint 0" << "LF HFE   Joint 1" << "LF KFE   Joint 2"
    << "RF HAA   Joint 3" << "RF HFE   Joint 4" << "RF KFE   Joint 5"
    << "LH HAA   Joint 6" << "LH HFE   Joint 7" << "LH KFE   Joint 8"
    << "RH HAA   Joint 9" << "RH HFE   Joint 10" << "RH KFE   Joint 11";

  return list; 
};

QString SevnceRobot::getPluginName() { return QString("Sevnce"); }

// std::string formatByteArray(const uint8_t *array, size_t size) {
//   std::stringstream ss;
//   ss << std::hex << std::setw(2) << std::setfill('0');
//   for (size_t i = 0; i < size; ++i) {
//     ss << std::setw(2) << static_cast<int>(array[i]) << " ";
//   }
//   ss << std::dec;
//   return ss.str();
// }

void SevnceRobot::writeData() {

  // send
  std::vector<uint8_t> buffer;
  encode(sevnceData->cmd, buffer);
  writeData(0x02, 0x1, buffer.data(), buffer.size());
}

void SevnceRobot::writeData(uint8_t dstID, uint8_t dataID, const uint8_t *data, size_t size){

  Data cmdData;
  cmdData.appendData(data,size);
  cmdData.head.dstID = dstID;
  cmdData.head.DataID = dataID;
  const Solver::ByteArrayPointer bap = solver_.makeDataPacket(cmdData);

  RobotBase::writeData(QByteArray((char *)bap.data, bap.size));
}

void SevnceRobot::readyRead() {

  // receive
  DataPktBufferTimePtrVec vec;
  bool ret = RobotBase::readData(vec);
  if (!ret) {
    return;
  }

  for (auto &it : vec) {
    solver_.pushBytes(it->buffer.data(), it->buffer.size());
    scalar_t& time = it->timestamp;
    while (solver_.getAvailableSize()) {
      auto data = solver_.getFirstDataPacket();
      unpackData(time, data);
    }
  }
}

void SevnceRobot::init(){
  int numberOfActuator = 12;
  int numberOfEndEffector = 4;
  
  RobotBase::init(numberOfActuator, numberOfEndEffector);

  setupWidgetsControls();
  setupSignalConnection();

  sevnceData->state.alarmStatus = -1;
}

void SevnceRobot::unpackData(scalar_t time, const Data& data){
  switch (data.head.srcID)
  {
  case 0x02:{
    if(data.head.DataID == 0x01){
      std::vector<uint8_t> ba_(data.data, data.data + data.length);
      if(decode(ba_, sevnceData->state)){
        copyToObservations(time);
        recordData();
        dataUpdate = true;
      } else {
        qDebug() << "[SevnceRobot] Received HighState data decoding failed";
      }
    }
  } break;
  
  default:
    break;
  }
}

void SevnceRobot::copyToObservations(scalar_t time){
  auto &s = sevnceData->state;
  // odom
  observations_->odom.position = {s.pose.x, s.pose.y, s.pose.z};
  observations_->odom.velocity = {s.velocity.xVel, s.velocity.yVel, s.velocity.zVel};
  observations_->odom.mileage = 0.0;

  // imu
  // printFloatArray(s.imu.quat, "imu.quat");
  // printFloatArray(s.imu.angularVel, "imu.angularVel");
  // printFloatArray(s.imu.linearAcc, "linearAcc.quat");
  observations_->imu.quat = { s.imu.quat[0], s.imu.quat[1], s.imu.quat[2], s.imu.quat[3] };
  observations_->imu.acceleration = {s.imu.linearAcc[0], s.imu.linearAcc[1], s.imu.linearAcc[2]};
  
  Eigen::Quaternion<scalar_t> q(s.imu.quat[0], s.imu.quat[1], s.imu.quat[2], s.imu.quat[3]);
  auto eulerZYX = eulerZYXFromQuaternion(q, observations_->imu.eulerAngles[2]);
  observations_->imu.eulerAngles = {eulerZYX[2],eulerZYX[1], eulerZYX[0]};
  observations_->imu.angularVelocity = {s.imu.angularVel[0], s.imu.angularVel[1], s.imu.angularVel[2]};
  observations_->imu.angularAcceleration = {0,0,0};

  // system
  observations_->system.cpuTemp = 0;
  observations_->system.cpuFerq = 0;
  observations_->system.cpuUsage       = s.sys.cpuUsage;
  observations_->system.memoryUsage    = s.sys.memoryUsage;
  observations_->system.diskUsage      = s.sys.diskUsage;
  observations_->system.status         = s.sys.alarmStatus;

  // sensor
  observations_->sensor.temp       = s.sys.bodyTemp;
  observations_->sensor.humidity   = s.sys.bodyHumidity;

  // bms
  observations_->battery.voltage = s.bms.voltage;
  observations_->battery.current = s.bms.current;
  observations_->battery.soc     = s.bms.soc;
  observations_->battery.cycle   = s.bms.cycle;
  observations_->battery.temp    = s.bms.temp;
  observations_->battery.status  = s.bms.status;

  for (size_t i = 0; i < numberOfActuator_; i++)  {
    auto &m = observations_->actuator;
    m[i].pos = s.joints[i].q;
    m[i].vel = s.joints[i].dq;
    m[i].torque = s.joints[i].tau;
    m[i].temperature = s.joints[i].motorTemp;
    m[i].driverTemperature = s.joints[i].driverTemp;
    m[i].state = s.joints[i].motorFault;
    m[i].power = s.joints[i].power;
  }

  updateDataSource(time);
}

void SevnceRobot::copyFromCommands(){
  auto &c = sevnceData->cmd;
  c.velocity.xVel = command_->input.velocity[0];
  c.velocity.yVel = command_->input.velocity[1];
  c.velocity.yawVel = command_->input.angularVelocity[2];

  c.goal.x = command_->input.position[0];
  c.goal.y = command_->input.position[1];
  c.goal.yaw = command_->input.eulerAngles[2];
}

QWidget *SevnceRobot::createCustomOperationWidget(){
  return nullptr;
}

QList<QWidget *> SevnceRobot::createCustomInfoWidgets(){
  return sevnceWidget->createCustomInfoWidgets();
}

const QString gotRobotMode(sevnce::sdk::RobotMode mode){
  using namespace sevnce::sdk;

  switch (mode)
  {
  case RobotMode::Passive :  return QString("Passive");    break;
  case RobotMode::Lying :    return QString("Lying");  break;
  case RobotMode::Squat :    return QString("Squat");  break;
  case RobotMode::Stand :    return QString("Stand");  break;
  case RobotMode::Pose :     return QString("Pose");   break;
  case RobotMode::Walk :     return QString("Walk");   break;
  case RobotMode::WalkRL1 :  return QString("RL1");    break;
  case RobotMode::WalkRL2 :  return QString("RL2");    break;
  case RobotMode::Docking :  return QString("Docking"); break;
  }

  return QString("");
}
void SevnceRobot::catchData(){
  copyFromCommands();
  writeData();
}

void SevnceRobot::displayData(){
  if (!dataUpdate) {
    return;
  }
  dataUpdate = false;

  auto& ui_State = sevnceWidget->ui_SevnceRobot_State;
  auto& ui_Ctrl = sevnceWidget->ui_SevnceRobot_Ctrl;
  auto &state__ = sevnceData->state;

  ui_State.lineEdit_bodyHeight  ->setText(QString::number(state__.bodyHeight, 'f', config_->display.precision)  + " m");
  ui_State.lineEdit_swingHeight ->setText(QString::number(state__.swingHeight, 'f', config_->display.precision) + " m");
  ui_State.widget_CtrlMode      ->setSelectUnitIndex(state__.ctrlMode      /* ? "是" : "否"*/ );
  ui_Ctrl.widget_isPerceptive  ->setSelectUnitIndex(state__.isPerceptive  /* ? "是" : "否"*/ );
  ui_Ctrl.widget_keepWalking   ->setSelectUnitIndex(state__.keepWalking   /* ? "是" : "否"*/ );
  ui_State.widget_isSwitching   ->setSelectUnitIndex(state__.isSwitching   /* ? "是" : "否"*/ );
  ui_State.widget_isStance      ->setSelectUnitIndex(state__.isStance      /* ? "是" : "否"*/ );

  QString curName = gotRobotMode(state__.robotMode);
  ui_Ctrl.widget_robotMode->setSelectUnit(curName);

  switch(state__.walkGait){
    case sevnce::sdk::WalkGait::Walk:
      ui_Ctrl.widget_walkGait->setSelectUnitIndex(0);
      break;
    case sevnce::sdk::WalkGait::Run:
      ui_Ctrl.widget_walkGait->setSelectUnitIndex(1);
      break;
    case sevnce::sdk::WalkGait::Climb:
      ui_Ctrl.widget_walkGait->setSelectUnitIndex(2);
      break;
  }

  switch (state__.walkMode){
    case sevnce::sdk::WalkMode::Velocity:
      ui_Ctrl.widget_walkMode->setSelectUnitIndex(0);

      break;
    case sevnce::sdk::WalkMode::Goal:
      ui_Ctrl.widget_walkMode->setSelectUnitIndex(1);

      break;
  }

  ui_State.widget_alarmStatus->setMultiSelectUnitIndex((uint32_t)state__.alarmStatus);
}

void SevnceRobot::setupWidgetsControls(){
  QStringList alarmStatusNameList = {
      "程控板通讯", "IMU通讯",      "电机通讯",   "电机故障", "电机过热",
      "驱动器过热", "单电机超功率", "总功率超限", "电池故障", "紧急停止",
      "舱体过热",   "系统警告",     "机器人摔倒", "云台通讯",
  };
  auto& ui_State = sevnceWidget->ui_SevnceRobot_State;
  auto& ui_Ctrl = sevnceWidget->ui_SevnceRobot_Ctrl;

  ui_Ctrl.widget_walkGait->addUnit("Walk");
  ui_Ctrl.widget_walkGait->addUnit("Run");
  ui_Ctrl.widget_walkGait->addUnit("Climb");
  ui_Ctrl.widget_walkGait->setAllowMouseClicked(true);
  ui_Ctrl.widget_walkGait->setBackgroundColor(QColor(100,100,100,50));

  ui_Ctrl.widget_robotMode->addUnit("Lying");
  ui_Ctrl.widget_robotMode->addUnit("Squat");
  ui_Ctrl.widget_robotMode->addUnit("Stand");
  ui_Ctrl.widget_robotMode->addUnit("Pose");
  ui_Ctrl.widget_robotMode->addUnit("Walk");
  ui_Ctrl.widget_robotMode->addUnit("WalkRL1");
  ui_Ctrl.widget_robotMode->addUnit("WalkRL2");
  ui_Ctrl.widget_robotMode->addUnit("Docking");
  ui_Ctrl.widget_robotMode->setAllowMouseClicked(true);
  ui_Ctrl.widget_robotMode->setBackgroundColor(QColor(100,100,100,50));

  ui_Ctrl.widget_walkMode->addUnit("Velocity");
  ui_Ctrl.widget_walkMode->addUnit("Goal");
  ui_Ctrl.widget_walkMode->setAllowMouseClicked(true);
  ui_Ctrl.widget_walkMode->setBackgroundColor(QColor(100,100,100,50));

  ui_State.widget_CtrlMode      ->addUnit("自动");
  ui_State.widget_CtrlMode      ->addUnit("手动");

  ui_Ctrl.widget_isPerceptive  ->addUnit("否");
  ui_Ctrl.widget_isPerceptive  ->addUnit("是");

  ui_Ctrl.widget_keepWalking   ->addUnit("否");
  ui_Ctrl.widget_keepWalking   ->addUnit("是");

  ui_State.widget_isSwitching   ->addUnit("否");
  ui_State.widget_isSwitching   ->addUnit("是");

  ui_State.widget_isStance      ->addUnit("否");
  ui_State.widget_isStance      ->addUnit("是");

  ui_State.widget_CtrlMode     ->setBackgroundColor(QColor(100,100,100,50));
  ui_Ctrl.widget_isPerceptive  ->setBackgroundColor(QColor(100,100,100,50));
  ui_Ctrl.widget_keepWalking   ->setBackgroundColor(QColor(100,100,100,50));
  ui_State.widget_isSwitching   ->setBackgroundColor(QColor(100,100,100,50));
  ui_State.widget_isStance      ->setBackgroundColor(QColor(100,100,100,50));

  ui_State.widget_CtrlMode     ->setSelectdColor(QColor(50, 180, 120));
  ui_State.widget_isStance      ->setSelectdColor(QColor(50, 180, 120));
  ui_State.widget_isSwitching   ->setSelectdColor(QColor(50, 180, 120));


  for(const QString &name : alarmStatusNameList){
    ui_State.widget_alarmStatus->addUnit(name);
  }
  ui_State.widget_alarmStatus->setSelectionMode(QWWindowButton::SelectionMode::Multiple);
  ui_State.widget_alarmStatus->setMinimumWidth(ui_State.widget_alarmStatus->getUnitSize() * 120);
  ui_State.widget_alarmStatus->setAllowMouseClicked(true);
  ui_State.widget_alarmStatus->setBackgroundColor(QColor(100,100,100,50));
  ui_State.widget_alarmStatus->setSelectdColor(QColor(220, 100, 100));

#if defined(Q_OS_WIN)
  ui_Ctrl.label_msg->setVisible(true);
#elif defined(Q_OS_MAC)
  ui_Ctrl.label_msg->setVisible(false);
#else
  ui_Ctrl.label_msg->setVisible(false);
#endif
}

void SevnceRobot::setupSignalConnection() {
  auto& ui_State = sevnceWidget->ui_SevnceRobot_State;
  auto& ui_Ctrl = sevnceWidget->ui_SevnceRobot_Ctrl;

  QObject::connect(ui_Ctrl.toggle,          &QWSwitcher::toggled, ui_Ctrl.widget,         &QWidget::setVisible);
  QObject::connect(ui_State.toggle,         &QWSwitcher::toggled, ui_State.widget,         &QWidget::setVisible);

  QObject::connect(ui_Ctrl.widget_robotMode, &QWWindowButton::selectUnitIndexChanged,
        [this,&ui_Ctrl](unsigned int index) {
          switchRobotMode(ui_Ctrl.widget_robotMode->getUnitName(index));
        });

  QObject::connect(ui_Ctrl.widget_walkGait, &QWWindowButton::selectUnitIndexChanged, [this,&ui_Ctrl](unsigned int index) {
    switch (index) {
    case 0:
      sevnceData->cmd.walkGait = sevnce::sdk::WalkGait::Walk;
      break;
    case 1:
      sevnceData->cmd.walkGait = sevnce::sdk::WalkGait::Run;
      break;
    case 2:
      sevnceData->cmd.walkGait = sevnce::sdk::WalkGait::Climb;
      break;
    }
    publishNotify(
        NotifyType::Info, QString("Command"),
        QString("Clicked: %1 %2 >> ")
                .arg(index)
                .arg(ui_Ctrl.widget_walkGait->getUnitName(index)) +
            QString("WalkGait to: %1").arg((int)sevnceData->cmd.walkGait));
  });

  QObject::connect(ui_Ctrl.widget_walkMode, &QWWindowButton::selectUnitIndexChanged, [this,&ui_Ctrl](unsigned int index) {
    switch (index) {
    case 0:
      sevnceData->cmd.walkMode = sevnce::sdk::WalkMode::Velocity;
      break;
    case 1:
      sevnceData->cmd.walkMode = sevnce::sdk::WalkMode::Goal;
      break;
    }
    publishNotify(
        NotifyType::Info, QString("Command"),
        QString("Clicked: %1 %2 >> ")
                .arg(index)
                .arg(ui_Ctrl.widget_walkMode->getUnitName(index)) +
            QString("WalkMode to: %1").arg((int)sevnceData->cmd.walkMode));
  });

  QObject::connect(ui_Ctrl.widget_isPerceptive, &QWWindowButton::selectUnitIndexChanged, [this](unsigned int index) {
    sevnceData->cmd.isPerceptive = index; 
  });
  QObject::connect(ui_Ctrl.widget_keepWalking,  &QWWindowButton::selectUnitIndexChanged, [this](unsigned int index) {
    sevnceData->cmd.keepWalking = index; 
  });

  QObject::connect(ui_Ctrl.btn_TakingControl, &QPushButton::clicked,this, [this](bool checked) {
    QByteArray data;
    data.append(static_cast<uint8_t>(checked));
    writeData(0x02, 0x03, (uint8_t*)data.data(), data.size());  // appAuthority
  });

}

void SevnceRobot::switchRobotMode(const QString &str) {
    if(str == "Stop") {   

      auto& ui_Ctrl = sevnceWidget->ui_SevnceRobot_Ctrl;
      int result = displayMessageDialog("This could be dangerous !", "Are you sure you want to stop robot?");
      if (result == QDialog::Rejected) {
        auto& state__ = sevnceData->state;
        QString curName = gotRobotMode(state__.robotMode);
        ui_Ctrl.widget_robotMode->setSelectUnit(curName);
        return;
      }

      sevnceData->cmd.switchCmd = sevnce::sdk::SwitchCmd::Stop;    
      }
    else if(str == "Stop")     {   sevnceData->cmd.switchCmd = sevnce::sdk::SwitchCmd::Stop;    }
    else if(str == "Sit")      {   sevnceData->cmd.switchCmd = sevnce::sdk::SwitchCmd::Sit;     }
    else if(str == "Stand")    {   sevnceData->cmd.switchCmd = sevnce::sdk::SwitchCmd::Stand;   }
    else if(str == "Pose")     {   sevnceData->cmd.switchCmd = sevnce::sdk::SwitchCmd::Pose;    }
    else if(str == "Walk")     {   sevnceData->cmd.switchCmd = sevnce::sdk::SwitchCmd::Walk;    }
    else if(str == "WalkRL1")  {   sevnceData->cmd.switchCmd = sevnce::sdk::SwitchCmd::WalkRL1; }
    else if(str == "WalkRL2")  {   sevnceData->cmd.switchCmd = sevnce::sdk::SwitchCmd::WalkRL2; }
    else if(str == "Docking")  {   sevnceData->cmd.switchCmd = sevnce::sdk::SwitchCmd::Docking; }

    publishNotify(NotifyType::Info, tr("Send to the robot"),
    tr("robot to: %1 %2").arg((int)sevnceData->cmd.switchCmd).arg(str));
  }

void SevnceRobot::flushConfiguration(){

}

void SevnceRobot::commStatusChanged(bool status) {
  if(status){
    // sevnceData->state.alarmStatus = -1;
  } else {
    sevnceData->state.alarmStatus = -1;
  }
}


void SevnceRobot::recordData(){
  if(dataRecorder_ == nullptr)
    return;
  if(!record_){
    return;
  }

  /*const auto & odom = observations_->odom;
  const auto & imu = observations_->imu;
  const auto & sys = observations_->system;
  const auto & bat = observations_->battery;
  const auto & act = observations_->actuator;

  auto& quat_buffer = imu.quat;

  QVector<scalar_t> data;
  data 
  << odom.position[0] << odom.position[1] << odom.position[2]
  << odom.velocity[0] << odom.velocity[1] << odom.velocity[2]

  << quat_buffer[0] << quat_buffer[1] << quat_buffer[2] << quat_buffer[3]
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

  dataRecorder_->submitRecord(data);*/
}


void SevnceRobot::setEnabledRecord(bool enabled){
  record_ = enabled;
  if(dataRecorder_ == nullptr){
    dataRecorder_ = new robot::AsyncDataRecorder(getRecordFilePath(), this);
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

}


extern "C" {
RobotBase *createRobot(QObject* parent) { return new sevnce::SevnceRobot(parent); }

void destroyRobot(RobotBase *robot) { delete robot; }
}