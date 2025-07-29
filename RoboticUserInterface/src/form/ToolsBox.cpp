#include "robotic_user_interface/form/ToolsBox.h"
#include "robotic_user_interface/form/FunctionCard.h"
#include "robotic_user_interface/toolbox/IMUEllipsoidFit.h"

ToolsBox::ToolsBox(QWidget *parent)
: QStackedWidget(parent){

  mainPage = new QWidget();
  this->addWidget(mainPage);

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

}


void ToolsBox::setupWidgetsControls(){

}

void ToolsBox::makeCard()
{
  FunctionCard* card = nullptr;

  // Ellipsoid Fitting
  card = new FunctionCard(QIcon(":/svg/svg/fit.svg"), tr("IMU Ellipsoid Fitting"), tr("Fit the center point of the ellipsoid through the data set"));
  layout->addWidget(card);
  IMUEllipsoidFit *ief = new IMUEllipsoidFit(this);
  ief->setConfiguration(config_);
  ief->setObservations(observations_);
  ief->init();
  this->addWidget(ief);
  QObject::connect(card, &FunctionCard::clicked, [this,ief](){
    this->setCurrentWidget(ief);
    ief->setActivate(true);
  });
  QObject::connect(ief, &IMUEllipsoidFit::back, [this,ief](){
    this->setCurrentWidget(mainPage);
    ief->setActivate(false);
  });
  QObject::connect(ief,     &IMUEllipsoidFit::publishNotify, this, &ToolsBox::publishNotify);

  card = new FunctionCard(QIcon(":/svg/svg/operation.svg"), tr(""), tr("wait .. "));
  layout->addWidget(card);
  QObject::connect(card, &FunctionCard::clicked, [this](){
    publishNotify(GCW::NotifyType::Info,tr("Wait a minute"),tr("wait wait wait . . ."));
  });

  // Any more
  card = new FunctionCard(QIcon(":/svg/svg/dot.svg"), tr("any more ?"), tr("wait .. "));
  layout->addWidget(card);
  QObject::connect(card, &FunctionCard::clicked, [this](){
    publishNotify(GCW::NotifyType::Info,tr("Wait a minute"),tr("wait wait wait . . ."));
  });



}

