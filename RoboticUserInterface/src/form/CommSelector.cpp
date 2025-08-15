#include "robotic_user_interface/form/CommSelector.h"

#include "FluControls/FluConfirmFlyout.h"
#include "qwool/qwdropwidget.h"

#include <QSerialPortInfo>
#include <QObject>


CommSelector::CommSelector(QWidget *parent)
  : QWWindowWidget(parent) {
  ui.setupUi(this);

  // setBorderRadius(0);
}
CommSelector::~CommSelector(){

}

void CommSelector::setupWidgetsControls(){
  setBorderRadius(0);
  setBackgroundColor(QColor(45,50,58));

  QColor onColor = QColor(83, 109, 145);
  
  ui.button_ok->setText("OK");
  ui.button_cancel->setText("Cancel");
  ui.HowToUse->setIcon(QIcon(":/svg/svg/question.svg"));

  ui.button_ok->setBackgroundColor(onColor);
  ui.button_cancel->setBackgroundColor(onColor);
  ui.HowToUse->setBackgroundColor(onColor);

  ui.widget_CommType->addUnit("UDP");
  ui.widget_CommType->addUnit("TCP");
  ui.widget_CommType->addUnit(tr("Bluetooth"));
  ui.widget_CommType->addUnit(tr("Serial"));

  ui.widget_protocol->addUnit(tr("Plugin"));
  ui.widget_protocol->addUnit(tr("JSON"));
  ui.widget_protocol->addUnit(tr("Float"));
  ui.widget_protocol->addUnit(tr("Raw"));

  ui.widget_TCPType->addUnit(tr("Client"));
  ui.widget_TCPType->addUnit(tr("Server"));
  ui.widget_TCPType->setBackgroundColor(QColor(100, 110, 110, 50));

  ui.lineEdit_udp_ip->setLabel("IP");
  ui.lineEdit_udp_port->setLabel(tr("Port"));
  ui.lineEdit_udp_listen->setLabel(tr("Listen"));

  ui.lineEdit_tcp_ip->setLabel("IP");
  ui.lineEdit_tcp_port->setLabel(tr("Port"));
  ui.lineEdit_tcp_listen->setLabel(tr("Listen"));

  ui.lineEdit_serial_baudrate->setLabel(tr("Baud Rate"));
  ui.lineEdit_serial_name->setLabel(tr("SerialPort"));
  ui.lineEdit_serial_stopBits->addUnit("1");
  ui.lineEdit_serial_stopBits->addUnit("2");
  ui.lineEdit_serial_stopBits->addUnit("1.5");
  ui.lineEdit_serial_dataBits->addUnit("5");
  ui.lineEdit_serial_dataBits->addUnit("6");
  ui.lineEdit_serial_dataBits->addUnit("7");
  ui.lineEdit_serial_dataBits->addUnit("8");
  ui.lineEdit_serial_stopBits->setBackgroundColor(QColor(100,110,110,50));
  ui.lineEdit_serial_dataBits->setBackgroundColor(QColor(100,110,110,50));
  
  ui.lineEdit_serial_parity->addUnit(tr("None"));
  ui.lineEdit_serial_parity->addUnit(tr("Odd"));
  ui.lineEdit_serial_parity->addUnit(tr("Even"));
  ui.lineEdit_serial_parity->addUnit(tr("Mark"));
  ui.lineEdit_serial_parity->addUnit(tr("Space"));
  ui.lineEdit_serial_parity->setBackgroundColor(QColor(100,110,110,50));

  ui.lineEdit_serial_flowControl->addUnit(tr("None"));
  ui.lineEdit_serial_flowControl->addUnit(tr("Software"));
  ui.lineEdit_serial_flowControl->addUnit(tr("Hardware"));
  ui.lineEdit_serial_flowControl->setBackgroundColor(QColor(100,110,110,50));

  ui.widget_CommType->setSelectUnitIndex(static_cast<int>(config_->comm.commType));
  ui.widget_CommType->setBackgroundColor(QColor(100,100,100,120));

  ui.widget_protocol->setSelectUnitIndex(static_cast<int>(config_->comm.commProtocol));
  ui.widget_protocol->setBackgroundColor(QColor(100,100,100,120));

  switchPageType(static_cast<int>(config_->comm.commType));
  switchPageProtocol(static_cast<int>(config_->comm.commProtocol));
  ui.lineEdit_json_timestamp->setVisible(config_->stream.timestampEnable_json);
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
  QObject::connect(ui.widget_TCPType, &QWWindowButton::selectUnitIndexChanged, [this](unsigned int index){
    QString key = ui.widget_TCPType->getUnitName(index);
    if (key == tr("Server")) {
      ui.stackedWidget_tcp->setCurrentWidget(ui.page_server);
    }
    else if (key == tr("Client")) {
      ui.stackedWidget_tcp->setCurrentWidget(ui.page_client);
    } 

  });
  QObject::connect(ui.HowToUse, &QPushButton::clicked, [this](bool clicked){
    auto flyout = new FluConfirmFlyout(ui.HowToUse, FluFlyoutPosition::Right);
    flyout->setTitle(tr("Communication Configurator"));
    flyout->setInfo(tr(
    "Protocol: Select the method for data parsing.\n"
    "Type: Select the communication method to use.\n"
    ));
    flyout->show();
  });

  QObject::connect(ui.widget_CommType, &QWWindowButton::selectUnitIndexChanged, this, &CommSelector::switchPageType);

  QObject::connect(ui.widget_protocol, &QWWindowButton::selectUnitIndexChanged, this, &CommSelector::switchPageProtocol);

  QObject::connect(ui.button_json_timestamp, &QWSwitcher::toggled, ui.lineEdit_json_timestamp, &QLineEdit::setVisible);
  
  QList<int> baudrates = {
      4800, 9600, 14400, 19200,
      38400, 56000, 57600, 115200, 128000, 230400, 256000,
      460800, 500000, 576000, 921600, 1000000, 1152000
  };

  menu_baudRate   = new FluMenu();
  menu_serialPort = new FluMenu();
  menu_udp_ip     = new FluMenu();
  menu_udp_listen = new FluMenu();
  menu_udp_port   = new FluMenu();
  menu_tcp_ip     = new FluMenu();
  menu_tcp_listen = new FluMenu();
  menu_tcp_port   = new FluMenu();

  for (int baud : baudrates) {
    FluAction* action = new FluAction(QString::number(baud));
    menu_baudRate->addAction(action);
    connect(action, &QAction::triggered, [=]() {
      ui.lineEdit_serial_baudrate->setText(QString::number(baud));
      });
  }

  QWDropWidget *drop_baudRate   = new QWDropWidget();
  QWDropWidget* drop_serialPort = new QWDropWidget();

  QWDropWidget* drop_udp_ip     = new QWDropWidget();
  QWDropWidget* drop_udp_listen = new QWDropWidget();
  QWDropWidget* drop_udp_port   = new QWDropWidget();

  QWDropWidget* drop_tcp_ip     = new QWDropWidget();
  QWDropWidget* drop_tcp_listen = new QWDropWidget();
  QWDropWidget* drop_tcp_port   = new QWDropWidget();

  drop_baudRate->setDropIcon(QIcon(":/svg/svg/arrow-right2.svg"));
  drop_serialPort->setDropIcon(QIcon(":/svg/svg/arrow-right2.svg"));

  drop_udp_ip->setDropIcon(QIcon(":/svg/svg/arrow-right2.svg"));
  drop_udp_listen->setDropIcon(QIcon(":/svg/svg/arrow-right2.svg"));
  drop_udp_port->setDropIcon(QIcon(":/svg/svg/arrow-right2.svg"));

  drop_tcp_ip->setDropIcon(QIcon(":/svg/svg/arrow-right2.svg"));
  drop_tcp_listen->setDropIcon(QIcon(":/svg/svg/arrow-right2.svg"));
  drop_tcp_port->setDropIcon(QIcon(":/svg/svg/arrow-right2.svg"));

  ui.gridLayout_udp->replaceWidget(ui.lineEdit_udp_ip, drop_udp_ip);
  ui.gridLayout_udp->replaceWidget(ui.lineEdit_udp_port, drop_udp_port);
  ui.gridLayout_udp->replaceWidget(ui.lineEdit_udp_listen, drop_udp_listen);

  ui.gridLayout_tcp_client->replaceWidget(ui.lineEdit_tcp_ip, drop_tcp_ip);
  ui.gridLayout_tcp_client->replaceWidget(ui.lineEdit_tcp_port, drop_tcp_port);
  ui.gridLayout_tcp_server->replaceWidget(ui.lineEdit_tcp_listen, drop_tcp_listen);

  ui.gridLayout_serial->replaceWidget(ui.lineEdit_serial_name, drop_serialPort);
  ui.gridLayout_serial->replaceWidget(ui.lineEdit_serial_baudrate, drop_baudRate);

  drop_tcp_ip->setWidget(ui.lineEdit_tcp_ip);
  drop_tcp_port->setWidget(ui.lineEdit_tcp_port);
  drop_tcp_listen->setWidget(ui.lineEdit_tcp_listen);

  drop_udp_ip->setWidget(ui.lineEdit_udp_ip);
  drop_udp_port->setWidget(ui.lineEdit_udp_port);
  drop_udp_listen->setWidget(ui.lineEdit_udp_listen);

  drop_serialPort->setWidget(ui.lineEdit_serial_name);
  drop_baudRate->setWidget(ui.lineEdit_serial_baudrate);

  // TCP 相关菜单
  drop_tcp_ip->setMenu(menu_tcp_ip, [this] {
    menu_tcp_ip->clear();
    if (config_->comm.tcp.ipHistory.isEmpty()) {
      FluAction* action = new FluAction(tr("No History"));
      menu_tcp_ip->addAction(action);
      return;
    }
    for (const auto& item : config_->comm.tcp.ipHistory) {
      FluAction* action = new FluAction(item);
      menu_tcp_ip->addAction(action);
      connect(action, &QAction::triggered, [=]() {
        ui.lineEdit_tcp_ip->setText(item);
        });
    }
    });
  drop_tcp_listen->setMenu(menu_tcp_listen, [this]() {
    menu_tcp_listen->clear();
    if (config_->comm.tcp.listenHistory.isEmpty()) {
      FluAction* action = new FluAction(tr("No History"));
      menu_tcp_listen->addAction(action);
      return;
    }
    for (const auto& item : config_->comm.tcp.listenHistory) {
      FluAction* action = new FluAction(QString::number(item));
      menu_tcp_listen->addAction(action);
      connect(action, &QAction::triggered, [=]() {
        ui.lineEdit_tcp_listen->setText(QString::number(item));
        });
    }
    });
  drop_tcp_port->setMenu(menu_tcp_port, [this]() {
    menu_tcp_port->clear();
    if (config_->comm.tcp.portHistory.isEmpty()) {
      FluAction* action = new FluAction(tr("No History"));
      menu_tcp_port->addAction(action);
      return;
    }
    for (const auto& item : config_->comm.tcp.portHistory) {
      FluAction* action = new FluAction(QString::number(item));
      menu_tcp_port->addAction(action);
      connect(action, &QAction::triggered, [=]() {
        ui.lineEdit_tcp_port->setText(QString::number(item));
        });
    }
    });

  // UDP 相关菜单
  drop_udp_ip->setMenu(menu_udp_ip, [this]() {
    menu_udp_ip->clear();
    if (config_->comm.udp.ipHistory.isEmpty()) {
      FluAction* action = new FluAction(tr("No History"));
      menu_udp_ip->addAction(action);
      return;
    }
    for (const auto& item : config_->comm.udp.ipHistory) {
      FluAction* action = new FluAction(item);
      menu_udp_ip->addAction(action);
      connect(action, &QAction::triggered, [=]() {
        ui.lineEdit_udp_ip->setText(item);
        });
    }
    });
  drop_udp_listen->setMenu(menu_udp_listen, [this]() {
    menu_udp_listen->clear();
    if (config_->comm.udp.listenHistory.isEmpty()) {
      FluAction* action = new FluAction(tr("No History"));
      menu_udp_listen->addAction(action);
      return;
    }
    for (const auto& item : config_->comm.udp.listenHistory) {
      FluAction* action = new FluAction(QString::number(item));
      menu_udp_listen->addAction(action);
      connect(action, &QAction::triggered, [=]() {
        ui.lineEdit_udp_listen->setText(QString::number(item));
        });
    }
    });
  drop_udp_port->setMenu(menu_udp_port, [this]() {
    menu_udp_port->clear();
    if (config_->comm.udp.portHistory.isEmpty()) {
      FluAction* action = new FluAction(tr("No History"));
      menu_udp_port->addAction(action);
      return;
    }
    for (const auto& item : config_->comm.udp.portHistory) {
      FluAction* action = new FluAction(QString::number(item));
      menu_udp_port->addAction(action);
      connect(action, &QAction::triggered, [=]() {
        ui.lineEdit_udp_port->setText(QString::number(item));
        });
    }
    });

  //qDebug() << " ************************ load *********************";
  //qDebug() << " udp ip" << config_->comm.udp.ipHistory;
  //qDebug() << " udp listen" << config_->comm.udp.listenHistory;
  //qDebug() << " udp port" << config_->comm.udp.portHistory;

  //qDebug() << " tcp ip" << config_->comm.tcp.ipHistory;
  //qDebug() << " tcp listen" << config_->comm.tcp.listenHistory;
  //qDebug() << " tcp port" << config_->comm.tcp.portHistory;
  //qDebug() << " *********************************************";



  drop_serialPort->setMenu(menu_serialPort, [this]() {
    scanSerialPort();
  });
  drop_baudRate->setMenu(menu_baudRate);

}

void CommSelector::pushParameters(){

  ui.lineEdit_udp_ip->setText(config_->comm.udp.ip);
  ui.lineEdit_udp_port->setText(QString::number(config_->comm.udp.port));
  ui.lineEdit_udp_listen->setText(QString::number(config_->comm.udp.listen));

  ui.lineEdit_tcp_ip->setText(config_->comm.tcp.ip);
  ui.lineEdit_tcp_port->setText(QString::number(config_->comm.tcp.port));
  ui.lineEdit_tcp_listen->setText(QString::number(config_->comm.tcp.listen));
  ui.widget_TCPType->setSelectUnitIndex(config_->comm.tcp.server);

  ui.lineEdit_serial_baudrate->setText(QString::number(config_->comm.serial.baudRate));
  ui.lineEdit_serial_name->setText(config_->comm.serial.serialName);
  ui.lineEdit_serial_stopBits->setSelectUnitIndex(static_cast<int>(config_->comm.serial.stopBits));
  ui.lineEdit_serial_dataBits->setSelectUnitIndex(static_cast<int>(config_->comm.serial.dataBits));
  ui.lineEdit_serial_parity->setSelectUnitIndex(static_cast<int>(config_->comm.serial.parity));
  ui.lineEdit_serial_flowControl->setSelectUnitIndex(static_cast<int>(config_->comm.serial.flowControl));

  ui.widget_CommType->setSelectUnitIndex((int)config_->comm.commType);
  ui.widget_protocol->setSelectUnitIndex((int)(config_->comm.commProtocol));
  ui.widget_TCPType->setSelectUnitIndex((int)config_->comm.tcp.server);

  ui.button_float_timestamp->setToggle(config_->stream.timestampEnable_float);
  ui.button_json_timestamp->setToggle(config_->stream.timestampEnable_json);
  ui.lineEdit_json_timestamp->setText(config_->stream.timestampString_json);
}

template<typename T>
void InsertHistory(QList<T> &list, const T& value, int count = 5) {
  list.removeAll(value);
  list.prepend(value);
  while (list.size() > count) {
    list.removeLast();
  }
}

void CommSelector::pullParameters(){
  config_->comm.udp.ip = ui.lineEdit_udp_ip->text();
  config_->comm.udp.port = ui.lineEdit_udp_port->text().toInt();;
  config_->comm.udp.listen = ui.lineEdit_udp_listen->text().toInt();

  config_->comm.tcp.ip = ui.lineEdit_tcp_ip->text();
  config_->comm.tcp.port = ui.lineEdit_tcp_port->text().toInt();;
  config_->comm.tcp.listen = ui.lineEdit_tcp_listen->text().toInt();
  config_->comm.tcp.server = ui.widget_TCPType->getCurrentUnitIndex();

  config_->comm.serial.baudRate = ui.lineEdit_serial_baudrate->text().toInt();
  config_->comm.serial.serialName = ui.lineEdit_serial_name->text();
  config_->comm.serial.stopBits = (CommunicationConfiguration::StopBits)ui.lineEdit_serial_stopBits->getCurrentUnitIndex();
  config_->comm.serial.dataBits = (CommunicationConfiguration::DataBits)ui.lineEdit_serial_dataBits->getCurrentUnitIndex();
  config_->comm.serial.parity = (CommunicationConfiguration::Parity)ui.lineEdit_serial_parity->getCurrentUnitIndex();
  config_->comm.serial.flowControl = (CommunicationConfiguration::FlowControl)ui.lineEdit_serial_flowControl->getCurrentUnitIndex();

  config_->comm.commType = (CommunicationConfiguration::CommType)ui.widget_CommType->getCurrentUnitIndex();
  config_->comm.commProtocol = (CommunicationConfiguration::CommProtocol)ui.widget_protocol->getCurrentUnitIndex();

  constexpr int count = 5;
  InsertHistory(config_->comm.udp.ipHistory,          config_->comm.udp.ip,                  count);
  InsertHistory(config_->comm.udp.portHistory,       config_->comm.udp.port,              count);
  InsertHistory(config_->comm.udp.listenHistory,     config_->comm.udp.listen ,            count);

  InsertHistory(config_->comm.tcp.ipHistory,            config_->comm.tcp.ip,                   count);
  InsertHistory(config_->comm.tcp.portHistory,        config_->comm.tcp.port,                count);
  InsertHistory(config_->comm.tcp.listenHistory,      config_->comm.tcp.listen,               count);

  config_->stream.timestampEnable_float = ui.button_float_timestamp->isToggled();
  config_->stream.timestampEnable_json = ui.button_json_timestamp->isToggled();
  config_->stream.timestampString_json = ui.lineEdit_json_timestamp->text();

  //qDebug() << " *********************************************";
  //qDebug() << " udp ip" << config_->comm.udp.ipHistory;
  //qDebug() << " udp listen" << config_->comm.udp.listenHistory;
  //qDebug() << " udp port" << config_->comm.udp.portHistory;

  //qDebug() << " tcp ip" << config_->comm.tcp.ipHistory;
  //qDebug() << " tcp listen" << config_->comm.tcp.listenHistory;
  //qDebug() << " tcp port" << config_->comm.tcp.portHistory;
  //qDebug() << " *********************************************";

}

void CommSelector::scanSerialPort()
{
  menu_serialPort->clear();
  const auto ports = QSerialPortInfo::availablePorts();

  if (ports.isEmpty()) {
    FluAction* action = new FluAction(tr("Serial port not detected"));
    menu_serialPort->addAction(action);
    action->setEnabled(false);
    return;
  }

  for (const QSerialPortInfo& info : ports) {
    QString portName = info.portName();                          // COM3, ttyUSB0, etc.
    QString description = info.description();                    // 描述
    QString manufacturer = info.manufacturer();
    if (description.isEmpty())
      description = tr("Unknown device");

    // 组合显示内容，例如 "COM3 - USB-SERIAL CH340"
    QString actionText = QString("%1 - %2 - %3").arg(portName, description, manufacturer);
    FluAction* action = new FluAction(actionText);
    menu_serialPort->addAction(action);

    // 使用 lambda 捕捉端口名并绑定点击行为
    connect(action, &QAction::triggered, [=]() {
      ui.lineEdit_serial_name->setText(portName);  // 填入纯粹的串口名
      });
  }
}

void CommSelector::switchPageType(unsigned int index) {

  QString key = ui.widget_CommType->getUnitName(index);
  if (key == "TCP") {
    ui.stackedWidget_type->setCurrentWidget(ui.page_tcp);
  }
  else if (key == "UDP") {
    ui.stackedWidget_type->setCurrentWidget(ui.page_udp);
  }
  else if (key == tr("Bluetooth")) {
    ui.stackedWidget_type->setCurrentWidget(ui.page_bluetooth);
  }
  else if (key == tr("Serial")) {
    ui.stackedWidget_type->setCurrentWidget(ui.page_serial);
  }
}

void CommSelector::switchPageProtocol(unsigned int index) {

  QString key = ui.widget_protocol->getUnitName(index);
  if (key == "Plugin") {
    ui.stackedWidget_protocol->hide();
    ui.stackedWidget_protocol->setCurrentWidget(ui.page_plugin);
  }
  else if (key == "JSON") {
    ui.stackedWidget_protocol->show();
    ui.stackedWidget_protocol->setCurrentWidget(ui.page_Json);
  }
  else if (key == tr("Float")) {
    ui.stackedWidget_protocol->show();
    ui.stackedWidget_protocol->setCurrentWidget(ui.page_Float);
  }
  else if (key == tr("Raw")) {
    ui.stackedWidget_protocol->hide();
    ui.stackedWidget_protocol->setCurrentWidget(ui.page_Raw);
  }
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
