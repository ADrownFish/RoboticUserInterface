#include <QCoreApplication>
#include <QUdpSocket>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <random>
#include <cmath>
#include <array>
#include <cstring>

#include "SevnceTypes.h"
#include "DataPacketSolver.h"

using namespace sevnce::high;

class UdpSender : public QObject {
  Q_OBJECT
  using Solver = robot::DataPacketSolver<10240, 10>;
  using SData = Solver::Data;
public:
  UdpSender(QObject* parent = nullptr) : QObject(parent), socket(new QUdpSocket(this)), dist(-0.05, 0.05) {
    //socket->connectToHost("127.0.0.1", 8888);
    qDebug() << "start to sent HighState of size: " << sizeof(HighState);
    connect(&timer, &QTimer::timeout, this, &UdpSender::sendData);
    timer.start(2); // 10Hz

    startTime = QDateTime::currentMSecsSinceEpoch() / 1000.0;

    solver.setDeviceID(2);
  }

private slots:
  void sendData() {
    HighState state = generateState();

    SData d;
    
    d.setHead(0x01,0x01);
    d.appendData(state);
    auto p = solver.makeDataPacket(d);

    socket->writeDatagram(reinterpret_cast<const char*>(p.data), p.size, QHostAddress("127.0.0.1"),26666);
    //qDebug() << "Sent HighState of size:" << sizeof(HighState);
  }

private:
  QUdpSocket* socket;
  QTimer timer;
  std::default_random_engine rng{ std::random_device{}() };
  std::uniform_real_distribution<float> dist;
  double startTime;
  Solver solver;

  HighState generateState() {
    HighState hs{};
    double t = QDateTime::currentMSecsSinceEpoch() / 1000.0 - startTime;

    // 模拟速度：正弦+噪声
    hs.velocity.xVel = std::sin(t) + dist(rng);
    hs.velocity.yVel = std::cos(t) + dist(rng);
    hs.velocity.zVel = 0.2f * std::sin(0.5 * t);

    hs.velocity.rollVel = 0.1f * std::cos(0.3 * t);
    hs.velocity.pitchVel = 0.1f * std::sin(0.3 * t);
    hs.velocity.yawVel = 0.1f * std::cos(0.6 * t);

    // IMU 模拟：欧拉角+线加速度+角速度
    for (int i = 0; i < 3; ++i) {
      hs.imu.rpy[i] = 0.1f * std::sin(t + i) + dist(rng);
      hs.imu.linearAcc[i] = 9.8f * ((i == 2) ? 1 : 0) + dist(rng);  // z方向保持重力加速度
      hs.imu.angularVel[i] = 0.01f * std::cos(t + i) + dist(rng);
    }
    hs.imu.quat = { 1, 0, 0, 0 };  // 简化处理
    hs.imu.timeStamp = QDateTime::currentMSecsSinceEpoch();
    hs.imu.temp = 36 + static_cast<int8_t>(5 * dist(rng));

    // 关节数据
    for (int i = 0; i < 12; ++i) {
      hs.joints[i].q = 0.5f * std::sin(t + i * 0.1f);
      hs.joints[i].dq = 0.1f * std::cos(t + i * 0.1f);
      hs.joints[i].tau = 0.05f * std::sin(t + i * 0.2f);
      hs.joints[i].motorFault = 0;
      hs.joints[i].motorCurrent = 1.0f + dist(rng);
      hs.joints[i].motorVoltage = 24.0f + dist(rng);
      hs.joints[i].motorTemp = 65 + static_cast<int8_t>(5 * dist(rng) * 100);
      hs.joints[i].driverTemp = 55 + static_cast<int8_t>(5 * dist(rng) * 100);
    }

    return hs;
  }
};