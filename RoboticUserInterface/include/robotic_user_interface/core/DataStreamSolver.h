#pragma once

#include "qwool/qwwindowwidget.h"
#include "qt_gcw/QSnackbarManager.h"

#include "robotic_user_interface/core/Types.h"
#include "robotic_user_interface/core/DataAllocator.h"

#include <QTimer>
#include <QMutex>

#include "robotic_user_interface/core/DataSource.h"

class DataStreamSolver : public QWWindowWidget {

	Q_OBJECT


public:

	DataStreamSolver(QWidget* parent = nullptr);

	~DataStreamSolver();

	void init();

	void setConfiguration(std::shared_ptr<Configuration> config);

	void setDataSource(const std::shared_ptr<DataSource>& p);

	void setDataAllocator(const QPointer<DataAllocator>& p);

	void appendData(const QByteArray& newData);

	QStringList extractCSVHeaders(const QString& filePath, bool hasHeader);

	void loadCSV(const QString& filePath,
												const QString& timeKey,
												bool hasHeader = true,
												const QString& pathSeparator = "/");

signals:
	void publishNotify(GCW::NotifyType type, const QString& title, const QString& text);

private:
	void setupSignalConnection();

private:

	std::shared_ptr<Configuration> config_;
	std::shared_ptr<DataSource>    dataSource_;
	QPointer<DataAllocator> dataAllocator_;

	ObjectNode::Ptr floatNode;
	ObjectNode::Ptr jsonNode;
	ObjectNode::Ptr csvNode;

	QTimer timer_;
	QMutex readMutex;

	// float
private:

	void processFloatData(const QByteArray& floatData);
	bool parseFloatPacket(const QByteArray& packet, QVector<float>& result);
	void dispatchFloatData(const QVector<float>& floatData, scalar_t timestamp);

	// Float字节流解析相关
	struct {
		QByteArray frameTail = QByteArray::fromHex("FFFFFFFF");
		QByteArray buffer;
		ObjectNode::Ptr root;
		const int MAX_FLOAT_COUNT = 200;  // 限制最大float数量
		const int MAX_PACKET_SIZE = MAX_FLOAT_COUNT * 4 + 4; // 最大数据包大小
	} floatParser_;


	// Json
private:
	enum JsonState { Start, InString, InEscape, InObject, InArray, Complete };
	struct {
		JsonState state = Start;
		int depth = 0;
		int startIndex = 0;
		QByteArray buffer;
		ObjectNode::Ptr root;
	} jsonParser_;
	
	void parseJsonBuffer();
	void parseJsonObject(const QByteArray& jsonData);
	void parseJsonValue(const QString& key, const QJsonValue& value, scalar_t timestamp, ObjectNode::Ptr parentNode);
};
