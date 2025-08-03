#include "robotic_user_interface/form/SettingsDisplay.h"

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

  ui.widget_font_point_size->addUnit(tr("10"));
  ui.widget_font_point_size->addUnit(tr("11"));
  ui.widget_font_point_size->addUnit(tr("12"));
  ui.widget_font_point_size->addUnit(tr("13"));
  ui.widget_font_point_size->setBackgroundColor(QColor(100, 100, 100, 50));

  ui.widget_antiAliasing->addUnit(tr("False"));
  ui.widget_antiAliasing->addUnit(tr("True"));
  ui.widget_antiAliasing->setBackgroundColor(QColor(100,100,100,50));

  ui.textEdit->setText(BuildInfo::getBriefInfo());
  ui.textEdit->setReadOnly(true);               // 设置只读
  ui.textEdit->setOpenExternalLinks(true);      // 启用外部超链接
  ui.textEdit->setTextInteractionFlags(Qt::TextBrowserInteraction);
  ui.textEdit->setHtml(BuildInfo::getBriefInfo());  // 设置HTML内容

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
  ui.widget_antiAliasing->setSelectUnitIndex(config_->app.antiAliasing?1:0);
  ui.widget_font_point_size->setSelectUnitIndex(toFontUnitIndex(config_->app.fontPointSize));
  ui.widget_language->setSelectUnitIndex(toLanguageIndex(config_->app.language));
}

void SettingsDisplay::pullParameters(){
  config_->display.setAngleUnit(static_cast<AngleUnit>(ui.widget_degRad->getCurrentUnitIndex()));
  config_->display.farmRate = ui.lineEdit_farmRate->text().toFloat();
  config_->display.precision = ui.lineEdit_precision->text().toFloat();

  config_->app.appName = ui.lineEdit_appName->text();
  config_->app.pluginName = ui.lineEdit_pluginName->text();
  config_->app.antiAliasing = ui.widget_antiAliasing->getCurrentUnitIndex();
  config_->app.fontPointSize = toFontPointSize(ui.widget_font_point_size->getCurrentUnitIndex());
  config_->app.language = toLanguageString(ui.widget_language->getCurrentUnitIndex());
}

int SettingsDisplay::toFontPointSize(int value){
  switch (value) {
    case 0:
      return 10;
    case 1:
      return 11;
    case 2:
      return 12;
    case 3:
      return 13;
  }

  return 10;
}
int SettingsDisplay::toFontUnitIndex(int value){
  switch (value) {
    case 10:
      return 0;
    case 11:
      return 1;
    case 12:
      return 2;
    case 13:
      return 3;
  }

  return 0;
}

int SettingsDisplay::toLanguageIndex(const QString& value){

  if(value == "zh_CN"){
    return 0;
  } else if(value == "en_US"){
    return 1;
  }
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