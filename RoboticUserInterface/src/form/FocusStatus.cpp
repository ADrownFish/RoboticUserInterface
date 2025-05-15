#include "robotic_user_interface/form/FocusStatus.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <chrono>   
#include <cstdint>  

StatusItem::StatusItem(QWidget* parent) : QWidget(parent) {
  backgrounds_[enableBackground] = QColor(200, 200, 200, 50);
}
StatusItem::~StatusItem() {

}

void StatusItem::setBackgroundColor(uint32_t key){
  enableBackground = key;
  update();
}

void StatusItem::setBackgroundColor(uint32_t key, const QColor& color){
  backgrounds_[key] = color;
  update();
}

void StatusItem::setSvg(uint32_t key, const QString& res) {
  if (!svgs_.contains(key)) {
    svgs_[key] = std::make_shared<QSvgRenderer>();
  }
  svgs_[key]->load(res);
  resetFixedWidth();
}
void StatusItem::setDescribe(uint32_t key, const QString& str) {
  texts_[key] = str;
  resetFixedWidth();
}

void StatusItem::clearDescribe(){
  texts_.clear();
}

void StatusItem::setBorderRadius(int value) {
  borderRadius_ = value;
}
int StatusItem::getBorderRadius() {
  return borderRadius_;
}

void StatusItem::setInterval(int value) {
  int maxValue = qMin(width() / 2, height() / 2);
  if (value > maxValue) {
    interval = maxValue;
  }
  else {
    interval = value;
  }

  update();
}

int StatusItem::getInterval(){
  return interval;
}

void StatusItem::setDisplayMode(DisplayMode value)
{
  mode = value;
  update();
}

StatusItem::DisplayMode StatusItem::getDisplayMode()
{
  return mode;
}

void StatusItem::resetFixedWidth() {
  if (mode == DisplayMode::normal) {
    int pixmapWidth = height() - interval;
    int fixedWidth = 0;
    fixedWidth += svgs_.size() * pixmapWidth;

    QFontMetrics fontMetrics(font());
    for (const auto& str : texts_.values()) {
      fixedWidth += interval;
      fixedWidth += fontMetrics.horizontalAdvance(str) * 1.1;
    }
    fixedWidth += interval;
    setFixedWidth(fixedWidth);
  } else {
    int pixmapWidth = height() - interval;
    int fixedWidth = 0;
    int maxTextWidth = 0;

    QFont drawfont = font();
    drawfont.setPointSizeF(drawfont.pointSize() / 2.);
    QFontMetrics fontMetrics(drawfont);

    fixedWidth += interval;
    fixedWidth += pixmapWidth;

    for (const auto& str : texts_.values()) {
      maxTextWidth = qMax(fontMetrics.horizontalAdvance(str) * 1.1, (double)maxTextWidth);
    }

    fixedWidth += interval;
    fixedWidth += maxTextWidth;
    fixedWidth += interval;
    setFixedWidth(fixedWidth);
  }

  update();
}

void StatusItem::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event)

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QPainterPath path;
  path.addRoundedRect(QRectF(0, 0, width(), height()), borderRadius_, borderRadius_);

  painter.setPen(Qt::NoPen);
  painter.setBrush(backgrounds_[enableBackground]);
  painter.drawPath(path);

  if (mode == DisplayMode::normal) {
    int svgWidth = height() - 2 * interval;
    QRect svgRect(interval, interval, svgWidth, svgWidth);

    for (auto& svg : svgs_) {
      svg->render(&painter, svgRect);
      svgRect.translate(svgWidth + interval, 0);
    }
    painter.setPen(Qt::white);
    QFontMetrics fontMetrics(font());
    for (const auto& str : texts_.values()) {
      int fontWidth = fontMetrics.horizontalAdvance(str) * 1.1;
      svgRect.setWidth(fontWidth);
      painter.drawText(svgRect, Qt::AlignCenter, str);
      svgRect.translate(fontWidth + interval, 0);
    }
  }  else {
    // 计算可用的上下总高度，再平均到每个 svg
    int svgHeightArea = height() - 2 * interval;
    int count = svgs_.size() > 0 ? svgs_.size() : 1;
    int svgWidth = svgHeightArea / count;

    QRect svgRect(interval, interval, svgWidth, svgWidth);

    for (auto& svg : svgs_) {
      svg->render(&painter, svgRect);
      svgRect.translate(0, svgWidth + interval);
    }
    svgRect.moveTo(svgRect.x() + svgWidth + interval, interval);
    QFont drawfont = font();
    drawfont.setPointSizeF(drawfont.pointSize() / 1.5);
    painter.setPen(Qt::white);
    painter.setFont(drawfont);
    QFontMetrics fontMetrics(drawfont);
    for (const auto& str : texts_.values()) {
      int fontWidth = fontMetrics.horizontalAdvance(str) * 1.1;
      svgRect.setWidth(fontWidth);
      painter.drawText(svgRect, Qt::AlignCenter, str);
      svgRect.translate(0, svgWidth + interval);
    }
  }
 
}
// 大小改变事件
void StatusItem::resizeEvent(QResizeEvent* event)  {
  update(); 
}

FocusStatus::FocusStatus(QObject*parent)
: QObject(parent){
  //ui.setupUi(this);

  connect(&timer_flush_, &QTimer::timeout, this, &FocusStatus::flush);

  timer_flush_.start(1000);
  //ui.button_nav->setText(QString("Nav"));
  //ui.button_nav->setIcon(QIcon(":/svg/svg/menu.svg"));

  //ui.widget_net_switcher->setToggle(false);
  //ui.widget_record_switcher->setToggle(false);

  createStatusItems();

  setCommStatus(false);
}
FocusStatus::~FocusStatus(){


}

void FocusStatus::setConfiguration(std::shared_ptr<Configuration> config){
  config_ = config;
}

void FocusStatus::setObservations(std::shared_ptr<ObservationsBase> config){
  observations_ = config;
}

void FocusStatus::appedUploadBytes(int byte){
  value_.uploadBytes += byte;
}
void FocusStatus::appendDownloadByte(int byte){
  value_.downloadBytes += byte;
}

void FocusStatus::createStatusItems() {
  itemsWidget = new QWidget();

  QHBoxLayout* layout = new QHBoxLayout();
  layout->setSpacing(5);
  layout->setContentsMargins(20, 0, 20, 0);
  itemsWidget->setLayout(layout);

  StatusItem *batterySoc = new StatusItem();
  StatusItem *netType = new StatusItem();
  StatusItem *netSpeed = new StatusItem();
  StatusItem *tempDriver = new StatusItem();
  StatusItem *tempMotor = new StatusItem();

  auto initStatusItem = [this, layout](StatusItem* item) {
    item->setFixedHeight(28);
    item->setBorderRadius(8);
    item->setInterval(2);
    item->setBackgroundColor(0, QColor(200, 200, 200, 10));
    layout->addWidget(item);
    };

  initStatusItem(netType);
  initStatusItem(netSpeed);
  initStatusItem(tempDriver);
  initStatusItem(tempMotor);
  initStatusItem(batterySoc);

  batterySoc->setSvg(0, ":/svg/svg/battery.svg");
  batterySoc->setDescribe(0, "0 %");

  netType->setSvg(0, ":/svg/svg/network.svg");
  netType->setDescribe(0, "NONE");
  netType->setBackgroundColor(1, QColor(130, 180, 129));

  netSpeed->setDisplayMode(StatusItem::DisplayMode::vertical);
  netSpeed->setInterval(1);
  netSpeed->setSvg(0, ":/svg/svg/arrow-up.svg");
  netSpeed->setDescribe(0, formatByteRateUnit(0));
  netSpeed->setSvg(1, ":/svg/svg/arrow-down.svg");
  netSpeed->setDescribe(1, formatByteRateUnit(0));

  tempDriver->setSvg(0, ":/svg/svg/temperature.svg");
  tempDriver->setDescribe(0, "Driver");
  tempDriver->setDescribe(1, "0");
  tempDriver->setDescribe(2, "℃");
  tempDriver->setBackgroundColor(1, QColor(200, 100, 100));

  tempMotor->setSvg(0, ":/svg/svg/temperature.svg");
  tempMotor->setDescribe(0, "Motor");
  tempMotor->setDescribe(1, "0");
  tempMotor->setDescribe(2, "℃");
  tempMotor->setBackgroundColor(1, QColor(200, 100, 100));

  items[StatusItemEnum::batterySoc] = batterySoc;
  items[StatusItemEnum::netType] = netType;
  items[StatusItemEnum::netSpeed] = netSpeed;
  items[StatusItemEnum::tempDriver] = tempDriver;
  items[StatusItemEnum::tempMotor] = tempMotor;
}

void FocusStatus::flush(){
  using namespace std::chrono;
  double currentTime =  static_cast<uint64_t>(duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count()) / 1000'000'000.;
  scalar_t dt = currentTime - value_.time;
  value_.time = currentTime;
  if(value_.time == 0)  return; 

  scalar_t speed_up =   (value_.uploadBytes - value_.lastUploadBytes) / dt;
  scalar_t speed_down = (value_.downloadBytes - value_.lastDownloadBytes) / dt;
  value_.lastUploadBytes   = value_.uploadBytes;
  value_.lastDownloadBytes = value_.downloadBytes;
  
  scalar_t maxTempMotor = 0.f;
  scalar_t maxTempDriver = 0.f;
  for (size_t i = 0; i < observations_->numberOfActuator_; i++){
    maxTempMotor  = qMax(maxTempMotor, observations_->actuator[i].temperature);
    maxTempDriver = qMax(maxTempDriver,observations_->actuator[i].driverTemperature);
  }
  

  //ui.batteryLevel->setText(formatBatteryUnit(observations_->battery.soc));
  //ui.network_download->setText(QString("↓ ") + formatByteUnit(speed_down));
  //ui.network_upload->setText(QString("↑ ") + formatByteUnit(speed_up));
  //ui.maxTemp_Motor->setText(QString("M %1 ℃").arg(QString::number(maxTempMotor, 'f', 2)));
  //ui.maxTemp_Driver->setText(QString("D %1 ℃").arg(QString::number(maxTempDriver, 'f', 2)));

  items[batterySoc]->setDescribe(0, formatBatteryUnit(observations_->battery.soc));
  items[netSpeed]->setDescribe(0, formatByteRateUnit(speed_up));
  items[netSpeed]->setDescribe(1, formatByteRateUnit(speed_down));
  items[tempDriver]->setDescribe(1, QString::number(maxTempDriver, 'f', 2));
  items[tempMotor]->setDescribe(1, QString::number(maxTempMotor, 'f', 2));

  if (maxTempDriver < 70) {
    items[tempDriver]->setBackgroundColor(0);
  }  else {
    items[tempDriver]->setBackgroundColor(1);
  }

  if (maxTempMotor < 75) {
    items[tempMotor]->setBackgroundColor(0);
  }  else {
    items[tempMotor]->setBackgroundColor(1);
  }
}

QString FocusStatus::formatByteRateUnit(qint64 bytes) {
  if (bytes < 1024) {
    return QString::number(bytes) + " B/s"; // 小于1KB
  } else if (bytes < 1024 * 1024) {
    return QString::number(bytes / 1024.0, 'f', 2) + " KB/s"; // 小于1MB
  } else if (bytes < 1024 * 1024 * 1024) {
    return QString::number(bytes / (1024.0 * 1024), 'f', 2) + " MB/s"; // 小于1GB
  } else {
    return QString::number(bytes / (1024.0 * 1024 * 1024), 'f', 2) +
           " GB/s"; // 大于1GB
  }
}

QString FocusStatus::formatBatteryUnit(double level) {
  return QString::number(level, 'f' , 2) + " %";
}

void FocusStatus::flushConfiguration(){

}

void FocusStatus::setCommStatus(bool status) {
  if (status) {
    items[netType]->setDescribe(0, CommunicationConfiguration::commTypeToQString(config_->comm.commType));
    items[netType]->setBackgroundColor(1);
  } else {
    items[netType]->setDescribe(0, CommunicationConfiguration::commTypeToQString(CommunicationConfiguration::CommType::None));
    items[netType]->setBackgroundColor(0);
  }
}

QWidget* FocusStatus::getStatusItemsWidget()
{
  return itemsWidget;
}
