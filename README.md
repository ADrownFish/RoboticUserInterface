# Robotic User Interface

**Robotic User Interface - 可定制机器人上位机框架**
**Robotic User Interface - Customizable Robotic HMI Framework**

友好的机器人交互界面，无论是哪一个机器人，继承并实现RobotBase基类中相对应的解包或组包，也可提供自定义的UI控件，会一并显示在UI中。

特点:
  - 接入 **UDP/TCP/Serial/BlueTooth** 原始数据流  
  - 内置（可选） **[DataPacketSolver](https://github.com/ADrownFish/DataPacketSolver)** 组包解包引擎  
  - 支持**调试模式**(类串口助手)和**集成模式**  

开发
  - C++ 17
  - Qt > 6.9
  - CMake > 3.20

跨平台开发
  - Windows
  - Ubuntu

目标包含多种开发工具:
  - 3D模型（TODO） 
  - 自定义加载用户插件(TODO)  
  - 曲线可视化
  - 数据收发
  - 开发工具。

可以通过不同的通信方式接入数据:
  - UDP
  - TCP（TODO）
  - BlueTooth(TODO)
  - Serial Port
  - CSV File    

![alt text](README_RES/curve)
![alt text](README_RES/main)
![alt text](README_RES/comm)
![alt text](README_RES/data)
![alt text](README_RES/imu_e)

