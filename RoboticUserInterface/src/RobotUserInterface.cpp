#include "robotic_user_interface//RobotUserInterface.h"
#include <functional>
#include <iostream>
#include <QDateTime>

RobotUserInterface::RobotUserInterface(int argc_, char **argv_, QWidget *parent )
  :FluWindowKitWidget(parent),argc(argc_),argv(argv_)
{
  qRegisterMetaType<GCW::NotifyType>("GCW::NotifyType");

  desktopWidget = new QWidget();
  ui.setupUi(desktopWidget);
  m_contentLayout->addWidget(desktopWidget);

  m_titleLabel->setText(tr("Robot User Interface"));
  m_iconButton->setIcon(QIcon(":/log/logo/main.svg"));

  this->setWindowTitle(tr("Robot User Interface"));
  this->setWindowIcon(QIcon(":/log/logo/main.svg"));
  this->setFocusPolicy(Qt::StrongFocus);
}

RobotUserInterface::~RobotUserInterface()
{
  delete topStatus_;
  delete commSelector_;
  delete settingsDisplay_;

  delete robotBase_;
}

void RobotUserInterface::publishNotify(GCW::NotifyType type,const QString &title, const QString& text){
  // ui.textBrowser->append(str);
  QDateTime currentDateTime = QDateTime::currentDateTime();
  QString out = currentDateTime.toString("[yyyy-MM-dd HH:mm:ss] [%1] %2\n").arg(title,text);
  std::cout << out.toLocal8Bit().data();
  snackbar_->addNotify(title,text,type);
}

void RobotUserInterface::resizeEvent(QResizeEvent* event){
  auto &card = config_->card;
  QSize windowSize = event->size();
  card.windowsWidth = windowSize.width();
  card.windowsHeight = windowSize.height();

  FluWindowKitWidget::resizeEvent(event);
}

void RobotUserInterface::closeEvent(QCloseEvent* event)
{
  this->shutdown();
}

void RobotUserInterface::keyPressEvent(QKeyEvent *event) {
  dashboard_base_->keyPressEventGlobal(event);
  FluWindowKitWidget::keyPressEvent(event);
}

void RobotUserInterface::keyReleaseEvent(QKeyEvent *event) {
  dashboard_base_->keyReleaseEventGlobal(event);
  FluWindowKitWidget::keyReleaseEvent(event);
}
void RobotUserInterface::setupSignalConnection() {

  // 消息传递
  QObject::connect(commSelector_, &CommSelector::publishNotify, this, &RobotUserInterface::publishNotify);
  QObject::connect(communicator_, &Communicator::publishNotify, this, &RobotUserInterface::publishNotify);
  QObject::connect(topStatus_,    &FocusStatus::publishNotify, this,    &RobotUserInterface::publishNotify);
  QObject::connect(robotBase_,    &RobotBase::publishNotify, this,    &RobotUserInterface::publishNotify);
  QObject::connect(curveDisplay_, &CurveDisplay::publishNotify, this, &RobotUserInterface::publishNotify);
  QObject::connect(toolsBox_,     &ToolsBox::publishNotify, this, &RobotUserInterface::publishNotify);

  // 设置
  QObject::connect(ui.widget_settings_apply, &QPushButton::clicked, [this](){
    publishNotify(GCW::Info, tr("Settings"), tr("configuration has been updated, Some settings require a reboot to take effect."));
    
    settingsDisplay_->pullParameters();
    flushConfiguration();
  });
  QObject::connect(ui.widget_settings_reset, &QPushButton::clicked, [this](){
    settingsDisplay_->pushParameters();
  });

  QObject::connect(commSelector_, &CommSelector::ok, this, [this]() {
    commSelectorDialog_->hideDialog();
    flushConfiguration();
    });
  QObject::connect(commSelector_, &CommSelector::cancel, commSelectorDialog_, &QtMaterialDialog::hideDialog);

  QObject::connect(communicator_, &Communicator::publishFocusStatus, topStatus_, &FocusStatus::setOtherStatus);
}

void RobotUserInterface::setupWidgetsControls() {
  // message
  snackbar_ = new GCW::QSnackbarManager(this);
  
  // page
  ui.page_curve->setLayout(ui.layout_curve);
  ui.page_info->setLayout(ui.layout_info);
  ui.page_settings->setLayout(ui.layout_settings);
  ui.page_operation->setLayout(ui.layout_operation);
  ui.stackedWidget->setCurrentWidget(ui.page_info);

  makeNav();

  QColor onColor = QColor(83, 109, 145);
  ui.widget_settings_apply->setText("Apply");
  ui.widget_settings_reset->setText("Reset");
  ui.widget_settings_apply->setBackgroundColor(onColor);
  ui.widget_settings_reset->setBackgroundColor(onColor);

  auto &pageName = config_->app.pageName;
  if(pageName == "Operation"){
    onPage_Operation();
    navView_->setClicked(pageItem[PageName::Operation]);
  } else if(pageName == "Info"){
    onPage_Info();
    navView_->setClicked(pageItem[PageName::Info]);
  } else if(pageName == "Curve"){
    onPage_Curve();
    navView_->setClicked(pageItem[PageName::Curve]);
  } else if(pageName == "Terminal"){
    onPage_Terminal();
    navView_->setClicked(pageItem[PageName::Terminal]);
  } else if(pageName == "Tools"){
    onPage_Tools();
    navView_->setClicked(pageItem[PageName::Tools]);
  } else if(pageName == "Settings"){
    onPage_Settings();
    navView_->setClicked(pageItem[PageName::Settings]);
  } 
}

void RobotUserInterface::makeNav(){
  
  NavigationItem* item = nullptr;
  auto& container = ui.stackedWidget;

  item = new NavigationItem();
  item->setText(tr("Operation"));
  item->setIcon(QIcon(":/svg/svg/operation.svg"));
  QObject::connect(item, &NavigationItem::clicked, this, &RobotUserInterface::onPage_Operation);
  navView_->addItemToCenter(item);
  pageItem[PageName::Operation] = item;

  item = new NavigationItem();
  item->setText(tr("Info"));
  item->setIcon(QIcon(":/svg/svg/info.svg"));
  QObject::connect(item, &NavigationItem::clicked, this, &RobotUserInterface::onPage_Info);
  navView_->addItemToCenter(item);
  navView_->setClicked(item);
  pageItem[PageName::Info] = item;

  item = new NavigationItem();
  item->setText(tr("Curve"));
  item->setIcon(QIcon(":/svg/svg/line.svg"));
  QObject::connect(item, &NavigationItem::clicked, this, &RobotUserInterface::onPage_Curve);
  navView_->addItemToCenter(item);
  pageItem[PageName::Curve] = item;

  item = new NavigationItem();
  item->setText(tr("Terminal"));
  item->setIcon(QIcon(":/svg/svg/DataStudio.svg"));
  QObject::connect(item, &NavigationItem::clicked, this, &RobotUserInterface::onPage_Terminal);
  navView_->addItemToCenter(item);
  pageItem[PageName::Terminal] = item;

  item = new NavigationItem();
  item->setText(tr("Tools"));
  item->setIcon(QIcon(":/svg/svg/tools.svg"));
  QObject::connect(item, &NavigationItem::clicked, this, &RobotUserInterface::onPage_Tools);
  navView_->addItemToCenter(item);
  pageItem[PageName::Tools] = item;

  item = new NavigationItem();
  item->setText(tr("Comm"));
  item->setIcon(QIcon(":/svg/svg/network.svg"));
  item->setAsButton(true);
  QObject::connect(item, &NavigationItem::clicked, [=]() { commSelectorDialog_->showDialog(); });
  navView_->addItemToBottom(item);

  item = new NavigationItem();
  item->setText(tr("Settings"));
  item->setIcon(QIcon(":/svg/svg/settings.svg"));
  QObject::connect(item, &NavigationItem::clicked, this, &RobotUserInterface::onPage_Settings);
  navView_->addItemToBottom(item);
  pageItem[PageName::Settings] = item;

  NavigationSwitcher* switcher = nullptr;
  QWSwitcher* toggle = nullptr;

  // 通信
  switcher = new NavigationSwitcher();
  toggle = switcher->getSwitcher();
  toggle->setToggle(false);
  switcher->setText(tr("Comm"));
    QObject::connect(toggle, &QWSwitcher::toggled, [this](bool ok) {
		if (ok) {
			if (!communicator_->isOpen()) {
				communicator_->setup(config_->comm);
				communicator_->open();
			}
		}
		else {
			if (communicator_->isOpen()) {
				communicator_->close();
			}
			topStatus_->setCommStatus(false);
		}
		});
  navView_->addItemToTop(switcher);

  QObject::connect(communicator_, &Communicator::CommStatusChanged, this, [this, toggle](bool ok) {
    toggle->setToggle(ok);
    topStatus_->setCommStatus(ok);
    robotBase_->commStatusChanged(ok);
  }, Qt::ConnectionType::QueuedConnection);

  // recording
  switcher = new NavigationSwitcher();
  toggle = switcher->getSwitcher();
  toggle->setToggle(false);
  switcher->setText(tr("Record"));
  navView_->addItemToTop(switcher);
  QObject::connect(toggle, &QWSwitcher::toggled, [this](bool ok) {
    robotBase_->setEnabledRecord(ok);
  });
}

void RobotUserInterface::shutdown()
{
  robotBase_->saveConfiguration();
  communicator_->shutdown();

  QCoreApplication::exit(0);
}

void RobotUserInterface::setRobotBase(RobotBase *robotBase){
  robotBase_ = robotBase;
}

void RobotUserInterface::init(){

  config_ = robotBase_->configuration();
  config_->runtime.worker = std::make_shared<Runtime::Worker>(config_->app.maxWorkerThread);

  // communicator
  communicator_ = new Communicator(this);
  communicator_->init();

  // dataAllocator
  dataAllocator_ = new DataAllocator();
  dataAllocator_->setCommPtr(communicator_);
  dataAllocator_->setConfiguration(config_);
  dataAllocator_->init();

  // dataStreamSolver
  dataStreamSolver_ = new DataStreamSolver();
  dataStreamSolver_->setConfiguration(config_);
  dataStreamSolver_->setDataAllocator(dataAllocator_);
  dataStreamSolver_->setDataSource(robotBase_->dataSource());
  dataStreamSolver_->init();

  // robotBase
  robotBase_->setDataAllocator(dataAllocator_);
  robotBase_->setTopWidget(this);

  // dashboard
  dashboard_base_ = new Dashboard();
  dashboard_base_->setConfiguration(config_);
  dashboard_base_->setObservations(robotBase_->observations());
  dashboard_base_->setCommand(robotBase_->command());
  dashboard_base_->init();
  dashboard_base_->addChilds(robotBase_->createCustomInfoWidgets());
  dashboard_base_->setActuators(robotBase_->getActuators());
  QSpacerItem* bottomSpacer = new QSpacerItem(100, 35, QSizePolicy::Minimum, QSizePolicy::Fixed);
  dashboard_base_->addChild(bottomSpacer);   // 底部添加空白
  dashboard_base_->flushConfiguration();
  ui.layout_info->addWidget(dashboard_base_);

  // top
  topStatus_ = new FocusStatus();
  topStatus_->setConfiguration(config_);
  topStatus_->setObservations(robotBase_->observations());
  topStatus_->setCommunicator(communicator_);
  m_titleBar->setWidgetAt(FluWindowKitTitleBar::StatusItems,topStatus_->getStatusItemsWidget());
  topStatus_->start();

  // comm Selector && dialog box
  commSelector_ = new CommSelector(desktopWidget);
  commSelector_->setConfiguration(config_);
  commSelector_->init();
  commSelector_->flushConfiguration();
  QVBoxLayout *commSelectorLayout = new QVBoxLayout;
  commSelectorLayout->setContentsMargins(0, 0, 0, 0);
  commSelectorLayout->setSpacing(0);
  commSelectorDialog_ = new QtMaterialDialog(desktopWidget);
  commSelectorDialog_->setWindowLayout(commSelectorLayout);
  commSelectorLayout->addWidget(commSelector_);
  commSelectorDialog_->hideDialog();

  // settings Display_
  settingsDisplay_ = new SettingsDisplay(desktopWidget);
  settingsDisplay_->setConfiguration(config_);
  settingsDisplay_->init();
  settingsDisplay_->pushParameters();
  ui.layout_obj_settings->addWidget(settingsDisplay_);

  // nav
  navView_ = new NavigationView();
  ui.LayoutMain->addWidget(navView_, 0, 1);
  ui.LayoutMain->addWidget(ui.stackedWidget, 0, 2);

  // curve
  curveDisplay_ = new CurveDisplay(desktopWidget);
  curveDisplay_->setDataSource(robotBase_->dataSource());
  curveDisplay_->setConfiguration(config_);
  curveDisplay_->setObservations(robotBase_->observations());
  curveDisplay_->setSteamSolver(dataStreamSolver_);
  curveDisplay_->init();
  ui.layout_curve->addWidget(curveDisplay_);

  // tools
  toolsBox_ = new ToolsBox();
  toolsBox_->setConfiguration(config_);
  toolsBox_->setObservations(robotBase_->observations());
  toolsBox_->init();
  ui.layout_toolsbox->addWidget(toolsBox_);

  // commTerminal
  commTerminal_ = new CommTerminal();
  commTerminal_->setConfiguration(config_);
  commTerminal_->setDataAllocator(dataAllocator_);
  commTerminal_->init();
  ui.layout_terminal->addWidget(commTerminal_);

  // init
  setupWidgetsControls();
  setupSignalConnection();

  // ui.stackedWidget->setCurrentWidget(ui.page_info);

  // top window
  QSize windowSize;
  windowSize.setWidth(config_->card.windowsWidth);
  windowSize.setHeight(config_->card.windowsHeight);
  resize(windowSize);

  // 设置主题
  FluThemeUtils::getUtils()->setTheme(FluTheme::Dark);

  navView_-> toggleExpandRetract();

  if (!config_->app.appName.isEmpty()) {
    m_titleLabel->setText(config_->app.appName);
  }
  m_titleLabel->setStyleSheet(
    R"(FluLabel{
      font-size: 16pt;
      color: #ffffff;
    })"
  );

  flushConfiguration();

}

void RobotUserInterface::onPage_Operation() {
  auto &container = ui.stackedWidget;

  container->setCurrentWidget(ui.page_operation);
  curveDisplay_->setActivate(false);
  commTerminal_->setActivate(false);

  auto &pageName = config_->app.pageName;
  pageName = "Operation";
}

void RobotUserInterface::onPage_Info() {
  auto &container = ui.stackedWidget;
  container->setCurrentWidget(ui.page_info);
  curveDisplay_->setActivate(false);
  commTerminal_->setActivate(false);

  auto &pageName = config_->app.pageName;
  pageName = "Info";
}

void RobotUserInterface::onPage_Curve() {
  auto &container = ui.stackedWidget;
  container->setCurrentWidget(ui.page_curve);
  curveDisplay_->setActivate(true);
  commTerminal_->setActivate(false);

  auto &pageName = config_->app.pageName;
  pageName = "Curve";
}

void RobotUserInterface::onPage_Terminal() {
  auto &container = ui.stackedWidget;
  if (config_->comm.commProtocol !=
      CommunicationConfiguration::CommProtocol::Raw) {
    publishNotify(GCW::Info, tr("Attention"),
                  tr("The communication terminal can only send and receive "
                     "data when the communication protocol is [Raw]."));
  }
  container->setCurrentWidget(ui.page_terminal);
  curveDisplay_->setActivate(false);
  commTerminal_->setActivate(true);

  auto &pageName = config_->app.pageName;
  pageName = "Terminal";
}

void RobotUserInterface::onPage_Tools() {
  auto &container = ui.stackedWidget;
  container->setCurrentWidget(ui.page_tools);
  curveDisplay_->setActivate(false);
  commTerminal_->setActivate(false);

  auto &pageName = config_->app.pageName;
  pageName = "Tools";
}

void RobotUserInterface::onPage_Settings() {
  auto &container = ui.stackedWidget;
  container->setCurrentWidget(ui.page_settings);
  curveDisplay_->setActivate(false);
  commTerminal_->setActivate(false);

  auto &pageName = config_->app.pageName;
  pageName = "Settings";
}


void RobotUserInterface::flushConfiguration() {
  using CCC = CommunicationConfiguration::CommProtocol;

  topStatus_->flushConfiguration();
  dashboard_base_->flushConfiguration();
  commSelector_->flushConfiguration();
  robotBase_->flushConfiguration();

  if (config_->comm.commProtocol == CCC::Float ||
    config_->comm.commProtocol == CCC::JSON) {
    dataStreamSolver_->setActivate(true);
  }
  else {
    dataStreamSolver_->setActivate(false);
  }
}