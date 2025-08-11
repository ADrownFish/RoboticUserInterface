#pragma once

#include "ui_CommTerminal.h"

#include "qwool/qwwindowwidget.h"
#include "qt_gcw/QSnackbarManager.h"

#include "robotic_user_interface/core/Types.h"
#include "robotic_user_interface/core/DataAllocator.h"

#include <QTimer>
#include <QMutex>
#include <QRegularExpressionValidator>

class CommTerminal : public QWWindowWidget {

	Q_OBJECT

public:

	CommTerminal(QWidget *parent = nullptr);

	~CommTerminal();

	void init();

	void setConfiguration(std::shared_ptr<Configuration> config);

	void setDataAllocator(const QPointer<DataAllocator>& p);

	void flushConfiguration();

	void setActivate(bool ok);

signals:
	void publishNotify(GCW::NotifyType type, const QString& title, const QString& text);

private:
	void setupSignalConnection();

	void setupWidgetsControls();

	void appendColoredText(int status, const QString& text);

	void processData();

	void sendData();

    void setLineEditMode(QLineEdit *p, EncodingType type);

private:
	Ui::CommTerminal ui;

	std::shared_ptr<Configuration> config_;

	QPointer<DataAllocator> dataAllocator_;
	QValidator* hexValidator;

	QByteArray recviveBuffer_;
	QTimer timer_;
	QMutex readMutex;

};
