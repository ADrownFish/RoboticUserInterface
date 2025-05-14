#include "robotic_user_interface/form/CommSelector.h"

CommSelector::CommSelector(QWidget *parent)
  : QWWindowWidget(parent) {
  ui.setupUi(this);

  // setBorderRadius(0);
}
CommSelector::~CommSelector(){

}

void CommSelector::setupWidgetsControls(){
  setBorderRadius(0);
  setBackgroundColor(QColor(74,85,115));
  
  ui.button_ok->setText("OK");
  ui.button_cancel->setText("Cancel");

  ui.lineEdit_CommType->addUnit("UDP");
  ui.lineEdit_CommType->addUnit("TCP");
  ui.lineEdit_CommType->addUnit("Bluetooth");
  ui.lineEdit_CommType->addUnit("Serial");

  ui.lineEdit_udp_ip->setLabel("IP");
  ui.lineEdit_udp_port->setLabel("Port");
  ui.lineEdit_udp_listen->setLabel("Listen");

  ui.lineEdit_tcp_ip->setLabel("IP");
  ui.lineEdit_tcp_port->setLabel("Port");
  ui.lineEdit_tcp_listen->setLabel("Listen");

  ui.lineEdit_serial_baudrate->setLabel("Baud Rate");
  ui.lineEdit_serial_name->setLabel("SerialPort");
  ui.lineEdit_serial_stopBits->addUnit("1");
  ui.lineEdit_serial_stopBits->addUnit("2");
  ui.lineEdit_serial_stopBits->addUnit("1.5");
  ui.lineEdit_serial_dataBits->addUnit("5");
  ui.lineEdit_serial_dataBits->addUnit("6");
  ui.lineEdit_serial_dataBits->addUnit("7");
  ui.lineEdit_serial_dataBits->addUnit("8");
  ui.lineEdit_serial_parity->addUnit("None");
  ui.lineEdit_serial_parity->addUnit("Odd");
  ui.lineEdit_serial_parity->addUnit("Even");
  ui.lineEdit_serial_parity->addUnit("Mark");
  ui.lineEdit_serial_parity->addUnit("Space");
  ui.lineEdit_serial_flowControl->addUnit("None");
  ui.lineEdit_serial_flowControl->addUnit("Software");
  ui.lineEdit_serial_flowControl->addUnit("Hardware");

  ui.lineEdit_CommType->setSelectUnitIndex(static_cast<int>(config_->comm.commType));
  QString key = ui.lineEdit_CommType->getUnitName(ui.lineEdit_CommType->getCurrentUnitIndex());
  if (key == "TCP") {
    ui.stackedWidget->setCurrentWidget(ui.page_tcp);
  } else if (key == "UDP") {
    ui.stackedWidget->setCurrentWidget(ui.page_udp);
  } else if (key == "Bluetooth") {
    ui.stackedWidget->setCurrentWidget(ui.page_bluetooth);
  } else if (key == "Serial") {
    ui.stackedWidget->setCurrentWidget(ui.page_serial);
  }
}

void CommSelector::setupSignalConnection(){

  // ok cancel
  QObject::connect(ui.button_ok, &QPushButton::clicked, [this](){
    pullParameters();

    emit ok();
  });
  QObject::connect(ui.button_cancel, &QPushButton::clicked, [this](){
    pushParameters();

    emit cancel();
  });
  // page switch
  QObject::connect(ui.lineEdit_CommType, &QWWindowButton::selectUnitIndexChanged, [this](unsigned int index){
    QString key = ui.lineEdit_CommType->getUnitName(index);
    if(key == "TCP"){
      ui.stackedWidget->setCurrentWidget(ui.page_tcp);
      // emit publishNotify(GCW::NotifyType::Warning,"CommSelector","Disabled");
    } else if(key == "UDP"){
      ui.stackedWidget->setCurrentWidget(ui.page_udp);
    } else if(key == "Bluetooth"){
      ui.stackedWidget->setCurrentWidget(ui.page_bluetooth);
      // emit publishNotify(GCW::NotifyType::Warning,"CommSelector","Disabled");
    } else if(key == "Serial"){
      ui.stackedWidget->setCurrentWidget(ui.page_serial);
      // emit publishNotify(GCW::NotifyType::Warning,"CommSelector","Disabled");
    }
  });
}

void CommSelector::pushParameters(){

  ui.lineEdit_udp_ip->setText(config_->comm.udp.ip);
  ui.lineEdit_udp_port->setText(QString::number(config_->comm.udp.port));
  ui.lineEdit_udp_listen->setText(QString::number(config_->comm.udp.listen));

  ui.lineEdit_tcp_ip->setText(config_->comm.tcp.ip);
  ui.lineEdit_tcp_port->setText(QString::number(config_->comm.tcp.port));
  ui.lineEdit_tcp_listen->setText(QString::number(config_->comm.tcp.listen));

  ui.lineEdit_serial_baudrate->setText(QString::number(config_->comm.serial.baudRate));
  ui.lineEdit_serial_name->setText(config_->comm.serial.serialName);
  ui.lineEdit_serial_stopBits->setSelectUnitIndex(static_cast<int>(config_->comm.serial.stopBits));
  ui.lineEdit_serial_dataBits->setSelectUnitIndex(static_cast<int>(config_->comm.serial.dataBits));
  ui.lineEdit_serial_parity->setSelectUnitIndex(static_cast<int>(config_->comm.serial.parity));
  ui.lineEdit_serial_flowControl->setSelectUnitIndex(static_cast<int>(config_->comm.serial.flowControl));

  ui.lineEdit_CommType->setSelectUnitIndex((int)config_->comm.commType);
}

void CommSelector::pullParameters(){
  config_->comm.udp.ip = ui.lineEdit_udp_ip->text();
  config_->comm.udp.port = ui.lineEdit_udp_port->text().toInt();;
  config_->comm.udp.listen = ui.lineEdit_udp_listen->text().toInt();

  config_->comm.tcp.ip = ui.lineEdit_tcp_ip->text();
  config_->comm.tcp.port = ui.lineEdit_tcp_port->text().toInt();;
  config_->comm.tcp.listen = ui.lineEdit_tcp_listen->text().toInt();

  config_->comm.serial.baudRate = ui.lineEdit_serial_baudrate->text().toInt();
  config_->comm.serial.serialName = ui.lineEdit_serial_name->text();
  config_->comm.serial.stopBits = (CommunicationConfiguration::StopBits)ui.lineEdit_serial_stopBits->getCurrentUnitIndex();
  config_->comm.serial.dataBits = (CommunicationConfiguration::DataBits)ui.lineEdit_serial_dataBits->getCurrentUnitIndex();
  config_->comm.serial.parity = (CommunicationConfiguration::Parity)ui.lineEdit_serial_parity->getCurrentUnitIndex();
  config_->comm.serial.flowControl = (CommunicationConfiguration::FlowControl)ui.lineEdit_serial_flowControl->getCurrentUnitIndex();

  config_->comm.commType = (CommunicationConfiguration::CommType)ui.lineEdit_CommType->getCurrentUnitIndex();
}

void CommSelector::setConfiguration(std::shared_ptr<Configuration> config){
  config_ = config;
}

void CommSelector::init(){
  setupWidgetsControls();
  setupSignalConnection();
}

void CommSelector::flushConfiguration(){
  pushParameters();

}
