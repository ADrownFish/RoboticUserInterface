#pragma once

#include <memory>
#include <iostream>
#include <chrono>   
#include <cstdint>  

#include <QString>
#include <QObject>
#include <QDateTime>

#include "robotic_user_interface/core/Types.h"

class ObjectData {

public:
	using Ptr = std::shared_ptr<ObjectData>;

public:
	QString name;

	bool enable = true;
	bool zeroData = true;
	scalar_t timeWindow = 10.0;

	QVector<scalar_t> data;
	QVector<scalar_t> time;

public:
	ObjectData(const QString &dataName){
		name = dataName;
	}

	void appendData(scalar_t time_, scalar_t data_) {
		time.append(time_);
		data.append(data_);
		if (zeroData && data_ != 0.0) {
			// 如果传进来的值 不为0，避免绘图出现缩放问题
			zeroData = false;
		}
		while (!time.isEmpty() && time.first() < time_ - timeWindow) {
			time.removeFirst();
			data.removeFirst();
		}
	}

	void setTimeWindow(scalar_t time) {
		timeWindow = time;
	}

	bool isZeroData() const {
		return zeroData;
	}

	void clear() {
		time.clear();
		data.clear();
		zeroData = true;
	}
};

class ObjectNode {
public:
	using Ptr = std::shared_ptr<ObjectNode>;

public:
	uint32_t id = 1;    // 0 means top
	QString name = "null";
	QVector<ObjectData::Ptr> data;
	QVector<ObjectNode::Ptr> children;

public:
	ObjectNode() {

	}
	ObjectNode(const QString &nodeName, const QVector<ObjectData::Ptr>& nodeData)
			: name(nodeName), data(nodeData) {}

	void addNode(ObjectNode::Ptr d) { 		children.append(d); 	}

	void addObject(ObjectData::Ptr d) { data.append(d); }

	ObjectData::Ptr findObjectData(const QString &name) const {
		for(auto &d : data){
			if(d->name == name){
				return d;
			}
		}

		return nullptr;
	}

	ObjectNode::Ptr findObjectNode(const QString& name) const {
		for (auto& d : children) {
			if (d->name == name) {
				return d;
			}
		}

		return nullptr;
	}

	QString findObjectDataPath(const  ObjectData::Ptr od) const {
		QString path;
		if (this->id != 0) {
			path = this->name + "/";
		}
		for (auto& d : data) {
			if (d.get() == od.get()) {
				return path + d->name;
			}
		}

		for (auto& d : children) {
			QString result = d->findObjectDataPath(od);
			if (!result.isEmpty()) {
				return path + result;
			}
		}

		return QString();
	}


	QString toString(int level = 0) const {
		QString indent(level * 2, ' ');
		QString result = indent + QString("Node: %1 %2\r\n").arg(name,QString::number(id));

		QString dataResult;
		for (const auto& d : data) {
			dataResult += QString("  %1[%2] ").arg(d->name).arg(d->data.size());
		}
		if (!dataResult.isEmpty()) {
			result += indent + dataResult + "\r\n";
		}
		
		for (const auto &child : children) {
			result += child->toString(level + 1);
		}

		return result;
	}

	void clearData() {
		for (auto &d : data) {
			d->data.clear();
			d->time.clear();
		}

		for (auto &child : children) {
			child->clearData();
		}
	}

	void resetID(uint32_t topID = 0) {
		for (auto& d : children) {
			d->id = topID + 1;
			d->resetID(d->id);
		}
	}

	void setTimeWindow(scalar_t time) {
		for (auto& d : data) {
			d->setTimeWindow(time);
		}

		for (auto& d : children) {
			d->setTimeWindow(time);
		}
	}
};

class DataSource : public QObject{
	Q_OBJECT
public:
	using Ptr = std::shared_ptr<DataSource>;

public:
	DataSource(int num_actuator = 20);

	ObjectNode::Ptr createTestNode();

	ObjectNode::Ptr createBaseNode(int num_actuator);

	void clearData();

	const ObjectNode::Ptr& topNode(){ return topNode_;}

	void resetTime();

	scalar_t time();

private:
	ObjectNode::Ptr createAxis3Node(const QString& nodeName);
	ObjectNode::Ptr createAxisQuatNode(const QString& nodeName);

	inline uint64_t timestamp_ms() {
		using namespace std::chrono;
		return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
	}

	inline uint64_t timestamp_ns() {
		using namespace std::chrono;
		return static_cast<uint64_t>(duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count());
	}
private:
	ObjectNode::Ptr topNode_;
	uint64_t startTime = 0;
};
