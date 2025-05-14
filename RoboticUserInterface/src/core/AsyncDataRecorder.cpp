#include "robotic_user_interface/core/AsyncDataRecorder.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace robot {

AsyncDataRecorder::AsyncDataRecorder(QString fileName, QObject *parent) 
: QObject(parent){
  _ThreadRunningFlag = true;
  _startTimestamp_ms = getCurrentTimestampMs();

  _filePath = fileName;

  qDebug() << "Data file path: " << _filePath;
}

AsyncDataRecorder::~AsyncDataRecorder() {
  // 切换保存通道,保存还没写入的值
  _submitChannelIndex = _currentChannelIndex;

  // 唤醒线程 清空最后的缓冲区
  wakeUpObjThread();

  _ThreadRunningFlag = false;
  if (_objThread.joinable()) {
    _objThread.join();
  }
}

QString AsyncDataRecorder::insertRecord(const recordData &values) {
  QString buffer;
  buffer.reserve(values.data.size()  * 24 + 32); // 按平均每列8字节+时间戳预估 
  QTextStream stream(&buffer);
  
  // 设置数值格式（等效原std::fixed和setprecision）
  stream.setRealNumberNotation(QTextStream::FixedNotation); 
  stream.setRealNumberPrecision(_savePrecision); 
  stream.setLocale(QLocale::c());  // 强制使用点号作为小数分隔符 

  // 时间戳输出 
  stream << values._Timestamp_ms;
  
  // 数据列处理 
  for (const auto& column : values.data)  {
    stream << "," << column;
  }
  stream << "\n";
  return buffer;
}

// 插入多条记录
bool AsyncDataRecorder::insertMultipleRecords(const recordData *records,
                                              int size) {
    QFile file(_filePath);
    if (!file.open(QIODevice::WriteOnly  | QIODevice::Append | QIODevice::Text)) {
      publishNotify(GCW::Warning, "AsyncDataRecorder" ,QString("File open: ") + file.errorString());
      return false;
    }
 
    QTextStream stream(&file);
    // stream.setAutoDetectUnicode(QStringConverter::Utf8);     // 强制UTF8编码 
    stream.setRealNumberNotation(QTextStream::FixedNotation); 
    stream.setRealNumberPrecision(_savePrecision); 

    // qDebug() << "recordData size: " << size ;
 
    try {
        for (int i = 0; i < size; ++i) {
            stream << records[i]._Timestamp_ms;    // 直接写入时间戳 
            for (const auto& column : records[i].data) {
                stream << "," << column;
            }
            stream << "\n";
        }
    } catch (const std::exception& e) {
      publishNotify(GCW::Warning, "AsyncDataRecorder" ,QString("File write: ") + e.what());
      file.close(); 
      return false;
    }
 
    stream.flush(); 
    file.close(); 
 
    return (stream.status()  == QTextStream::Ok) && (file.error()  == QFile::NoError);
}

void AsyncDataRecorder::objThreadFun() {
  while (_ThreadRunningFlag) {
    // 等待主线程发送信号
    std::unique_lock<std::mutex> lock(_mutex_write);
    _cv_write.wait(lock, [this] { return _isReady_write; });

    // 在这里执行子线程的操作
    executeWrite();
    // 结束

    // 重置标志，等待下一次信号
    _isReady_write = false;
  }
}
void AsyncDataRecorder::setBufferSize(uint32_t bufferSize) {
  _bufferSize = bufferSize;

  _dataBuffer[CurrentChannelIndex::channel1].resize(bufferSize);
  _dataBuffer[CurrentChannelIndex::channel2].resize(bufferSize);
}

void AsyncDataRecorder::init(int bufferSize,
                             const QStringList &columns) {
  if (_init) {
    publishNotify(GCW::Warning, "AsyncDataRecorder" ,"Already initialized");
    return;
  }
  // 重置缓冲区
  _currentChannelIndex = CurrentChannelIndex::channel1;
  _currentDataBufferIndex[CurrentChannelIndex::channel1] = 0;
  _currentDataBufferIndex[CurrentChannelIndex::channel2] = 0;

  setBufferSize(bufferSize);

  // 创建子线程
  std::function<void(void *)> fun =
      std::bind(&AsyncDataRecorder::objThreadFun, this);
  _objThread = std::thread(fun, this);

  createTable(columns);

  // 初始化完成
  _init = true;
}
void AsyncDataRecorder::submitRecord(const QVector<scalar_t> &data) {
  if (!_init) {
    publishNotify(GCW::Warning, "AsyncDataRecorder" ,"You must first call init() to initialize .");
    return;
  }

  if (data.size() != _tableLength) {
    publishNotify(GCW::Warning, "AsyncDataRecorder" ,"The number of elements and header is inconsistent.");
  }

  auto &_new_data = _dataBuffer[_currentChannelIndex].at(
      _currentDataBufferIndex[_currentChannelIndex]);
  _new_data.data = data;
  // qDebug() << "QVector<scalar_t> size: " << data.size() ;
  _new_data._Timestamp_ms =
      (getCurrentTimestampMs() - _startTimestamp_ms) / 1000.;
  _currentDataBufferIndex[_currentChannelIndex]++;

  // 如果缓冲区已满
  if (_currentDataBufferIndex[_currentChannelIndex] == _bufferSize) {
    switchCurrentDataBuffer();
    wakeUpObjThread();
  }
}

void AsyncDataRecorder::switchCurrentDataBuffer() {
  _submitChannelIndex = _currentChannelIndex;

  // 切换通道
  _currentChannelIndex = (CurrentChannelIndex)((((int)_currentChannelIndex) + 1) % CurrentChannelIndex::count);

  // 下一次的 = 0
  _currentDataBufferIndex[_currentChannelIndex] = 0;
}
void AsyncDataRecorder::executeWrite() {
  const auto &records = _dataBuffer[_submitChannelIndex].data();
  int recordsSize = _currentDataBufferIndex[_submitChannelIndex];

  bool ok = insertMultipleRecords(records, recordsSize);
}
void AsyncDataRecorder::resetStartTime() {
  _startTimestamp_ms = getCurrentTimestampMs();
}

QString AsyncDataRecorder::getCurrentFilePath() { return _filePath; }

void AsyncDataRecorder::wakeUpObjThread() {
  {
    std::lock_guard<std::mutex> lock(_mutex_write);
    _isReady_write = true;
  }

  _cv_write.notify_one();
}

bool AsyncDataRecorder::createTable(const QStringList &columns) {
    QFile file(_filePath);
    if (!file.open(QIODevice::WriteOnly  | QIODevice::Text)) {
      publishNotify(GCW::Warning, "AsyncDataRecorder" ,QString("create file error: ") + file.errorString());
      return false;
    }
 
    QTextStream stream(&file);
    // stream.setEncoding(QStringConverter::Utf8);   // 强制UTF8编码 
    stream.setFieldAlignment(QTextStream::AlignLeft); 

    try {
        stream << "timestamp";
        for (const auto& column : columns) {
            // if (column.contains(',')  || column.contains('\n'))  {
            //     qCritical() << "非法列名:" << column;
            //     return false;
            // }
            stream << "," << column.toUtf8().constData();  // 防乱码处理 
        }
        stream << "\n";  // 统一换行符 

        _tableLength = static_cast<int>(columns.size())  + 1; // timestamp计入列数 
        
    } catch (const std::exception& e) {
      publishNotify(GCW::Warning, "AsyncDataRecorder" ,QString("create tab header error: ") + e.what());
      file.close(); 
      return false;
    }

    stream.flush(); 
    file.close(); 
    return (stream.status()  == QTextStream::Ok) && (file.error()  == QFile::NoError);
}
void AsyncDataRecorder::setSavePrecision(int precision) {
  _savePrecision = precision;
}

qint64 AsyncDataRecorder::getCurrentTimestampMs() {
  return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

} // namespace sevnce
