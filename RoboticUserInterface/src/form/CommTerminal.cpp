#include "robotic_user_interface/form/CommTerminal.h"

CommTerminal::CommTerminal(QWidget* parent)
	: QWWindowWidget(parent)
{
	ui.setupUi(this);


}

CommTerminal::~CommTerminal()
{

}

void CommTerminal::init()
{
	setupWidgetsControls();
	setupSignalConnection();
}

void CommTerminal::setConfiguration(std::shared_ptr<Configuration> config)
{
	config_ = config;
}

void CommTerminal::setDataAllocator(const QPointer<DataAllocator>& p)
{
  dataAllocator_ = p;

	// 数据传递
	QObject::connect(dataAllocator_, &DataAllocator::readyRead, [this]() {

    // on signal thread
		if (config_->comm.commProtocol == CommunicationConfiguration::CommProtocol::Raw) {
			QByteArray buffer;
			dataAllocator_->read(CommunicationConfiguration::CommProtocol::Raw, buffer);

      readMutex.lock();
      recviveBuffer_.append(buffer);
      readMutex.unlock();
		}
	});
}

void CommTerminal::flushConfiguration()
{
}

void CommTerminal::setActivate(bool ok)
{
	if (ok) {
		timer_.start();
	}
	else {
		timer_.stop();
	}
}

void CommTerminal::setupWidgetsControls()
{
	setBorderRadius(8);
	QColor bgColor(32, 32, 32);
	setBackgroundColor(bgColor);

  QColor onColor = QColor(83, 109, 145);
  QColor offColor = QColor(63, 63, 70, 50);

	ui.widget_output_coding->appendState({ true, ("Abc"), onColor , QString(":/svg/svg/string.svg") });
	ui.widget_output_coding->appendState({ true, ("Hex"), offColor , QString(":/svg/svg/hex.svg") });

	ui.widget_input_coding->appendState({ true, ("Abc"), onColor , QString(":/svg/svg/string.svg") });
	ui.widget_input_coding->appendState({ true, ("Hex"), QColor(80, 85, 90) , QString(":/svg/svg/hex.svg") });

	ui.widget_output_clear->setText(tr("Clear"));
	ui.widget_output_clear->setIcon(QIcon(":/svg/svg/clear.svg"));
  ui.widget_output_clear->setBackgroundColor(onColor);

	ui.widget_input_clear->setText(tr("Clear"));
	ui.widget_input_clear->setIcon(QIcon(":/svg/svg/clear.svg"));
  ui.widget_input_clear->setBackgroundColor(onColor);

	ui.widget_send->setText(tr("Send"));
	ui.widget_send->setIcon(QIcon(":/svg/svg/return.svg"));
  ui.widget_send->setBackgroundColor(onColor);

	timer_.setInterval(50);

  QRegularExpression regex("^(?:[0-9A-Fa-f]{2}\\s?)*$");
  hexValidator = new QRegularExpressionValidator(regex, this);

  ui.widget_input_coding->setSelectState(static_cast<int>(config_->terminal.inputType));
  ui.widget_output_coding->setSelectState(static_cast<int>(config_->terminal.outputType));

  setLineEditMode(ui.widget_input, config_->terminal.inputType);
}

void CommTerminal::setupSignalConnection()
{
  QObject::connect(ui.widget_input_clear, &QPushButton::clicked, [this]() {
		ui.widget_input->clear();
		});
  QObject::connect(ui.widget_output_clear, &QPushButton::clicked, [this]() {
		ui.textBrowser->clear();
		});

  QObject::connect(ui.widget_send, &QPushButton::clicked, this, &CommTerminal::sendData);

  QObject::connect(ui.widget_input, &QLineEdit::returnPressed, this, &CommTerminal::sendData);

  QObject::connect(&timer_, &QTimer::timeout, this, &CommTerminal::processData);

  QObject::connect(ui.widget_output_coding, &QWSwitcherButton::selectIndexChanged, [this](int index) {
        if(index){
            config_->terminal.outputType = EncodingType::Hex;
        } else {
            config_->terminal.outputType = EncodingType::Abc;
        }
        });
  QObject::connect(ui.widget_input_coding, &QWSwitcherButton::selectIndexChanged, [this](int index) {
        if(index){
            config_->terminal.inputType = EncodingType::Hex;
        } else {
            config_->terminal.inputType = EncodingType::Abc;
        }
        setLineEditMode(ui.widget_input, config_->terminal.inputType);
        });
}

void CommTerminal::appendColoredText(int status, const QString& text) {
	QString color;
	switch (status) {
	case 0:
		color = "#00BFFF"; // 天蓝色 (DeepSkyBlue)
		break;
	case 1:
		color = "#00EEAA"; // 绿色
		break;
	default:
		color = "#FFFFFF"; // 默认白色，防止异常状态
		break;
	}

	QString htmlText = QString("<span style=\"color:%1;\">%2</span>").arg(color, text.toHtmlEscaped());
	ui.textBrowser->append(htmlText);
}

void CommTerminal::processData()
{
  QByteArray buffer;

  readMutex.lock();
  if (recviveBuffer_.isEmpty()) {
    readMutex.unlock();
    return;
  }
  buffer = std::move(recviveBuffer_);
  readMutex.unlock();

  if (config_->terminal.outputType == EncodingType::Hex) {
    QString hexStr = buffer.toHex(' ');
    appendColoredText(0, hexStr);
  } else {
    appendColoredText(0, QString::fromLocal8Bit(buffer));
  }
}

void CommTerminal::sendData()
{
	QString str = ui.widget_input->text();
	if (str.isEmpty()) {
		return;
	}
	appendColoredText(1, str);
	ui.widget_input->clear();

  if(config_->terminal.inputType == EncodingType::Hex){
    QStringList hexList = str.split(" ", Qt::SkipEmptyParts);

    // 检查最后一个元素是否为单个字符
    if (!hexList.isEmpty() && hexList.last().size() == 1) {
      QString last = hexList.takeLast(); // 取出末尾的一个字符
      hexList.append("0" + last);        // 补零后重新加入
    }

    QByteArray byteArray;
    for (const QString &hex : hexList) {
      bool ok;
      quint8 byte = hex.toUInt(&ok, 16);
      if (!ok) {
        qWarning() << "Invalid hex value:" << hex;
        continue; // 或者 return/throw，根据容错策略决定
      }
      byteArray.append(static_cast<char>(byte));
    }

    dataAllocator_->write(CommunicationConfiguration::CommProtocol::Raw, byteArray);
  } else {
    dataAllocator_->write(CommunicationConfiguration::CommProtocol::Raw, str.toLocal8Bit());
  }
}

void CommTerminal::setLineEditMode(QLineEdit *p, EncodingType type) {
  static QRegularExpression hexRegex("^([0-9A-Fa-f]{2}(\\s)?)*$");
  static QValidator *hexValidator =
      new QRegularExpressionValidator(hexRegex, p);

  QString currentText = p->text();

  if (type == EncodingType::Hex) {
    // 设置 HEX 校验器
    p->setValidator(hexValidator);

    // 转换当前文本为 HEX
    QByteArray asciiData = currentText.toUtf8();
    QString hexText = asciiData.toHex(' ').toUpper();
    p->setText(hexText);
  } else {
    // 清除校验器
    p->setValidator(nullptr);

    // 尝试将 Hex 转为 ASCII
    QString cleaned = currentText;
    cleaned.remove(' '); // 去除空格

    QByteArray asciiData;
    bool ok = true;
    if (cleaned.length() % 2 == 0 && !cleaned.isEmpty()) {
      asciiData = QByteArray::fromHex(cleaned.toLatin1());
    } else {
      ok = false; // 非偶数字节，不合法
    }

    if (ok)
      p->setText(QString::fromUtf8(asciiData));
    else
      p->clear(); // 或者提示用户“无效 Hex 字符串”
  }
}
