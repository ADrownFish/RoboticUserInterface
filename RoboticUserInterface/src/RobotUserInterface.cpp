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

  this->setFocusPolicy(Qt::StrongFocus);

  m_titleLabel->setText("Robot User Interface");
  //BlurMode blurMode;
  //blurMode.HandleMethodCall(this, 4, kInitialize, 0);
  //blurMode.setColorBackground(QColor(255, 73, 242, 64));
  //blurMode.HandleMethodCall(this, 4, kSetEffect, 0);
}

RobotUserInterface::~RobotUserInterface()
{
  robotBase_->saveConfiguration();
  communicator_->close();

  delete topStatus_;
  delete commSelector_;
  delete settingsDisplay_;
  // delete fileCatcher_;

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
  auto &card = robotBase_->configuration()->card;
  QSize windowSize = event->size();
  card.windowsWidth = windowSize.width();
  card.windowsHeight = windowSize.height();

  //buttom_button_widget->move(10,desktopWidget->  height() - buttom_button_widget->height() - 10);
  //buttom_button_widget_bg->move(10, desktopWidget->height() - buttom_button_widget->height() - 10);
  
  FluWindowKitWidget::resizeEvent(event);
}
// void RobotUserInterface::loadConfig() {

// }

// void RobotUserInterface::saveConfig() {

// }

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
  // QObject::connect(fileCatcher_,  &FileCatcher::publishNotify, this, &RobotUserInterface::publishNotify);

  //auto& topStatusUi = topStatus_->form();
  // 导航
  //QObject::connect(button_nav_, &QPushButton::clicked, [this](){
    //drawer_->openDrawer();
    //button_nav_->hide();
  //});

  // 设置
  QObject::connect(ui.widget_settings_apply, &QPushButton::clicked, [this](){
    publishNotify(GCW::Info, QString("Settings"), QString("configuration has been updated"));
    
    settingsDisplay_->pullParameters();
    topStatus_->flushConfiguration();
    dashboard_base_->flushConfiguration();
    commSelector_->flushConfiguration();
    robotBase_->flushConfiguration();
  });
  QObject::connect(ui.widget_settings_reset, &QPushButton::clicked, [this](){
    settingsDisplay_->pushParameters();
  });



  QObject::connect(commSelector_, &CommSelector::ok, commSelectorDialog_, &QtMaterialDialog::hideDialog);
  QObject::connect(commSelector_, &CommSelector::cancel, commSelectorDialog_, &QtMaterialDialog::hideDialog);

  // 数据传递
  // QObject::connect(communicator_, &Communicator::readyRead, robotBase_,    &RobotBase::readyRead);
  // QObject::connect(robotBase_,    &RobotBase::readyWrite,   communicator_, &Communicator::readyWrite);
  QObject::connect(communicator_, &Communicator::readyRead, [this](){
    
    QByteArray buffer;
    communicator_->read(buffer);
    topStatus_->appendDownloadByte(buffer.size());
    robotBase_->readyRead(buffer);
  });
  QObject::connect(robotBase_,    &RobotBase::readyWrite,   [this](const QByteArray &buffer){
    if(communicator_->isOpen()) topStatus_->appedUploadBytes(buffer.size());
    communicator_->write(buffer);
  });

}

void RobotUserInterface::setupWidgetsControls() {
  //this->setBorderRadius(0);
  //this->setBackgroundColor(QColor(40,49,73));

  // message
  snackbar_ = new GCW::QSnackbarManager(this);
  
  // page
  ui.page_curve->setLayout(ui.layout_curve);
  ui.page_info->setLayout(ui.layout_info);
  ui.page_settings->setLayout(ui.layout_settings);
  ui.page_operation->setLayout(ui.layout_operation);
  ui.stackedWidget->setCurrentWidget(ui.page_info);

  //// button
  //button_operation_ = new QtMaterialRaisedButton(this);
  //button_operation_->setText("Operation");
  //button_info_      = new QtMaterialRaisedButton(this);
  //button_info_->setText("Info");
  //button_curve_     = new QtMaterialRaisedButton(this);
  //button_curve_->setText("Curve");
  //button_settings_  = new QtMaterialRaisedButton(this);
  //button_settings_->setText("Settings");
  //button_comm_  = new QtMaterialRaisedButton(this);
  //button_comm_->setText("Communition");

  //button_nav_ = new QtMaterialRaisedButton;
  //button_nav_->setText("NAV");

  makeNav();

  // button_fileCatch_  = new QtMaterialRaisedButton(this);
  // button_fileCatch_->setText("File Catch");

  ui.widget_settings_apply->setText("Apply");
  ui.widget_settings_reset->setText("Reset");

  //QVBoxLayout *drawerLayout = new QVBoxLayout;
  //drawerLayout->addWidget(button_operation_);
  //drawerLayout->addWidget(button_info_);
  //drawerLayout->addWidget(button_curve_);
  //drawerLayout->addWidget(button_comm_);
  //// drawerLayout->addWidget(button_fileCatch_);
  //drawerLayout->addStretch(3);
  //drawerLayout->addWidget(button_settings_);
  //drawer_->setDrawerLayout(drawerLayout);

}

void RobotUserInterface::makeNav(){
  
  NavigationItem* item = nullptr;
  auto& container = ui.stackedWidget;

  item = new NavigationItem();
  item->setText(tr("Operation"));
  item->setIcon(QIcon(":/svg/svg/operation.svg"));
  QObject::connect(item, &NavigationItem::clicked, [=]() { container->setCurrentWidget(ui.page_operation); });
  navView_->addItemToCenter(item);

  item = new NavigationItem();
  item->setText(tr("Info"));
  item->setIcon(QIcon(":/svg/svg/info.svg"));
  QObject::connect(item, &NavigationItem::clicked, [=]() { container->setCurrentWidget(ui.page_info); });
  navView_->addItemToCenter(item);
  navView_->setClicked(item);

  item = new NavigationItem();
  item->setText(tr("Curve"));
  item->setIcon(QIcon(":/svg/svg/line.svg"));
  QObject::connect(item, &NavigationItem::clicked, [=]() { container->setCurrentWidget(ui.page_curve); });
  navView_->addItemToCenter(item);

  item = new NavigationItem();
  item->setText(tr("Tools"));
  item->setIcon(QIcon(":/svg/svg/tools.svg"));
  QObject::connect(item, &NavigationItem::clicked, [=]() { container->setCurrentWidget(ui.page_tools); });
  navView_->addItemToCenter(item);

  item = new NavigationItem();
  item->setText(tr("Comm"));
  item->setIcon(QIcon(":/svg/svg/network.svg"));
  item->setAsButton(true);
  QObject::connect(item, &NavigationItem::clicked, [=]() { commSelectorDialog_->showDialog(); });
  navView_->addItemToBottom(item);

  item = new NavigationItem();
  item->setText(tr("Settings"));
  item->setIcon(QIcon(":/svg/svg/settings.svg"));
  QObject::connect(item, &NavigationItem::clicked, [=]() { container->setCurrentWidget(ui.page_settings); });
  navView_->addItemToBottom(item);

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
				communicator_->setup(robotBase_->configuration()->comm);
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

	QObject::connect(communicator_, &Communicator::openResult, [this, toggle](bool ok) {
    toggle->setToggle(ok);
		topStatus_->setCommStatus(ok);
		robotBase_->networkStatusChanged(ok);
		});

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

void RobotUserInterface::setRobotBase(RobotBase *robotBase){
  robotBase_ = robotBase;
}

void RobotUserInterface::init(){

  // dashboard
  dashboard_base_ = new Dashboard();
  dashboard_base_->setConfiguration(robotBase_->configuration());
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
  topStatus_->setConfiguration(robotBase_->configuration());
  topStatus_->setObservations(robotBase_->observations());
  //m_titleBar->insertDefaultSpace(4);
  //ui.layout_topStatus->addWidget( topStatus_);

  //m_titleBar->takeWidgetAt(FluWindowKitTitleBar::MenuWidget);
  //m_titleBar->setWidgetAt(FluWindowKitTitleBar::MenuWidget, topStatus_->getNavWidget());
  //m_titleBar->setWidgetAt(FluWindowKitTitleBar::StatusItems, new QPushButton("new button"));
  m_titleBar->setWidgetAt(FluWindowKitTitleBar::StatusItems,topStatus_->getStatusItemsWidget());

  // comm Selector && dialog box
  commSelector_ = new CommSelector(desktopWidget);
  commSelector_->setConfiguration(robotBase_->configuration());
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
  settingsDisplay_->setConfiguration(robotBase_->configuration());
  settingsDisplay_->init();
  settingsDisplay_->pushParameters();
  ui.layout_obj_settings->addWidget(settingsDisplay_);
  
  // communicator
  communicator_ = new Communicator(this);
  communicator_->init();

  navView_ = new NavigationView(this);
  ui.LayoutMain->addWidget(navView_, 0, 1);
  ui.LayoutMain->addWidget(ui.stackedWidget, 0, 2);

  // curve
  curveDisplay_ = new CurveDisplay(desktopWidget);
  curveDisplay_->setDataSource(robotBase_->dataSource());
  curveDisplay_->setConfiguration(robotBase_->configuration());
  curveDisplay_->setObservations(robotBase_->observations());
  curveDisplay_->init();
  ui.layout_curve->addWidget(curveDisplay_);

  // tools
  toolsBox_ = new ToolsBox(this);
  toolsBox_->init();
  ui.layout_toolsbox->addWidget(toolsBox_);

  // file Catcher
  // fileCatcher_ = new FileCatcher(ui.page_fileCatch);
  // fileCatcher_->setConfiguration(robotBase_->configuration());
  // fileCatcher_->init();
  // ui.layout_fileCatch->addWidget(fileCatcher_);
  
  // init
  setupWidgetsControls();
  setupSignalConnection();

  ui.stackedWidget->setCurrentWidget(ui.page_info);

  // top window
  QSize windowSize;
  windowSize.setWidth(robotBase_->configuration()->card.windowsWidth);
  windowSize.setHeight(robotBase_->configuration()->card.windowsHeight);
  resize(windowSize);

  // 设置主题
  FluThemeUtils::getUtils()->setTheme(FluTheme::Dark);

  // 先不用
  //button_operation_->deleteLater();


  navView_-> toggleExpandRetract();
}
