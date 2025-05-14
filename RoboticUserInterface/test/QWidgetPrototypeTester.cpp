#include "QWidgetPrototypeTester.h"

#include <QGridLayout>

QWidgetPrototypeTester::QWidgetPrototypeTester(QWidget * parent)
	: QWidget(parent) {
	combobox = new QtMaterialComboBox(this);
	
	QGridLayout* grid = new QGridLayout(this);
	this->setLayout(grid);

	grid->addWidget(combobox);
}
