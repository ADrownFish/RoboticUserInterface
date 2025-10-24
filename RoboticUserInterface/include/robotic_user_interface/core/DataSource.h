#pragma once

#include <memory>
#include <iostream>
#include <cstdint>  
#include <chrono>

#include <QString>
#include <QObject>
#include <QDateTime>

#include "robotic_user_interface/core/Types.h"

class ObjectData {

public:
	using Ptr = std::shared_ptr<ObjectData>;
	enum DataType {
		Dynamic,    // 动态数据，不断新增的
		Static,     // 静态数据，数据量不变
	};

public:
	QVector<scalar_t> data;
	QVector<scalar_t> time;
	
	QString name;

	bool enable = true;
	bool refreshOnceFlag = false;
	DataType type = DataType::Dynamic;
	scalar_t timeWindow = 10.0;

public:
	explicit ObjectData(){
		name = "No Name";
	}

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

		time.append(time_);
		data.append(data_);

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

	void clear() {
		time.clear();
		data.clear();
	}
};

class ObjectNode {
public:
	using Ptr = std::shared_ptr<ObjectNode>;

public:
	int32_t id = 1;    // -1 means root, 0 means top, 1~n means child
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

  QString findPathFromObjectData(const ObjectData::Ptr &od,
                                 bool ContainsTopLevelNode = true) const {
    return findPathFromObjectData(od.get(), ContainsTopLevelNode);
  }

  QString findPathFromObjectData(const ObjectData *od,
                                 bool ContainsTopLevelNode = true) const {
    QString path;

    // 构建当前节点的路径部分
    if (this->id != -1) {  // 不是根节点
      if (this->id == 0) { // 顶层节点
        if (ContainsTopLevelNode) {
          path = this->name + "/";
        } else {
          path = ""; // 不包含顶层节点时，路径为空
        }
      } else { // 普通节点 (id > 0)
        path = this->name + "/";
      }
    } else {     // 根节点
      path = "";
    }

    // 在当前节点的直接子节点中查找
    for (auto &d : data) {
      if (d.get() == od) {
        return path + d->name;
      }
    }

    // 递归在子节点中查找
    for (auto &d : children) {
      QString result = d->findPathFromObjectData(od, ContainsTopLevelNode);
      if (!result.isEmpty()) {
        // 如果当前节点是根节点，直接返回子节点的结果（不添加任何前缀）
        if (this->id == -1) {
          return result;
        }
        // 如果当前节点是顶层节点且不包含顶层节点，直接返回子节点的结果
        else if (this->id == 0 && !ContainsTopLevelNode) {
          return result;
        }
        // 其他情况：拼接当前节点的路径和子节点的结果
        else {
          return path + result;
        }
      }
    }

    return QString();
  }

  /**
     * @brief 查找或创建对象数据
     * @param path 数据路径，如 "robot/imu/acc/x"
     * @param createIfNotExist 如果为true，路径不存在时自动创建
     * @return 找到或创建的对象数据指针，找不到返回nullptr
     */
  ObjectData::Ptr findObjectDataFromPath(const QString &path,
                                         bool createIfNotExist = false) {
    QStringList parts = path.split('/', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
      return nullptr;
    }

    ObjectNode *currentNode = this; // 从当前节点开始

    // 遍历路径中的节点部分（除最后一部分）
    for (int i = 0; i < parts.size() - 1; ++i) {
      const QString &part = parts[i];
      bool found = false;

      // 在当前节点的子节点中查找
      for (const auto &child : currentNode->children) {
        if (child->name == part) {
          currentNode = child.get();
          found = true;
          break;
        }
      }

      // 没找到且需要创建
      if (!found) {
        if (createIfNotExist) {
          auto newNode = std::make_shared<ObjectNode>(part);
          newNode->name = part;
          newNode->id = i;
          currentNode->children.push_back(newNode);
          currentNode = newNode.get();
        } else {
          return nullptr;
        }
      }
    }

    // 查找最后一部分的数据
    const QString &dataName = parts.last();
    for (const auto &data : currentNode->data) {
      if (data->name == dataName) {
        return data;
      }
    }

    // 没找到且要创建
    if (createIfNotExist) {
      auto newData = std::make_shared<ObjectData>();
      newData->name = dataName;
      currentNode->data.push_back(newData);
      return newData;
    }

    return nullptr;
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

	void setRefreshOnce() {
		for (auto& d : data) {
			d->refreshOnceFlag = true;
		}
		for (auto& d : children) {
			d->setRefreshOnce();
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

	void resetID(int32_t id_) {
		this->id = id_;

		for (auto& d : children) {
			d->id = id_ + 1;
			d->resetID(d->id);
		}
	}
	
	void resetID() {
		for (auto& d : children) {
			d->id = this->id + 1;
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

	ObjectNode::Ptr createPluginNode(int num_actuator);

	void clearData();         // 清除所有数据

	void resetDataSource();   // 重置，只剩Plugin

	const ObjectNode::Ptr& topNode(){ return topNode_;}

	void resetTime();
	
	scalar_t time() const;

	scalar_t startTime() const;

private:
	ObjectNode::Ptr createAxis3Node(const QString& nodeName);
	ObjectNode::Ptr createAxisQuatNode(const QString& nodeName);

private:
	ObjectNode::Ptr topNode_;
	scalar_t startTime_ = 0;
};
