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
	enum DataType {
		Dynamic,    // 动态数据，不断新增的
		Static,         // 静态数据，数据量不变
	};

public:
	QVector<scalar_t> data;
	QVector<scalar_t> time;
	
	QString name;

	DataType type = DataType::Dynamic;
	bool enable = true;
	bool refreshOnceFlag = false;
	scalar_t timeWindow = 10.0;

private:
	bool zeroData = true;
	bool commonValue = false;


public:
	ObjectData(const QString &dataName){
		name = dataName;
	}

	void appendData(scalar_t time_, scalar_t data_) {

		// 如果时间被重置，则删除所有数据
		if(!time.isEmpty()){
			if(time_ < time.last()){
				data.clear();
				time.clear();
			}
		}

		// 如果数据是重复的，则更改标志位，防止后续axis缩放问题 TODO
		time.append(time_);
		data.append(data_);

		// 如果传进来的值 不为0，避免绘图出现缩放问题
		if (zeroData && data_ != 0.0) {
			zeroData = false;
		}
		if (type == ObjectData::Static) {
			return;
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

	bool isCommonValue() const {
		return commonValue;
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
	ObjectNode(const QString& nodeName) : name(nodeName) {

	}
	ObjectNode(const QString &nodeName, const QVector<ObjectData::Ptr>& nodeData)
			: name(nodeName), data(nodeData) {}

	void addNode(ObjectNode::Ptr d) {
		children.append(d); 	
	}

	void addObject(ObjectData::Ptr d) {
		data.append(d);
		//qDebug() << "addObject:" << d->name << "  " << QString(d->type == ObjectData::Static ? "Static" : "Dynamic");
	}

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

	QString findObjectDataPath(const ObjectData::Ptr od) const {
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

	bool exists(const ObjectData::Ptr od) const {
		if(data.contains(od)){
			return true;
		}

		for (auto& d : children) {
			if (d->exists(od)) {
				return true;
			}
		}

		return false;
	}

	bool exists(const ObjectData* od) const {
		for(auto p : data){
			if(p.get() == od){
				return true;
			}
		}

		for (auto& d : children) {
			if (d->exists(od)) {
				return true;
			}
		}

		return false;
	}


	void setAllStatic() {
    for (auto& d : data) {
      d->type = ObjectData::DataType::Static;
    }
    for (auto& d : children) {
      d->setAllStatic();
    }
	}

	void setrefreshOnce() {
		for (auto& d : data) {
			d->refreshOnceFlag = true;
		}
		for (auto& d : children) {
			d->setrefreshOnce();
		}
	}

	QString toString(int level = 0) const {
		QString indent(level * 2, ' ');
		QString result = indent + QString("Node: %1 %2\r\n").arg(name,QString::number(id));

		QString dataResult;
		for (const auto& d : data) {
			dataResult += QString("  %1(%2)(%3) ").arg(d->name).arg(d->data.size()).arg(d->type == ObjectData::Static ? "Static" : "Dynamic");
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

	void clear() {
		for (auto& d : data) {
			d.reset();
		}
		data.clear();

		for (auto& child : children) {
			child->clear();
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

public:

	static inline uint64_t timestamp_ms() {
		using namespace std::chrono;
		return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
	}

	static inline scalar_t timestamp_ms_f() {
		using namespace std::chrono;
        return static_cast<uint64_t>(duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count()) / 1000'000'000.;
	}
	
	static inline uint64_t timestamp_ns() {
		using namespace std::chrono;
		return static_cast<uint64_t>(duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count());
	}

private:
	ObjectNode::Ptr createAxis3Node(const QString& nodeName);
	ObjectNode::Ptr createAxisQuatNode(const QString& nodeName);

private:
	ObjectNode::Ptr topNode_;
	uint64_t startTime = 0;
};
