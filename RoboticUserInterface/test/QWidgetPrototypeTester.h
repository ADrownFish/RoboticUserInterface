#pragma once

#include <QWidget>

#include "qt_material_widgets/qtmaterialcombobox.h"


#include <QApplication>
#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProcess>
#include <QKeyEvent>
#include <QDebug>

class SSHConsole : public QWidget {
  Q_OBJECT

public:
  SSHConsole(QWidget* parent = nullptr) : QWidget(parent) {
    // 创建 UI 控件
    inputBox = new QLineEdit(this);
    inputBox->setPlaceholderText("请输入命令...");
    outputBox = new QTextEdit(this);
    outputBox->setReadOnly(true);  // 设置为只读
    sendButton = new QPushButton("发送", this);

    // 布局设置
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(outputBox);
    layout->addWidget(inputBox);
    layout->addWidget(sendButton);
    setLayout(layout);

    // 连接信号与槽
    connect(sendButton, &QPushButton::clicked, this, &SSHConsole::executeCommand);
    connect(inputBox, &QLineEdit::returnPressed, this, &SSHConsole::executeCommand);

    // 设置QProcess
    process = new QProcess(this);
    connect(process, &QProcess::readyReadStandardOutput, this, &SSHConsole::handleProcessOutput);
    connect(process, &QProcess::readyReadStandardError, this, &SSHConsole::handleProcessError);
  }

private slots:
  // 执行命令
  void executeCommand() {
    QString command = inputBox->text();
    if (!command.isEmpty()) {
      outputBox->append("> " + command);  // 显示输入的命令

      // 启动进程执行命令
      process->start(command);
      inputBox->clear();  // 清空输入框
    }
  }

  // 处理输出
  void handleProcessOutput() {
    QByteArray output = process->readAllStandardOutput();
    outputBox->append(output);  // 将命令的输出显示在输出框中
  }

  // 处理错误输出
  void handleProcessError() {
    QByteArray error = process->readAllStandardError();
    outputBox->append("ERROR: " + error);  // 将错误输出显示在输出框中
  }

private:
  QLineEdit* inputBox;
  QTextEdit* outputBox;
  QPushButton* sendButton;
  QProcess* process;
};


#include <QWidget>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QStyleOption>

// 内部自绘轨道部件
class RangeTrack : public QWidget {
  Q_OBJECT
    Q_PROPERTY(double cursorScale0 READ cursorScale0 WRITE setCursorScale0)
    Q_PROPERTY(double cursorScale1 READ cursorScale1 WRITE setCursorScale1)
public:
  RangeTrack(QWidget* parent = nullptr);

  double cursorScale0() const { return m_cursorScale[0]; }
  double cursorScale1() const { return m_cursorScale[1]; }
  void setCursorScale0(double s) {
    if (!qFuzzyCompare(m_cursorScale[0], s)) {
      m_cursorScale[0] = s;
      update();
    }
  }
  void setCursorScale1(double s) {
    if (!qFuzzyCompare(m_cursorScale[1], s)) {
      m_cursorScale[1] = s;
      update();
    }
  }

  // 设置轨道高度和游标尺寸
  void setTrackHeight(int h) { m_trackHeight = h; update(); }
  void setCursorSize(int w, int h) { m_cursorWidth = w; m_cursorHeight = h; update(); }

  // 设置值（0.0-1.0 归一化）
  void setLeftValue(double v);
  void setRightValue(double v);
  double leftValue() const { return m_leftPos; }
  double rightValue() const { return m_rightPos; }

  // 允许外部获取或设置实际数值（从归一化映射）
  void setValueRange(double min, double max) { m_minVal = min; m_maxVal = max; }
  double valueRangeMin() const { return m_minVal; }
  double valueRangeMax() const { return m_maxVal; }

signals:
  // 值改变信号（归一化 0-1）
  void valuesChanged(double left, double right);

protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  bool event(QEvent* event) override;

private:
  // 自定义属性用于动画
  double m_cursorScale[2]; // 游标缩放比例（index 0-左游标，1-右游标）
  double cursorScale(int idx) const { return m_cursorScale[idx]; }
  void setCursorScale(int idx, double s) { m_cursorScale[idx] = s; update(); }

  // 友元声明方便动画访问私有属性
  friend class QPropertyAnimation;

  // 最大值最小值（用于数值模式映射）
  double m_minVal = 0.0, m_maxVal = 100.0;
  // 归一化位置 0.0-1.0
  double m_leftPos = 0.25, m_rightPos = 0.75;
  // 鼠标交互状态
  int m_hoveredCursor = -1; // 当前悬停的游标索引
  int m_selectedCursor = -1; // 当前选中的游标索引
  bool m_dragging = false;
  // 轨道和游标样式参数
  int m_trackHeight = 10;
  int m_cursorWidth = 4, m_cursorHeight = 30;

  // 动画对象
  QPropertyAnimation* m_anim[2]; // 两个游标的动画

  // 辅助函数：将实际值映射到归一化比例
  double valueToPos(double val) const {
    if (m_maxVal == m_minVal) return 0;
    return (val - m_minVal) / (m_maxVal - m_minVal);
  }
  double posToValue(double pos) const {
    return m_minVal + pos * (m_maxVal - m_minVal);
  }
  // 更新模式显示器和绘制
  void updateCursorPos(int idx, double ratio);
};

class QWRangeSelector : public QWidget
{
  Q_OBJECT
public:
  enum Mode { Percentage, Value };

  explicit QWRangeSelector(QWidget* parent = nullptr);
  ~QWRangeSelector();

  // 设置模式（百分比或数值）
  void setMode(Mode m);
  Mode mode() const { return m_mode; }

  // 设置轨道高度和游标尺寸
  void setTrackHeight(int h);
  void setCursorSize(int w, int h);

  // 设置数值范围（默认为 0-100）
  void setRange(double minVal, double maxVal);

  // 获取/设置起始和结束值（根据当前模式，数值或百分比）
  double startValue() const;
  double endValue() const;
  void setStartValue(double v);
  void setEndValue(double v);

signals:
  // 范围值改变时发出信号
  void rangeChanged(double start, double end);

private slots:
  // 输入框编辑结束时更新对应值
  void onStartEditingFinished();
  void onEndEditingFinished();

private:

  // 成员变量
  Mode m_mode = Percentage;
  RangeTrack* m_trackWidget;
  QLineEdit* m_startEdit;
  QLineEdit* m_endEdit;
  double m_minVal = 0.0, m_maxVal = 100.0;

  // 内部布局与样式设置
  void initUi();
  void initConnections();
  QString valueToString(double v) const;
  double stringToValue(const QString& s) const;
};




class QWidgetPrototypeTester : public QWidget{
	Q_OBJECT

public:
	QWidgetPrototypeTester(QWidget *parent = nullptr);
	 

	QtMaterialComboBox *combobox;
};

