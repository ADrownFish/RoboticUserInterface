#include "robotic_user_interface/form/SettingsDisplay.h"

#include "qwool/qwdropwidget.h"
#include "FluControls/FluMenu.h"
#include "FluControls/FluAction.h"

#include <QSvgRenderer>

#include "BuildInfo.h"

SettingsDisplay::SettingsDisplay(QWidget *parent)
: QWWindowWidget(parent){
  ui.setupUi(this);
}

SettingsDisplay::~SettingsDisplay(){
  
}


void SettingsDisplay::init(){
  setupWidgetsControls();
  setupSignalConnection();
}


void SettingsDisplay::setConfiguration(std::shared_ptr<Configuration> config){
  config_ = config;

}


void SettingsDisplay::SettingsDisplay::setupSignalConnection(){
  connect(ui.lineEdit_farmRate, &QLineEdit::editingFinished, this, [=]() {
      int value = ui.lineEdit_farmRate->text().toInt();
      if (value < 1) value = 1;
      if (value > 60) value = 60;
      ui.lineEdit_farmRate->setText(QString::number(value));
  });
  connect(ui.lineEdit_precision, &QLineEdit::editingFinished, this, [=]() {
      int value = ui.lineEdit_precision->text().toInt();
      if (value < 1) value = 1;
      if (value > 6) value = 6;
      ui.lineEdit_precision->setText(QString::number(value));
  });
}


void SettingsDisplay::setupWidgetsControls(){
  
  QColor bgColor(32, 32, 32);

  setBorderRadius(8);
  setBackgroundColor(bgColor);

  ui.widget_degRad->addUnit(tr("Deg"));
  ui.widget_degRad->addUnit(tr("Rad"));
  ui.widget_degRad->setBackgroundColor(QColor(100,100,100,50));

  ui.widget_language->addUnit(tr("Chinese"));
  ui.widget_language->addUnit("English");
  ui.widget_language->setBackgroundColor(QColor(100, 100, 100, 50));

  ui.widget_font->addUnit(tr("NotoSansSC"));
  ui.widget_font->setBackgroundColor(QColor(100, 100, 100, 50));

  ui.widget_font_point_size->addUnit(tr("8"));
  ui.widget_font_point_size->addUnit(tr("9"));
  ui.widget_font_point_size->addUnit(tr("10"));
  ui.widget_font_point_size->addUnit(tr("11"));
  ui.widget_font_point_size->addUnit(tr("12"));
  ui.widget_font_point_size->addUnit(tr("13"));
  ui.widget_font_point_size->addUnit(tr("14"));
  ui.widget_font_point_size->addUnit(tr("15"));
  ui.widget_font_point_size->setBackgroundColor(QColor(100, 100, 100, 50));

  ui.widget_PlotAntiAliasing->addUnit(tr("False"));
  ui.widget_PlotAntiAliasing->addUnit(tr("True"));
  ui.widget_PlotAntiAliasing->setBackgroundColor(QColor(100,100,100,50));

  ui.widget_LeggedTopNodes->addUnit(tr("False"));
  ui.widget_LeggedTopNodes->addUnit(tr("True"));
  ui.widget_LeggedTopNodes->setBackgroundColor(QColor(100,100,100,50));

  ui.textEdit->setText(BuildInfo::getBriefInfo());
  ui.textEdit->setReadOnly(true);               // 设置只读
  ui.textEdit->setOpenExternalLinks(true);      // 启用外部超链接
  ui.textEdit->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui.textEdit->setHtml(BuildInfo::getBriefInfo());  // 设置HTML内容

  ui.lineEdit_farmRate->setValidator(new QIntValidator(1, 60, this));
  ui.lineEdit_precision->setValidator(new QIntValidator(1, 6, this));

  ui.lineEdit_pluginName->setReadOnly(true);

  QWDropWidget *drop_plugin = new QWDropWidget();
  drop_plugin->setDropIcon(QIcon(":/svg/svg/arrow-right2.svg"));
  drop_plugin->setFixedSize(300 + 24, 50);
  ui.gridLayout_app->replaceWidget(ui.lineEdit_pluginName, drop_plugin);
  drop_plugin->setWidget(ui.lineEdit_pluginName);
  FluMenu *menu = new FluMenu();
  // 暂定有这些
  QStringList plugins = {"DefaultRobot","SevnceRobot"};
  for(auto & plugin : plugins){
    FluAction *action = new FluAction(plugin);
    menu->addAction(action);
    QObject::connect(action, &FluAction::triggered, [this, action]() {
      ui.lineEdit_pluginName->setText(action->text());
    });
  }
  drop_plugin->setMenu(menu);

  QSvgRenderer renderer(QString(":/log/logo/main.svg"));
  QPixmap pixmap(ui.label_icon->size());
  pixmap.fill(Qt::transparent); // 透明背景
  QPainter painter(&pixmap);
  renderer.render(&painter);
  ui.label_icon->setPixmap(pixmap);
  ui.label_icon->setScaledContents(true); // 允许缩放
}


void SettingsDisplay::pushParameters(){
  ui.widget_degRad->setSelectUnitIndex(static_cast<int>(config_->display.angleUnit));
  ui.lineEdit_farmRate->setText(QString::number(config_->display.farmRate));
  ui.lineEdit_precision->setText(QString::number(config_->display.precision));

  ui.lineEdit_appName->setText(config_->app.appName);
  ui.lineEdit_pluginName->setText(config_->app.pluginName);
  ui.widget_font_point_size->setSelectUnitIndex(toFontUnitIndex(config_->app.fontPointSize));
  ui.widget_language->setSelectUnitIndex(toLanguageIndex(config_->app.language));

  ui.widget_PlotAntiAliasing->setSelectUnitIndex(config_->plot.antiAliasing?1:0);
  ui.widget_LeggedTopNodes->setSelectUnitIndex(config_->plot.leggedTopNode?1:0);
}

void SettingsDisplay::pullParameters(){
  config_->display.setAngleUnit(static_cast<AngleUnit>(ui.widget_degRad->getCurrentUnitIndex()));
  config_->display.farmRate = ui.lineEdit_farmRate->text().toFloat();
  config_->display.precision = ui.lineEdit_precision->text().toFloat();

  config_->app.appName = ui.lineEdit_appName->text();
  config_->app.pluginName = ui.lineEdit_pluginName->text();
  config_->app.fontPointSize = toFontPointSize(ui.widget_font_point_size->getCurrentUnitIndex());
  config_->app.language = toLanguageString(ui.widget_language->getCurrentUnitIndex());
  
  config_->plot.antiAliasing = ui.widget_PlotAntiAliasing->getCurrentUnitIndex();
  config_->plot.leggedTopNode = ui.widget_LeggedTopNodes->getCurrentUnitIndex();
}

int SettingsDisplay::toFontPointSize(int value){
  switch (value) {
    case 0:
      return 8;
    case 1:
      return 9;
    case 2:
      return 10;
    case 3:
      return 11;
    case 4:
      return 12;
    case 5:
      return 13;
    case 6:
      return 14;
    case 7:
      return 15;
  }

  return 10;
}
int SettingsDisplay::toFontUnitIndex(int value){
  switch (value) {
    case 8:
      return 0;
    case 9:
      return 1;
    case 10:
      return 2;
    case 11:
      return 3;
    case 12:
      return 4;
    case 13:
      return 5;
    case 14:
      return 6;
    case 15:
      return 7;
  }

  return 0;
}

int SettingsDisplay::toLanguageIndex(const QString& value){

  if(value == "zh_CN"){
    return 0;
  } else if(value == "en_US"){
    return 1;
  }

  return 0;
}

QString SettingsDisplay::toLanguageString(int value){
  switch (value) {
    case 0:
      return "zh_CN";
    case 1:
      return "en_US";
  }
  return "zh_CN";
}