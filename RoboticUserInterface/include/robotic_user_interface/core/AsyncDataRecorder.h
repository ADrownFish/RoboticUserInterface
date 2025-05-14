#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

#include <QObject>

#include "../../../libraries/qt_gcw/QSnackbarManager.h"

namespace robot
{
  class AsyncDataRecorder : public QObject {
    Q_OBJECT
    using scalar_t = double;
    
    enum CurrentChannelIndex
    {
      channel1 = 0x00,
      channel2,
      count,
    };

    struct recordData{
      qreal _Timestamp_ms = 0;
      QVector<scalar_t> data;
    };

  private:
    bool _init = false;

    std::atomic<bool> _ThreadRunningFlag;

    // 时间戳相关
    uint64_t _startTimestamp_ms = 0;

    // 双缓冲区
    std::vector<recordData> _dataBuffer[CurrentChannelIndex::count];
    CurrentChannelIndex _currentChannelIndex = CurrentChannelIndex::channel1;
    CurrentChannelIndex _submitChannelIndex = CurrentChannelIndex::channel1;
    uint32_t _currentDataBufferIndex[CurrentChannelIndex::count] = {0};
    uint32_t _bufferSize = 5;

    // 子线程唤醒
    std::mutex _mutex_write;
    std::condition_variable _cv_write;
    bool _isReady_write = false;

    // 子线程
    std::thread _objThread;

    // 保存
    QString _filePath;
    int _savePrecision = 4;
    int _tableLength = 0;

  public:
    AsyncDataRecorder(QString fileName, QObject *parent = nullptr);
    ~AsyncDataRecorder();

  private:
    
    // 创建表
    bool createTable(const QStringList &columns);
    // 插入记录
    QString insertRecord(const recordData &values);
    // 执行写入操作
    void executeWrite();
    // 插入多条记录
    bool insertMultipleRecords(const recordData *records, int size);
    // 切换缓冲区通道
    void switchCurrentDataBuffer();
    //唤醒子线程
    void wakeUpObjThread();

    qint64 getCurrentTimestampMs();

  public:
    // 初始化
    void init(int bufferSize, const QStringList &columns);
    // 设置缓冲区大小
    void setBufferSize(uint32_t bufferSize);    
    // 重新设置起始时间
    void resetStartTime();
    // 获取当前数据库路径(包含文件名)
    QString getCurrentFilePath();
    //设置保存精度
    void setSavePrecision(int precision);

    // 提交一条数据记录
    void submitRecord(const QVector<scalar_t> &data);

  signals:
    void publishNotify(GCW::NotifyType type,const QString &title, const QString& text);

  private:
    void objThreadFun();
  };

}
