#include "robotic_user_interface/form/ToolsBox.h"

#include "robotic_user_interface/form/FunctionCard.h"

ToolsBox::ToolsBox(QWidget *parent)
: QWidget(parent){
 
  layout = new QFlowLayout(this);
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


void ToolsBox::ToolsBox::setupSignalConnection(){

}


void ToolsBox::setupWidgetsControls(){

}

void ToolsBox::makeCard()
{
  FunctionCard* card = nullptr;

  card = new FunctionCard(QIcon(":/svg/svg/fit.svg"), tr("Ellipsoid Fitting"), tr("Fit the center point of the ellipsoid through the data set"));
  layout->addWidget(card);

  for (size_t i = 0; i < 5; i++)
  {
    card = new FunctionCard(QIcon(":/svg/svg/dot.svg"), tr("any more ?"), tr("wait .. "));
    layout->addWidget(card);
  }


}

