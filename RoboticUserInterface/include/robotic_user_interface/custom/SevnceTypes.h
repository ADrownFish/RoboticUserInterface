#pragma once

#include <string>
#include <array>

namespace sevnce {
namespace high {

// 返回码
enum class RetCode {
    Success = 0,           // 成功
    Timeout = 1,           // 超时
    Error   = 2,           // 错误
};

// 切换命令
enum class SwitchCmd {
    None = 0,              // 保持当前模式
    Stop = 1,              // 急停(立即切换到卧倒模式)
    Prone = 2,             // 卧倒(正常切换到卧倒模式)
    Stand = 3,             // 站立
    Pose = 4,              // 调姿
    Walk = 5,              // 行走(传统控制)
    WalkRL1 = 6,           // 行走(强化学习1)
    WalkRL2 = 7,           // 行走(强化学习2)
};

// 机器人模式
enum class RobotMode {
    Prone = 1,             // 卧倒状态
    Lying = 2,             // 卧姿状态(中间状态)
    Squat = 3,             // 蹲姿状态(中间状态)
    Stand = 4,             // 站立状态
    Pose = 5,              // 调姿状态
    Walk = 6,              // 行走状态(传统控制)
    WalkRL1 = 7,           // 行走状态(强化学习1)
    WalkRL2 = 8,           // 行走状态(强化学习2)
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

#pragma pack(1)

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

// 目标模式命令
struct GoalCmd {
    float x;
    float y;
    float yaw;
};

struct Connect {
    uint8_t isRequest;  // 0 - 释放连接 1 - 申请连接
};

struct Authority {
    uint8_t isRequest;  // 0 - 释放控制权 1 - 申请控制权
};

struct HighCmd {
    // 切换命令
    SwitchCmd switchCmd;
    // 是否感知模式(1:感知模式 0:盲踩模式)
    uint8_t isPerceptive;
    // 姿态(mode=Pose时有效)
    PoseCmd pose;
    // 速度(mode=Walk、WalkRL1、WalkRL1时有效)
    VelocityCmd velocity;
    // 目标(mode=Walk时有效)
    GoalCmd goal;
    // 行走步态(mode=Walk时有效)
    WalkGait walkGait;
    // 行走模式(mode=Walk时有效)
    WalkMode walkMode;
    // 持续踏步(1:持续踏步 0:自动停止踏步, mode=Walk时有效)
    uint8_t keepWalking;
    // 锁定地形(1:锁定地形 0:解除释放, mode=Walk时有效)
    uint8_t lockedTerrain;
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
    // 电机电流(A)
    float motorCurrent;
    // 电机电压(V)
    float motorVoltage;
    // 关节电机温度(°C)
    int8_t motorTemp;
    // 驱动器温度(°C)
    int8_t driverTemp;
};

// BMS信息
struct Bms {
    // 状态
    uint32_t status;
    // 剩余电量(%)
    uint8_t soc;
    // 电流(A)
    float current;
    // 电压(V)
    float voltage;
    // 温度(°C)
    int8_t temp;
    // 完全充放电次数
    uint16_t cycle;
};

// IMU信息
struct IMU {
    // 姿态四元数(w,x,y,z)
    std::array<float, 4> quat;
    // 欧拉角(rad)
    std::array<float, 3> rpy;
    // 角速度(rad/s)
    std::array<float, 3> angularVel;
    // 线加速度(m/s2)
    std::array<float, 3> linearAcc;
    // 时间戳
    uint64_t timeStamp;
    // 温度(°C)
    int8_t temp;
};

// 系统信息
struct SysInfo {
    uint8_t alarmStatus;
    int8_t cpuCoreMaxTemp;
    int8_t cpuPackageTemp;
    uint8_t cpuUsage;
    uint8_t memoryUsage;
    uint8_t diskUsage;
    int8_t bodyTemp;
    uint8_t  bodyHumidity;
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
    // 姿态四元数
    float quat_w;
    float quat_x;
    float quat_y;
    float quat_z;
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
    uint8_t isPerceptive;
    // 位姿
    Pose pose;
    // 速度
    Velocity velocity;
    // 行走步态(mode=Walk时有效)
    WalkGait walkGait;
    // 行走模式(mode=Walk时有效)
    WalkMode walkMode;
    // 持续踏步(1:持续踏步 0:自动停止踏步, mode=Walk时有效)
    uint8_t keepWalking;
    // 锁定地形(1：锁定地形 0:解除锁定, mode=Walk时有效)
    uint8_t lockedTerrain;
    // 机身高度(mode=Pose、Walk时有效)
    float bodyHeight;
    // 抬腿高度(mode=Walk时有效)
    float swingHeight;
    // 状态切换中(不接收任何命令， 同时pose、velocity、goal全置为零)
    uint8_t isSwitching;
    // 静止站立(非持续踏步时，速度命令为零或目标达到，同时未受到外力扰动，自动静止站立)
    uint8_t isStance;
    // 电池信息
    Bms bms;
    // IMU信息
    IMU imu;
    // 系统信息
    SysInfo sys;
    // 12个关节信息
    std::array<JointState, 12> joints;
};

#pragma pack()
}
}
