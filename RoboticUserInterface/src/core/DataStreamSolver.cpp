#include "robotic_user_interface/core/DataStreamSolver.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QCoreApplication>

#include <QFile>
#include <QTextStream>
#include <QDebug>

DataStreamSolver::DataStreamSolver(QWidget* parent) {



}

DataStreamSolver::~DataStreamSolver() {



}

void DataStreamSolver::init() {

	setupSignalConnection();
}

void DataStreamSolver::setConfiguration(std::shared_ptr<Configuration> config) {
	config_ = config;
}

void DataStreamSolver::setDataSource(const std::shared_ptr<DataSource>& p)
{
  dataSource_ = p;

  floatNode = std::make_shared<ObjectNode>();
  floatNode->name = "Float";

  jsonNode = std::make_shared<ObjectNode>();
  jsonNode->name = "Json";

  csvNode = std::make_shared<ObjectNode>();
  csvNode->name = "Csv";

  ObjectNode::Ptr topNode = dataSource_->topNode();
  topNode->addNode(floatNode);
  topNode->addNode(jsonNode);
  topNode->addNode(csvNode);
  topNode->resetID();

  floatParser_.root = floatNode;
  jsonParser_.root = jsonNode;
}

void DataStreamSolver::setDataAllocator(const QPointer<DataAllocator>& p) {
	dataAllocator_ = p;
}

void DataStreamSolver::setActivate(bool ok)
{
  if (ok) {
    timer_.start();
  }
  else {
    timer_.stop();
  }
}

void DataStreamSolver::setupSignalConnection() {
  using CCC = CommunicationConfiguration::CommProtocol;

  // 数据传递
  QObject::connect(dataAllocator_, &DataAllocator::readyRead, [this]() {
    // on signal thread
    if (config_->comm.commProtocol == CCC::JSON ||
      config_->comm.commProtocol == CCC::Float
      ) {
      QByteArray buffer;
      dataAllocator_->read(config_->comm.commProtocol, buffer);

      readMutex.lock();
      recviveBuffer_.append(buffer);
      readMutex.unlock();
    }
    });

  QObject::connect(&timer_, &QTimer::timeout, this, &DataStreamSolver::processData);
}


void DataStreamSolver::processData() {
  QByteArray buffer;

  readMutex.lock();
  if (recviveBuffer_.isEmpty()) {
    readMutex.unlock();
    return;
  }
  buffer = std::move(recviveBuffer_);
  readMutex.unlock();

  if (config_->comm.commProtocol == CommunicationConfiguration::CommProtocol::JSON){
    jsonParser_.buffer.append(buffer);
    parseJsonBuffer();
  } else if (config_->comm.commProtocol == CommunicationConfiguration::CommProtocol::Float) {
    floatParser_.buffer.append(buffer);
    parseFloatData();
  }
}

QStringList DataStreamSolver::extractCSVHeaders(const QString& filePath, bool hasHeader) {
  QStringList headers;
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "Failed to open file:" << filePath;
    return headers;
  }

  QTextStream in(&file);
  if (hasHeader && !in.atEnd()) {
    QString headerLine = in.readLine();
    headers = headerLine.split(',');
  }
  else {
    // 如果没有表头，则读取第一行确定列数
    if (!in.atEnd()) {
      QString dataLine = in.readLine();
      int colCount = dataLine.split(',').size();
      for (int i = 0; i < colCount; ++i) {
        headers.append(QString("col_%1").arg(i));
      }
    }
  }
  file.close();
  return headers;
}


void DataStreamSolver::loadCSV(const QString& filePath,
                                                     const QString& timeKey,
                                                     bool hasHeader,
                                                     const QString& pathSeparator)
{
  // 清理数据
  //csvNode->clear();
  const int timeout = 10;
  QElapsedTimer timer;
  timer.start();

  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Failed to open file:" << filePath;
    return;
  }

  QTextStream in(&file);
  QStringList headers;
  int timeColIndex = -1;

  // 处理表头
  if (hasHeader && !in.atEnd()) {
    headers = in.readLine().split(',');
    timeColIndex = headers.indexOf(timeKey);
  }
  else if (!in.atEnd()) {
    // 无表头时自动生成列名
    QString firstLine = in.readLine();
    QStringList firstData = firstLine.split(',');
    for (int i = 0; i < firstData.size(); ++i) {
      headers.append(QString("col_%1").arg(i));
    }
    timeColIndex = (timeKey.isEmpty()) ? -1 : timeKey.toInt();
  }

  // 构建数据节点树
  QVector<ObjectData::Ptr> dataObjects(headers.size(), nullptr);
  for (int col = 0; col < headers.size(); ++col) {

    //  处理一次事件，防止卡死
    if (timer.elapsed() > timeout) {
      QCoreApplication::processEvents();
      timer.restart();
    }

    if (col == timeColIndex) continue;  // 跳过时间列

    QString fullPath = headers[col];
    QStringList parts = fullPath.split(pathSeparator, Qt::SkipEmptyParts);

    ObjectNode* currentNode = csvNode.get();
    // 遍历路径节点
    for (int i = 0; i < parts.size() - 1; ++i) {
      ObjectNode::Ptr childNode = currentNode->findObjectNode(parts[i]);
      if (!childNode) {
        childNode = std::make_shared<ObjectNode>(parts[i], QVector<ObjectData::Ptr>());
        currentNode->addNode(childNode);
      }
      currentNode = childNode.get();
    }
    // 创建数据节点
    ObjectData::Ptr dataObj = currentNode->findObjectData(parts.last());
    if (!dataObj) {
      dataObj = std::make_shared<ObjectData>(parts.last());
      currentNode->addObject(dataObj);
    } else {
      dataObj->clear();      
    }

    dataObj->refreshOnceFlag = true;
    dataObj->type = ObjectData::DataType::Static;

    dataObjects[col] = dataObj;
  }

  // 处理数据行
  int rowIndex = 0;
  if (!hasHeader) {
    in.seek(0);  // 无表头时重置到文件开头
    in.readLine(); // 跳过已读的第一行
  }

  while (!in.atEnd()) {
    //  处理一次事件，防止卡死
    if (timer.elapsed() > timeout) {
      QCoreApplication::processEvents();
      timer.restart();
    }

    QString line = in.readLine();
    QStringList values = line.split(',');
    if (values.isEmpty()) continue;

    // 确定时间戳
    scalar_t timestamp = rowIndex;
    if (timeColIndex >= 0 && timeColIndex < values.size()) {
      bool ok;
      scalar_t t = values[timeColIndex].toDouble(&ok);
      if (ok) {
        timestamp = t;
      }
    }

    // 填充数据
    int maxCol = qMin(values.size(), dataObjects.size());
    for (int col = 0; col < maxCol; ++col) {
      if (col == timeColIndex || !dataObjects[col]) continue;

      bool ok;
      scalar_t value = values[col].toDouble(&ok);
      if (ok) {
        dataObjects[col]->appendData(timestamp, value);
      }
    }
    rowIndex++;
  }

  file.close();
  csvNode->resetID(1);  // 重置节点ID

  std::cout  << csvNode->toString().toLocal8Bit().data() << std::endl;
}

void DataStreamSolver::parseJsonBuffer() {
  for (int i = 0; i < jsonParser_.buffer.size(); ++i) {
    const char c = jsonParser_.buffer[i];

    switch (jsonParser_.state) {
    case Start:
      if (c == '{') {
        jsonParser_.state = InObject;
        jsonParser_.depth = 1;
        jsonParser_.startIndex = i;
      }
      break;

    case InObject:
    case InArray:
      if (c == '"') {
        jsonParser_.state = InString;
      }
      else if (c == '{' || c == '[') {
        jsonParser_.depth++;
      }
      else if (c == '}' || c == ']') {
        jsonParser_.depth--;
        if (jsonParser_.depth == 0) {
          jsonParser_.state = Complete;
        }
      }
      break;

    case InString:
      if (c == '\\') {
        jsonParser_.state = InEscape;
      }
      else if (c == '"') {
        jsonParser_.state = (jsonParser_.depth > 0) ? InObject : Start;
      }
      break;

    case InEscape:
      jsonParser_.state = InString;
      break;

    case Complete:
      break;
    }

    if (jsonParser_.state == Complete) {
      const int length = i - jsonParser_.startIndex + 1;
      const QByteArray jsonData = jsonParser_.buffer.mid(jsonParser_.startIndex, length);
      jsonParser_.buffer.remove(0, jsonParser_.startIndex + length);
      i = -1;  // 重置索引
      jsonParser_.startIndex = 0;
      jsonParser_.state = Start;

      parseJsonObject(jsonData);
    }
  }
}

void DataStreamSolver::parseJsonObject(const QByteArray& jsonData) {
  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);

  if (error.error != QJsonParseError::NoError || !doc.isObject()) {
    return;
  }

  scalar_t timestamp = 0;
  QJsonObject rootObj = doc.object();
  if (config_->stream.timestampEnable_json && rootObj.contains(config_->stream.timestampString_json)) {
    timestamp = rootObj.value(config_->stream.timestampString_json).toDouble();
  }  else{
    timestamp = dataSource_->time();
  }

  bool emitSignal = false;

  for (auto it = rootObj.constBegin(); it != rootObj.constEnd(); ++it) {
    parseJsonValue(it.key(), it.value(), timestamp, jsonParser_.root, emitSignal);
  }

  if (emitSignal) {
    emit updateTree("Json");
    std::cerr << jsonParser_.root->toString().toLocal8Bit().data();
    jsonParser_.root->resetID(1);
    jsonParser_.root->setTimeWindow(config_->plot.cacheDuration);
  }
}

void DataStreamSolver::parseJsonValue(const QString& key, const QJsonValue& value, scalar_t timestamp, ObjectNode::Ptr parentNode, bool& hasNew) {
	if (value.isDouble() || value.isBool()) {
		scalar_t val = value.toDouble();
		if (value.isBool()) {
			val = value.toBool() ? 1.0 : 0.0;
		}

		ObjectData::Ptr data = parentNode->findObjectData(key);
		if (!data) {
			data = std::make_shared<ObjectData>(key);
			data->type = ObjectData::DataType::Dynamic;
			parentNode->addObject(data);

			hasNew = true;
		}
		data->appendData(timestamp, val);
	}
	else if (value.isObject()) {
		ObjectNode::Ptr node = parentNode->findObjectNode(key);
		if (!node) {
			node = std::make_shared<ObjectNode>(key);
			parentNode->addNode(node);
		}

		QJsonObject obj = value.toObject();
		for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
			parseJsonValue(it.key(), it.value(), timestamp, node, hasNew);
		}
	}
	else if (value.isArray()) {
		QJsonArray arr = value.toArray();
		for (int i = 0; i < arr.size(); ++i) {
			QString indexedKey = QString("%1[%2]").arg(key).arg(i);
			parseJsonValue(indexedKey, arr[i], timestamp, parentNode, hasNew);
		}
	}

  // 忽略其他类型（字符串等）
}


void DataStreamSolver::parseFloatData() {

  bool emitSignal = false;

  while (true) {
    // 查找帧尾
    int tailPos = floatParser_.buffer.indexOf(floatParser_.frameTail);
    if (tailPos == -1) {
      // 如果缓冲区过大，丢弃部分数据防止内存溢出
      if (floatParser_.buffer.size() > floatParser_.MAX_PACKET_SIZE * 2) {
        // 保留最后可能包含不完整帧尾的部分
        floatParser_.buffer = floatParser_.buffer.right(floatParser_.MAX_PACKET_SIZE);
      }
      break;
    }

    // 检查数据包长度是否合法
    int packetSize = tailPos + 4;
    if (packetSize > floatParser_.MAX_PACKET_SIZE) {
      //qWarning() << "Float packet too large (" << packetSize << "bytes), discarding...";
      floatParser_.buffer.remove(0, packetSize);
      continue;
    }

    // 提取完整数据包
    QByteArray packet = floatParser_.buffer.left(packetSize);
    floatParser_.buffer.remove(0, packetSize);

    // 解析数据包
    QVector<float> floatData;
    if (parseFloatPacket(packet, floatData)) {
      
      if (floatData.isEmpty()) {
        return;
      }

      scalar_t timestamp = 0.0;
      if (config_->stream.timestampEnable_float) {
        timestamp = floatData.front();
      } else {
        timestamp = dataSource_->time();
      }

      // 如果存在直接给值
      for (int i = 0; i < floatData.size(); ++i) {
        QString dataName = QString("Data %1").arg(i + 1);
        auto objData = floatParser_.root->findObjectData(dataName);
        if (!objData) {
          objData = std::make_shared<ObjectData>(dataName);
          objData->type = ObjectData::DataType::Dynamic;
          floatParser_.root->addObject(objData);
          
          emitSignal = true;
        }
        objData->appendData(timestamp, floatData[i]);
      }
    }
  }

  if (emitSignal) {
    emit updateTree("Float");
    qDebug() << floatParser_.root->toString();
    floatParser_.root->resetID(1);
    floatParser_.root->setTimeWindow(config_->plot.cacheDuration);
  }
}

bool DataStreamSolver::parseFloatPacket(const QByteArray& packet, QVector<float>& result) {
  // 检查最小长度和格式
  if (packet.size() < 8 || (packet.size() - 4) % 4 != 0) {
    return false;
  }

  // 检查帧尾
  if (packet.right(4) != floatParser_.frameTail) {
    return false;
  }

  // 计算float数量
  int floatCount = (packet.size() - 4) / 4;
  if (floatCount > floatParser_.MAX_FLOAT_COUNT) {
    return false;
  }

  // 解析float数据
  result.resize(floatCount);
  const char* data = packet.constData();
  for (int i = 0; i < floatCount; ++i) {
    // 直接内存拷贝，避免额外内存分配
    memcpy(&result[i], data + i * 4, sizeof(float));
  }

  return true;
}
