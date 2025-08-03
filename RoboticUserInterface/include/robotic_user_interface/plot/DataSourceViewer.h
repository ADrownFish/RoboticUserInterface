#pragma once

#include <QApplication>
#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QPointer>
#include <QDrag>
#include <QMimeData>

#include "robotic_user_interface/core/DataSource.h"
#include "robotic_user_interface/core/DataStreamSolver.h"
#include "robotic_user_interface/form/CsvLoadDialog.h"

#include "qwool/qwdropwidget.h"
#include "FluControls/FluAction.h"
#include "FluControls/FluMenu.h"
#include "qt_gcw/QSnackbarManager.h"
#include "qcustomplot/qcustomplot.h"
#include "qt_material_widgets/qtmaterialdialog.h"

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

  void clear() {
    for (auto& d : data) {
      d.reset();
    }
    data.clear();

    for (auto& d : children) {
      d->clear();
    }
    children.clear();
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

  void setSteamSolver(DataStreamSolver* ss);

  void init();

  void setupWidgetsControls();

  void setupSignalConnection();

  void updateNodeValue();

  
signals:
  void publishNotify(GCW::NotifyType type, const QString &title, const QString &text);
  
  void readyLoad(const QString& path);

  void updateObjectData();
  
private:
  void makeImage();
  
  void readyDrag(QTreeWidgetItem* item);
  
  void flushItem();
  
  void clearAllData();

  void clearItsData(QMenu *subMenu);

  void removeItem(QMenu *subMenu);

  void resetTree();

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;

private:
  // 绘图
  QPixmap Pixmap;
  QColor backgroudColor = QColor(20, 20, 20);

  QPointer<DataSourceViewerTreeWidget> treeWidget;
  DataSource::Ptr dataSource_;

  ObjectNodeViewer::Ptr viewer_plugin;
  ObjectNodeViewer::Ptr viewer_csv;
  ObjectNodeViewer::Ptr viewer_float;
  ObjectNodeViewer::Ptr viewer_json;

  QTreeWidgetItem* item_csv;
  QTreeWidgetItem* item_plugin;
  QTreeWidgetItem* item_float;
  QTreeWidgetItem* item_json;

  //防止刷新过快
  QElapsedTimer elapsedTimer;
  qint64 lastEnterTime = 0;

  QTimer clock_flush;
  QFont uiFont;
  std::shared_ptr<Configuration> config_;

  uint32_t BorderRadius = 10;
  bool isActive = false;

  QVBoxLayout *layout;

  // 数据解析器
  QPointer<DataStreamSolver> dataStreamSolver_ = nullptr;

  // 上下拉按钮
  QPointer<QWDropWidget>     dropButton_;

  // 菜单
  FluMenu* menu;


  // load csv 部分
  QPointer<QtMaterialDialog> SelectorDialog_;
  QPointer<CsvLoadDialog>     csvLoadDialog_;
};
