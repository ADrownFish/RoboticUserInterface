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


class QWidgetPrototypeTester : public QWidget{
	Q_OBJECT

public:
	QWidgetPrototypeTester(QWidget *parent = nullptr);
	 

	QtMaterialComboBox *combobox;
};

