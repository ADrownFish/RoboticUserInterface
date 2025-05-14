#pragma once

#include <QApplication>
#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QPointer>
#include <QDrag>
#include <QMimeData>

#include "robotic_user_interface/core/DataSource.h"
#include "qt_gcw/QSnackbarManager.h"
#include "qcustomplot/qcustomplot.h"

class ObjectDataViewer {

public:
	using Ptr = std::shared_ptr<ObjectDataViewer>;

public:
	ObjectData::Ptr data;
  QTreeWidgetItem* item = nullptr;
  //std::shared_ptr<QCPGraph> graph;

public:
	ObjectDataViewer() {

	}
	ObjectDataViewer(const ObjectData::Ptr& od) {
		data = od;
	}
};

class ObjectNodeViewer {
public:
	using Ptr = std::shared_ptr<ObjectNodeViewer>;

public:
  QTreeWidgetItem* item;
  QVector<ObjectDataViewer::Ptr> data;
  QVector<ObjectNodeViewer::Ptr> children;

public:
	ObjectNodeViewer() {

	}

  ~ObjectNodeViewer() {
    for (auto& d : children) {
      d.reset();
    }

    for (auto& d : data) {
      d.reset();
    }
  }

  void addNode(ObjectNodeViewer::Ptr d) { children.append(d); }

  void addObject(ObjectDataViewer::Ptr d) { data.append(d); }

  void updateViewer() {
    for (auto& d : data) {
      if (d->data->data.count()) {
        scalar_t value = d->data->data.back();
        d->item->setText(1, QString::number(value, 'f', 2));
      } else {
        d->item->setText(1, QString::number(0, 'f', 2));
      }
    }

    for (auto& d : children) {
      d->updateViewer();
    }
  }

  ObjectDataViewer::Ptr findObjectDataViewer(const  QTreeWidgetItem* item) const {
    for (auto& d : data) {
      if (d->item == item) {
        return d;
      }
    }

    for (auto& d : children) {
      ObjectDataViewer::Ptr result = d->findObjectDataViewer(item);
      if (result) {
        return result;
      }
    }

    return nullptr;
  }

  void populateTree(QTreeWidgetItem* parentItem, ObjectNode::Ptr node) {
    item = parentItem;
    item->setText(0, node->name);

    for (const auto& dataObj : node->data) {
      auto dataViewer = std::make_shared<ObjectDataViewer>(dataObj);
      QTreeWidgetItem* dataItem = new QTreeWidgetItem(item, QStringList(dataObj->name));
      dataViewer->item = dataItem;

      addObject(dataViewer);
    }

    for (const auto& childNode : node->children) {
      auto childViewer = std::make_shared<ObjectNodeViewer>();
      QTreeWidgetItem * newItem = new QTreeWidgetItem(parentItem);
      childViewer->populateTree(newItem, childNode);

      addNode(childViewer);
    }
  }
};

class DataSourceViewerTreeWidget : public QTreeWidget {
  Q_OBJECT
public:
  explicit DataSourceViewerTreeWidget(QWidget* parent = nullptr) : QTreeWidget(parent) {
    setDragEnabled(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    connect(this, &QTreeWidget::itemExpanded, this, &DataSourceViewerTreeWidget::onitemExpanded);  }

protected:
  void startDrag(Qt::DropActions supportedActions) override {
    QTreeWidgetItem* item = currentItem();
		if (!item) 
      return;
    
    readyDrag(item);
  }

  void onitemExpanded(QTreeWidgetItem* item) {
    this->resizeColumnToContents(0);  // 自动调整列宽
  }

signals:
  void readyDrag(QTreeWidgetItem* item);
};

class DataSourceViewer : public QWidget {
  Q_OBJECT
public:
  DataSourceViewer(QWidget *parent = nullptr);
  ~DataSourceViewer();

  void updateDataSource (const DataSource::Ptr &ds);

  void setConfiguration(std::shared_ptr<Configuration> config);

  void init();

  void setupWidgetsControls();

  void setupSignalConnection();

  void updateNodeValue();

signals:
  void publishNotify(GCW::NotifyType type, const QString &title, const QString &text);

private:
  void makeImage();

  void readyDrag(QTreeWidgetItem* item);

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  // 绘图
  QPixmap Pixmap;
  QColor backgroudColor = QColor(20, 20, 20);

  QPointer<DataSourceViewerTreeWidget> treeWidget;
  ObjectNodeViewer::Ptr objectNodeViewer_;
  DataSource::Ptr dataSource_;

  QTimer clock_flush;
  QFont uiFont;
  std::shared_ptr<Configuration> config_;

  uint32_t BorderRadius = 10;
  bool isActive = false;
};
