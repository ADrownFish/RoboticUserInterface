#include <QApplication>
#include <QFontDatabase>
// #include <QtCore>
// #include <QtCore/qglobal.h>

#include <iostream>

#include "robotic_user_interface/RobotUserInterface.h"
#include "robotic_user_interface/custom/SevnceRobot.h"

#include "../test/QWidgetPrototypeTester.h"

#include "FluUtils/FluUtils.h"

// #define PROTOTYPE_TESTER true
#define PROTOTYPE_TESTER false



int main(int argc, char *argv[]){

    // Q_INIT_RESOURCE(RUI);

  // 环境设置
#if defined(Q_OS_ANDROID)
    QApplication::setAttribute(Qt::AA_UseOpenGLES);
    // QApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
#else
    QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

    //QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif


  qputenv("QT_ANDROID_ENABLE_WORKAROUND_TO_DISABLE_PREDICTIVE_TEXT", "1");
  qputenv("QT_LOGGING_RULES", "qt.qpa.*=false"); // 减少日志输出

  // 打印参数
  for (int i = 0; i < argc; i++)  {
    std::cout << " >> " << argv[i] << std::endl;
  }

  QApplication a(argc, argv); 

  FluLogUtils::__init();

  // 加载翻译
  QTranslator translator;
  if (FluConfigUtils::getUtils()->getLanguage() == "zh-CN")  {
    bool bLoad = translator.load("../i18n/zh-cn.qm");
    if (bLoad)    {
      a.installTranslator(&translator);
    }
  }
  else  {
    bool bLoad = translator.load("../i18n/en-US.qm");
    if (bLoad)    {
      a.installTranslator(&translator);
    }
  }

  // 加载字体
  Q_INIT_RESOURCE(resources);
  int fontId = QFontDatabase::addApplicationFont(":/libraries/fonts/Roboto/NotoSansSC.ttf");
  if (fontId != -1) {
    QString fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    QFont defaultFont(fontFamily);
    QApplication::setFont(defaultFont);
  }

  // 加载主类
#if PROTOTYPE_TESTER == true
  QWidgetPrototypeTester w;
  w.show();
#else
  RobotUserInterface w(argc, argv);

  QPointer<sevnce::SevnceRobot> robot = new sevnce::SevnceRobot();
  robot->init(12, 4);
  w.setRobotBase(robot);
  w.init();
  w.show();
#endif

  return a.exec();
}



