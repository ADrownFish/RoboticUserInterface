#include "robotic_user_interface/form/CsvLoadDialog.h"

#include <QFileDialog>
#include <QFileInfo>

CsvLoadDialog::CsvLoadDialog(QWidget* parent)
: QWWindowWidget(parent){
	ui.setupUi(this);

	setBorderRadius(0);
	setBackgroundColor(QColor(45, 50, 58));
}

CsvLoadDialog::~CsvLoadDialog() {


}

void CsvLoadDialog::init() {

	setupWidgetsControls();
	setupSignalConnection();
}

void CsvLoadDialog::setConfiguration(std::shared_ptr<Configuration> config) {


}

void CsvLoadDialog::loadFile(const QString& path)
{
	ui.title->setText(tr("How do you want to load the file?"));
	ui.stackedWidget_load_operation->setCurrentWidget(ui.page_operation);
	ui.stackedWidget_load_item->setCurrentWidget(ui.page_type);

	ui.lineEdit_file->setText(path);
	ui.widget_loading->hide();

	filePath = path;

	//提取 头
	auto headers = dataStreamSolver_->extractCSVHeaders(path, true);

	// 准备加载
	if (headers.isEmpty()) {
		dataStreamSolver_->loadCSV(path, "", false);
	}
	else {
		// 询问用户时间列是哪个，显示数据源
		auto radioBox = ui.radioBox;
		for (auto p : radioButtons_) {
			p->deleteLater();
		}
		radioButtons_.clear();
		for (auto header : headers) {
			auto btn = new FluRadioButton(header, radioBox);
			radioBox->addRadioButton(btn);
			radioButtons_.append(btn);
		}

		if (!radioButtons_.isEmpty()) {
			radioButtons_.first()->setChecked(true);
		}
	}
}

void CsvLoadDialog::setSteamSolver(DataStreamSolver* ss)
{
	dataStreamSolver_ = ss;
}

void CsvLoadDialog::setupSignalConnection() {

	QObject::connect(ui.button_cancel, &QPushButton::clicked, this, &CsvLoadDialog::cancel);
	QObject::connect(ui.button_loadit, &QPushButton::clicked, this, &CsvLoadDialog::loadit);

	QObject::connect(ui.widget_LoadingType, &QWWindowButton::selectUnitIndexChanged, this, &CsvLoadDialog::switchType);

}

void CsvLoadDialog::setupWidgetsControls() {

	QColor onColor = QColor(83, 109, 145);
	QColor offColor = QColor(63, 63, 70, 50);

	ui.widget_loading->hide();

	ui.widget_LoadingType->addUnit(tr("By Quantity"));
	ui.widget_LoadingType->addUnit(tr("Select One"));
  ui.widget_LoadingType->setSelectUnitIndex(1);
  ui.widget_LoadingType->setBackgroundColor(QColor(100, 110, 110, 50));

	ui.widget_DelimiterType->addItem(",");
	ui.widget_DelimiterType->addItem("Space");

	ui.button_cancel->setText(tr("Cancel"));
	ui.button_cancel->setIcon(QIcon(":/svg/svg/back.svg"));
	ui.button_cancel->setBackgroundColor(onColor);

	ui.button_loadit->setText(tr("Load It"));
	ui.button_loadit->setIcon(QIcon(":/svg/svg/load.svg"));
	ui.button_loadit->setBackgroundColor(onColor);

	switchType(1);
}

void CsvLoadDialog::loadit() {

	if(!QFileInfo(filePath).exists()){
		publishNotify(GCW::NotifyType::Info, tr("Acttention"), tr("You need to select a file"));
		return;
	}

	ui.title->setText(tr("Loading file, please wait"));
	ui.stackedWidget_load_operation->setCurrentWidget(ui.page_loading);
	ui.stackedWidget_load_item->setCurrentWidget(ui.page_progress);

	ui.widget_loading->show();

	bool isNullHeader = ui.widget_LoadingType->getCurrentUnitIndex() == 0;
	if (isNullHeader)	{
		dataStreamSolver_->loadCSV(filePath, "", false, "/" , [this](int &progress){
			ui.label_progress->setText(QString("%1 %").arg(progress));
		});
	} else {
		QString selectedHeader;
		for (auto it : radioButtons_) {
			if (it->isChecked()) {
				selectedHeader = it->text();
				break;
			}
		}
		dataStreamSolver_->loadCSV(filePath, selectedHeader, true, "/" , [this](int &progress){
			ui.label_progress->setText(QString("%1 %").arg(progress));
		});
	}

	ui.widget_loading->hide();
	emit ok();
}

void CsvLoadDialog::switchType(int index)
{
	if (index) {
		ui.stackedWidget_load_item->setCurrentIndex(0);
	}
	else {
		ui.stackedWidget_load_item->setCurrentIndex(1);
	}
}
