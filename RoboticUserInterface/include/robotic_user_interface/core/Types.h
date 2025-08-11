#pragma once

#include "taskflow/taskflow/taskflow.hpp"

#include "Eigen/Dense"

#include <memory>
#include <vector>
#include <array>
#include <math.h>

#include <QVector>
#include <QList>
#include <QSharedPointer>
#include <QColor>
#include <QFont>
#include <QString>
#include <QRegularExpression>
#include <QRandomGenerator>

// ****************************************************************
using scalar_t = double;
// using quaternion     = std::array<scalar_t, 4>;
using quaternion     = Eigen::Quaternion<scalar_t>;

using vector_t       = std::vector<scalar_t>;
using vector_bool_t  = std::vector<bool>;
// using vector3_t      = std::array<scalar_t, 3>;

// using vector_t       = Eigen::Matrix<scalar_t, -1, 1>;
// using vector_bool_t  = std::vector<bool>;
using vector3_t      = Eigen::Matrix<scalar_t, 3, 1>;

// using matrix_t      = Eigen::Matrix<scalar_t, -1, -1>;
// using matrix3_t      = Eigen::Matrix<scalar_t, 3, 3>;

constexpr scalar_t Rad2Deg = scalar_t(180.0 / M_PI);
constexpr scalar_t Deg2Rad = scalar_t(M_PI / 180.0);
// ****************************************************************


class CommandBase {
public:
  using Ptr = std::shared_ptr<CommandBase>;
  struct Input {
    vector_t position;
    vector_t velocity;
    vector_t eulerAngles;
    vector_t angularVelocity;
  };

  CommandBase(){
    input.position.resize(3,0);
    input.velocity.resize(3,0);
    input.eulerAngles.resize(3,0);
    input.angularVelocity.resize(3,0);
  }

public:
  Input input;
};

class ObservationsBase {
public:
  using Ptr = std::shared_ptr<ObservationsBase>;

  struct Actuator {
    uint32_t state = 0;
    scalar_t pos = 0.f;
    scalar_t vel = 0.f;
    scalar_t torque = 0.f;
    scalar_t voltage = 0.f;
    scalar_t current = 0.f;
    scalar_t power = 0.f;
    scalar_t temperature = 0.f;
    scalar_t driverTemperature = 0.f;
  };
  struct Odom {
    vector_t position;
    vector_t velocity;
  };
  struct Battery {
    int32_t cycle;
    int32_t status;
    double soc;
    double temp;
    double current;
    double voltage;
  };
  struct Imu {
    quaternion quat;
    vector_t eulerAngles;
    vector_t acceleration;

    vector_t angularVelocity;
    vector_t angularAcceleration;

    vector_t Magnetometer;
  };
  struct System {
    int32_t status;
    double cpuUsage;
    double memoryUsage;
    double diskUsage;
    double cpuCoreMaxTemp;
    double cpuPackageTemp;
  };

  struct Sensor {
    double temp;
    double humidity;
  };
  struct EndEffector {
    vector_t position;
    vector_t velocity;
    vector_t force;

    vector_bool_t contactFlag_sensor;
    vector_bool_t contactFlag_estimate;
  };
public:
  ObservationsBase(int numberOfActuator, int numberOfEndEffector){
    numberOfActuator_ = numberOfActuator;
    numberOfEndEffector_ = numberOfEndEffector;
    int numberOfEndEffectorDOFs = numberOfEndEffector_ * 3;

    // odom
    odom.position.resize(3, 0.0);
    odom.velocity.resize(3, 0.0);

    // imu

    imu.quat = quaternion{1.0,0.0,0.0,0.0};
    imu.acceleration.resize(3, 0.0);
    imu.eulerAngles.resize(3, 0.0);

    imu.angularVelocity.resize(3, 0.0);
    imu.angularAcceleration.resize(3, 0.0);

    imu.Magnetometer.resize(3, 0.0);

    // battery
    battery.status = 0;
    battery.soc = 0;
    battery.current = 0.0;
    battery.voltage = 0.0;
    battery.temp = 0;
    battery.cycle = 0;

    // system
    system.status = 0;
    system.cpuCoreMaxTemp = 0;
    system.cpuPackageTemp = 0;
    system.cpuUsage = 0;
    system.memoryUsage = 0;
    system.diskUsage = 0;

    // sensor
    sensor.temp = 0;
    sensor.humidity = 0;

    // actuator
    actuator.resize(numberOfActuator);

    // end Effector
    endEffector.position  .resize(numberOfEndEffector_ * 3, 0.0);
    endEffector.velocity  .resize(numberOfEndEffector_ * 3, 0.0);
    endEffector.force.resize(numberOfEndEffector_ * 3, 0.0);

    endEffector.contactFlag_sensor.resize(numberOfEndEffector_,false);
    endEffector.contactFlag_estimate.resize(numberOfEndEffector_,false);
  }

  ObservationsBase(const ObservationsBase &other)
    : numberOfActuator_(other.numberOfActuator_),
      numberOfEndEffector_(other.numberOfEndEffector_),
      actuator(other.actuator), 
      odom(other.odom), 
      imu(other.imu),
      battery(other.battery), 
      endEffector(other.endEffector),
      system(other.system),
      sensor(other.sensor) {}

  ObservationsBase &operator=(const ObservationsBase &other) {
    if (this != &other) {
      numberOfActuator_ = other.numberOfActuator_;
      numberOfEndEffector_ = other.numberOfEndEffector_;

      actuator = other.actuator;
      odom = other.odom;
      imu = other.imu;
      battery = other.battery;
      endEffector = other.endEffector;
      system = other.system;
      sensor = other.sensor;
    }
    return *this;
  }

  ~ObservationsBase() {}

public:
  int numberOfActuator_ = 12;
  int numberOfEndEffector_ = 4;

  std::vector<Actuator> actuator;
  Odom odom;
  Imu imu;
  Battery battery;
  EndEffector endEffector;
  System system;
  Sensor sensor;
};

class CommunicationConfiguration {
public:
  enum class Parity {
    NONE,
    ODD,
    EVEN,
    MARK,
    SPACE
  };
  enum class FlowControl {
    NONE,
    SOFTWARE,
    HARDWARE,
  };
  enum class StopBits {
    ONE,
    TWO,
    ONEPOINTFIVE
  };
  enum class DataBits {
    BITS_5,
    BITS_6,
    BITS_7,
    BITS_8,
  };
  enum class CommType {
    UDP,
    TCP,
    BLUETOOTH,
    SERIAL,
    None,
  };
  enum class CommProtocol{
    Plugin,
    JSON,
    Float,
    Raw,
  };

  struct UDP {
    QList<int> listenHistory = {};
    QList<QString> ipHistory = {};
    QList<int> portHistory = {};

    int listen = 26666;
    QString ip = "127.0.0.1";
    int port = 25555;
  };
  struct TCP {
    QList<int> listenHistory = {};
    QList<QString> ipHistory = {};
    QList<int> portHistory = {};

    int server = 0;

    int listen = 25555;
    QString ip = "127.0.0.1";
    int port = 26666;
  };
  struct Serial {
    QString serialName = "";
    int baudRate = 9600;
    Parity parity = Parity::NONE;
    FlowControl flowControl = FlowControl::NONE;
    StopBits stopBits = StopBits::ONE;
    DataBits dataBits = DataBits::BITS_8;
  };

  inline static QString commTypeToQString(CommType type) {
    switch (type) {
    case CommType::UDP:
      return "UDP";
    case CommType::TCP:
      return "TCP";
    case CommType::BLUETOOTH:
      return "Bluetooth";
    case CommType::SERIAL:
      return "Serial";
    default:
      return "None";
    }
  }

  inline static QString commProtocolToQString(CommProtocol type) {
    switch (type) {
    case CommProtocol::Plugin:
      return "Plugin";
    case CommProtocol::JSON:
      return "JSON";
    case CommProtocol::Float:
      return "Float";
    case CommProtocol::Raw:
      return "Raw";
    default:
      return "Unknown";
    }
  }

  UDP udp;
  TCP tcp;
  Serial serial;
  CommType commType = CommType::UDP;
  CommProtocol commProtocol = CommProtocol::Plugin;
};

enum class AngleUnit {
  Degree = 0,  // 角度（度）
  Radian = 1,  // 弧度
};

class DisplayConfiguration {
public:
  int getDt(){
    return 1000 / farmRate;
  }
  void setAngleUnit(AngleUnit v){
    angleUnit = v;
    degRad = ((angleUnit == AngleUnit::Radian) ? " rad" : " deg");
    degRadPerSecond_ = degRad + "/s";
    degRadScale = ((angleUnit == AngleUnit::Radian) ? 1.0 : Rad2Deg);
  }
  AngleUnit getAngleUnit() { return angleUnit; }
  float getAngleScale() const { return degRadScale; }
  const QString &getDegRad() const { return degRad; }
  const QString &getDegRadPerSecond() const { return degRadPerSecond_; }

public:
  int spacing = 5;
  int farmRate = 2;
  int precision = 2;
  int contentsMargins = 5;

  AngleUnit angleUnit = AngleUnit::Degree;

private:
  float degRadScale = 1.0f;
  QString degRad;
  QString degRadPerSecond_;
};

class CardConfiguration {
public:
  bool odom = false;
  bool bms = false;
  bool imu = false;
  bool system = false;
  bool command = false;

  int windowsWidth = 1000;
  int windowsHeight = 420;
};

class ActionConfiguration{
  struct Limit_t{
    scalar_t lower = 0.0;
    scalar_t upper = 0.0;
  };
public:
  Limit_t pos_limit[3] =   {{(scalar_t) - 100, (scalar_t)100},   {(scalar_t)-100, (scalar_t)100},   {(scalar_t)-100, (scalar_t)100}};    // x y z
  Limit_t vel_limit[3] =   {{(scalar_t)-1.0, (scalar_t)1.0},   {(scalar_t)-0.5, (scalar_t)0.5},   {(scalar_t)-1.0, (scalar_t)1.0}};    // x y z
  Limit_t euler_limit[3] = {{(scalar_t)-1.57, (scalar_t)1.57}, {(scalar_t)-1.57, (scalar_t)1.57}, {(scalar_t)-15.7, (scalar_t)15.7}};  // r p y
  Limit_t omega_limit[3] = {{(scalar_t)-1.57, (scalar_t)1.57}, {(scalar_t)-1.57, (scalar_t)1.57}, {(scalar_t)-1.57, (scalar_t)1.57}};  // r p y

  scalar_t pos_key[3] = { (scalar_t)0.0, (scalar_t)0.0, (scalar_t)0.0};
  scalar_t vel_key[3] = { (scalar_t)1.0,(scalar_t)0.5, (scalar_t)2.0};
  scalar_t euler_key[3] = { (scalar_t)0.0, (scalar_t)0.0, (scalar_t)0.0};
  scalar_t omega_key[3] = { (scalar_t)0.0, (scalar_t)0.0, (scalar_t)2.0};
};

class FilePathConfiguration {
public:
  QString host = "localhost";
  QString username = "root";
  QString password = "root";
  QString path = "";
};

enum class PlotLineType{
  Line,
  Point,
  LinePoint,
};

class PlotConfiguration {
public:
  bool yAutoScale = true; // Y轴自动缩放
  bool xAutoScale = true; // X轴自动缩放
  bool settingsVisible = false;    // 更多显示
  bool legend = true;
  bool link = true;
  bool tracker = true;
  bool isPaused = false;
  bool gridLine = true;

  PlotLineType plotLine = PlotLineType::Line;   // 绘图模式，可以是枚举或整数表示不同的模式
  int cacheDuration = 10;       // 数据量缓存窗口，以秒或其他时间单位表示
  int plotSamplingRate = 8;   // 采样率
  int plotFlushRate = 20;       // 绘图帧率（Frames Per Second）
};

enum class EncodingType{
  Abc,
  Hex,
};

class TerminalConfiguaration{
public:
  EncodingType outputType = EncodingType::Abc;
  EncodingType inputType = EncodingType::Abc;
};

//运行中的全局暂存配置，不保存到配置文件
class Runtime {
public:
  struct Worker{
    Worker(int num_threads = 4)
    : executor(num_threads){
    }
    tf::Executor executor;  // 固定4个线程
    tf::Taskflow taskflow;
  };

  scalar_t currentPlotFrameRate;
  std::shared_ptr<Worker> worker;
  
};

class MainAppConfiguration {
public:

  bool antiAliasing = true;
  int fontPointSize = 10;
  int maxWorkerThread = 4;
  QString appName = "Robotic User Interface";
  QString fontFamily = "NotoSansSC";
  QString pluginName = "DefaultRobot";
  QString language = "zh_CN";
  QString pageName = "Info";
};

class StreamSolverConfiguration {
public:
  bool timestampEnable_float = true;

  bool timestampEnable_json = true;
  QString timestampString_json = "timestamp";
};

class Configuration {
public:
  DisplayConfiguration display;
  CardConfiguration card;
  CommunicationConfiguration comm;
  ActionConfiguration action;
  PlotConfiguration plot;
  TerminalConfiguaration terminal;
  MainAppConfiguration app;
  StreamSolverConfiguration stream;
  Runtime runtime;

  QVector<std::shared_ptr<FilePathConfiguration>> filePaths;
};

class ColorScheme {
public:
  enum ColorSchemeEnum {
    Blue,
    Red,
    Yellow,
    Purple,    
    Indigo,
    LightBlue,
    Cyan,
    Teal,
    Green,
    LightGreen,
    Lime,
    Pink,
    Amber,
    Orange,
    White,
    DeepOrange,
    DeepPurple,
    Brown,
    Grey,
    BlueGrey,
    NumColors
  };

  static QColor getColor(ColorSchemeEnum e) {
    switch (e) {
    case White:       return QColor(250, 250, 250);
    case Red:         return QColor(244, 67, 54);
    case Pink:        return QColor(233, 30, 99);
    case Purple:      return QColor(156, 39, 176);
    case DeepPurple:  return QColor(103, 58, 183);
    case Indigo:      return QColor(63, 81, 181);
    case Blue:        return QColor(33, 150, 243);
    case LightBlue:   return QColor(3, 169, 244);
    case Cyan:        return QColor(0, 188, 212);
    case Teal:        return QColor(0, 150, 136);
    case Green:       return QColor(76, 175, 80);
    case LightGreen:  return QColor(139, 195, 74);
    case Lime:        return QColor(205, 220, 57);
    case Yellow:      return QColor(255, 235, 59);
    case Amber:       return QColor(255, 193, 7);
    case Orange:      return QColor(255, 152, 0);
    case DeepOrange:  return QColor(255, 87, 34);
    case Brown:       return QColor(121, 85, 72);
    case Grey:        return QColor(158, 158, 158);
    case BlueGrey:    return QColor(96, 125, 139);
    default:          return QColor(0, 0, 0); // fallback 黑色
    }
  }

  static QColor getRandomColor() {
    int randomIndex = QRandomGenerator::global()->bounded(ColorScheme::NumColors);
    return getColor(static_cast<ColorSchemeEnum>(randomIndex));
  }

  static QColor getColor(int index) {
    int wrappedIndex = index % ColorScheme::NumColors;
    if (wrappedIndex < 0)
      return getColor(static_cast<ColorSchemeEnum>(0));

    return getColor(static_cast<ColorSchemeEnum>(wrappedIndex));
  }
};
