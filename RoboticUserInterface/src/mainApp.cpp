#include <QApplication>
#include <QFontDatabase>
#include <QProcess>

#include <iostream>

#include "robotic_user_interface/RobotUserInterface.h"
#include "robotic_user_interface/custom/DefaultRobot.h"
#include "robotic_user_interface/custom/SevnceRobot.h"

#include "../test/QWidgetPrototypeTester.h"

#include "FluUtils/FluUtils.h"


#define PROTOTYPE_TESTER false

std::function<void()> hookFunction;

#ifdef _WIN32
#include <windows.h>
BOOL WINAPI ConsoleHandler(DWORD signal) {
  if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) {
    hookFunction();
    return TRUE;
  }
  return FALSE;
}

#else
#include <csignal>
void handleSignal(int signum) {
  hookFunction();
}
#endif

int main(int argc, char *argv[]){

  /*********** 环境设置 ***********/
#if defined(Q_OS_ANDROID)
    QApplication::setAttribute(Qt::AA_UseOpenGLES);
    // QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
#else
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

  qputenv("QT_ANDROID_ENABLE_WORKAROUND_TO_DISABLE_PREDICTIVE_TEXT", "1");
  qputenv("QT_LOGGING_RULES", "qt.qpa.*=false"); // 减少日志输出

  /*********** 打印参数 ***********/
  for (int i = 0; i < argc; i++)  {
    std::cout << " >> " << argv[i] << std::endl;
  }

  QApplication a(argc, argv);
  a.setWindowIcon(QIcon(":/log/logo/main.svg"));
  FluLogUtils::__init();
  InitialConfiguration initCfg = ConfigManager::getInitialConfiguration();

  /*********** 加载翻译 ***********/
  QTranslator translator;
  //QString locale = QLocale::system().name(); // 如 zh_CN
   QString qmPath = QString(":/translations/%1.qm").arg(initCfg.language);
  //QString qmPath = QString(":/translations/zh_CN.qm");
  if (translator.load(qmPath)) {
    a.installTranslator(&translator);
  }

  /*********** 加载字体 ***********/
  QString FontName(":/libraries/fonts/NotoSansSC.ttf");
  Q_INIT_RESOURCE(resources);
  int fontId = QFontDatabase::addApplicationFont(FontName);
  if (fontId != -1) {
    auto fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QString fontFamily = fontFamilies.at(0);
    QFont defaultFont(fontFamily);
    defaultFont.setPointSize(initCfg.fontSize);
    QApplication::setFont(defaultFont);

    qDebug() << QString("%2[%1] has been loaded: ").arg(initCfg.fontSize).arg(FontName) << defaultFont;
  }

  /*********** 加载主类 ***********/
#if PROTOTYPE_TESTER == true
  QWidgetPrototypeTester w;
  w.show();
#else
  RobotUserInterface w(argc, argv);

  /*********** 加载插件 ***********/
  QSharedPointer<RobotBase> robot;
  if(initCfg.pluginName == "SevnceRobot"){
    robot = QSharedPointer<sevnce::SevnceRobot>::create();
    robot->init();
  } else {
    robot = QSharedPointer<robot::DefaultRobot>::create();
    robot->init();
  }

  w.setRobotBase(robot.get());
  w.init();
  w.show();

  hookFunction = [&]() {
    w.shutdown();
  };
#endif


  // ********** 捕获退出信号 **********
#ifdef _WIN32
  SetConsoleCtrlHandler(ConsoleHandler, TRUE);
#else 
  std::signal(SIGINT, handleSignal);     // Ctrl+C
  std::signal(SIGTERM, handleSignal);  // kill
  std::signal(SIGHUP, handleSignal);    // 控制台挂起
#endif

  int code =  a.exec();
  if(code == 666){
    // 如果需要重启
    QProcess::startDetached(qApp->applicationFilePath());
  }
  return code;
}



