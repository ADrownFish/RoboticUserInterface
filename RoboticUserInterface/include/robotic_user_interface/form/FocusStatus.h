#pragma once

#include <QMap>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>

#include <Map>

#include "qt_gcw/QSnackbarManager.h"
#include "qwool/qwwindowwidget.h"

#include "robotic_user_interface/form/ActuatorDisplay.h"
#include "robotic_user_interface/core/Types.h"


class StatusItem : public QWidget {
  Q_OBJECT
public:
  enum class DisplayMode {
    normal,
    vertical,
  };
public:
  StatusItem(QWidget* parent = nullptr);
  ~StatusItem();

  void setBackgroundColor(uint32_t key);
  void setBackgroundColor(uint32_t key, const QColor& color);
  void setSvg(uint32_t key, const QString& res);
  void setDescribe(uint32_t key, const QString& str);
  void clearDescribe();

  void setBorderRadius(int value);
  int getBorderRadius();

  void setInterval(int value);
  int getInterval();

  void setDisplayMode(DisplayMode value);
  DisplayMode getDisplayMode();

  void resetFixedWidth();

protected:
  void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

private:
  int borderRadius_ = 20;
  int interval = 2;
  int enableBackground = 0;

  QMap<uint32_t, QColor> backgrounds_;
  QMap<uint32_t, std::shared_ptr<QSvgRenderer>> svgs_;
  QMap<uint32_t, QString> texts_;

  DisplayMode mode = DisplayMode::normal;
};

class FocusStatus : public QObject
{
  Q_OBJECT

public:
  enum StatusItemEnum{
    batterySoc,
    netType,
    netSpeed,
    tempDriver,
    tempMotor,
  };

public:
  struct TopStatusValue {
    uint64_t uploadBytes = 0;
    uint64_t downloadBytes = 0;
    uint64_t lastUploadBytes = 0;
    uint64_t lastDownloadBytes = 0;

    uint64_t time = 0;
  };
  
public:
  FocusStatus(QObject*parent = nullptr);
  ~FocusStatus();

  void setConfiguration(std::shared_ptr<Configuration> config);

  void setObservations(std::shared_ptr<ObservationsBase> config);

  void flushConfiguration();

  void appedUploadBytes(int byte);

  void appendDownloadByte(int byte);

  void setCommStatus(bool status);

  QWidget* getStatusItemsWidget();


private:
  void flush();

  QString formatByteRateUnit(qint64 bytes);
  
  QString formatBatteryUnit(double level);

  void createStatusItems();

signals:
  void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

private:
  QTimer timer_flush_;

  TopStatusValue value_;

  std::map<StatusItemEnum, StatusItem*> items;
  QWidget *itemsWidget;

  std::shared_ptr<Configuration> config_;
  std::shared_ptr<ObservationsBase> observations_;
};
