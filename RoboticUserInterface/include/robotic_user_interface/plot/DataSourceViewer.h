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
	ObjectNodeViewer();

  ~ObjectNodeViewer();

  void addNode(ObjectNodeViewer::Ptr d);

  void addObject(ObjectDataViewer::Ptr d);

  void updateViewer();

  void clear();

  void clearData();

  ObjectDataViewer::Ptr findObjectDataViewer(const  QTreeWidgetItem* item) const;

  void populateTree(QTreeWidgetItem* parentItem, ObjectNode::Ptr node);
};



class DataSourceViewerTreeWidget : public QTreeWidget {
  Q_OBJECT
public:
  explicit DataSourceViewerTreeWidget(QWidget* parent = nullptr);

  void setTopNode(const ObjectNode::Ptr& p);

protected:
  void startDrag(Qt::DropActions supportedActions) override;

  void onItemExpanded(QTreeWidgetItem* item);


  // menu 
  void clearItem();
    
  void deleteItem();

  void mousePressEvent(QMouseEvent* event);

signals:
  void readyDrag(QList<QTreeWidgetItem*> item);

  void checkObjectData();

private:
  ObjectNode::Ptr topNode_;
  ObjectNode::Ptr selectedNode;
  QTreeWidgetItem* selectedItem = nullptr;

  QSharedPointer<FluMenu> menu;

  FluAction* action_eliminate_selection;
  FluAction* action_clear_item_data;
  FluAction* action_delete_item;
};

class DataSourceViewer : public QWidget {
  Q_OBJECT
public:
  DataSourceViewer(QWidget *parent = nullptr);
  ~DataSourceViewer();

  void setDataSource(const DataSource::Ptr &ds);

  void setConfiguration(std::shared_ptr<Configuration> config);

  void setSteamSolver(DataStreamSolver* ss);

  void init();

  void setupWidgetsControls();

  void setupSignalConnection();

  void updateNodeValue();

  QPointer<DataSourceViewerTreeWidget> getTreeWidget();
  
signals:
  void publishNotify(GCW::NotifyType type, const QString &title, const QString &text);
  
  void readyLoad(const QString& path);

  void checkObjectData();
  
private:
  void makeImage();
  
  void readyDrag(QList<QTreeWidgetItem*> item);
  
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
