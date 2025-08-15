#include "robotic_user_interface/form/ToolsBox.h"
#include "robotic_user_interface/form/FunctionCard.h"
#include "robotic_user_interface/toolbox/IMUEllipsoidFit.h"

ToolsBox::ToolsBox(QWidget *parent)
: QWidget(parent){

  homeButton = new QtMaterialRaisedButton();
  homeButton->setFixedSize(40,30);
  homeButton->setIcon(QIcon(":/svg/svg/back.svg"));
  QHBoxLayout* buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(homeButton);
  buttonLayout->addStretch();
  
  stackwidget = new QStackedWidget();
  
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(buttonLayout);
  mainLayout->addWidget(stackwidget);

  mainPage = new QWidget();
  stackwidget->addWidget(mainPage);

  layout = new QFlowLayout(mainPage);
  layout->setSpacing(10);
  layout->setContentsMargins(5, 5, 5, 5);
}

ToolsBox::~ToolsBox(){
  

}

void ToolsBox::init(){
  setupWidgetsControls();
  setupSignalConnection();

  makeCard();
}


void ToolsBox::setConfiguration(std::shared_ptr<Configuration> config){
  config_ = config;
}

void ToolsBox::setObservations(std::shared_ptr<ObservationsBase> obs){
  observations_ = obs;
}

void ToolsBox::ToolsBox::setupSignalConnection(){
  QObject::connect(homeButton, &QtMaterialRaisedButton::clicked, [this](){
    stackwidget->setCurrentWidget(mainPage);
  });
}


void ToolsBox::setupWidgetsControls(){

}

void ToolsBox::makeCard()
{
  FunctionCard* card = nullptr;

  auto initWidget = [](PluginBase *plugin,  ToolsBox *this_) -> PluginBase*{
    plugin->setConfiguration(this_->config_);
    plugin->setObservations(this_->observations_);
    plugin->setNotifyCallback([this_](PluginBase::NotifyType type, const QString& title, const QString& text) -> void{
      this_->publishNotify((GCW::NotifyType)type, title, text);
    });
    plugin->initialize();

    this_->stackwidget->addWidget(plugin);

    FunctionCard *card = new FunctionCard(
      QIcon(plugin->pluginIcon()),
      plugin->pluginName(),
      plugin->pluginDescription()
    );
    this_->layout->addWidget(card);    

    QObject::connect(card, &FunctionCard::clicked, [this_,plugin](){
      this_->stackwidget->setCurrentWidget(plugin);
      plugin->setActivate(true);
    });

    return plugin;
  };


  PluginBase *plugin = initWidget(new IMUEllipsoidFit(this), this);
}

