#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>

#include "robotic_user_interface//core/BluetoothManager.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr)
    : QMainWindow(parent)
  {
    setupUI();
    setupConnections();

    // 检查蓝牙可用性
    if (!m_bluetoothManager.isBluetoothAvailable()) {
      QMessageBox::warning(this, "警告", "本设备蓝牙不可用");
    }
  }

  ~MainWindow() = default;

private:
  void setupUI()
  {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // 状态显示
    m_statusLabel = new QLabel("状态: 未连接", this);
    mainLayout->addWidget(m_statusLabel);

    // 设备列表
    m_devicesList = new QListWidget(this);
    mainLayout->addWidget(new QLabel("发现的设备:"));
    mainLayout->addWidget(m_devicesList);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_scanButton = new QPushButton("扫描设备", this);
    buttonLayout->addWidget(m_scanButton);

    m_connectButton = new QPushButton("连接设备", this);
    buttonLayout->addWidget(m_connectButton);

    m_disconnectButton = new QPushButton("断开连接", this);
    buttonLayout->addWidget(m_disconnectButton);

    m_clearButton = new QPushButton("清空列表", this);
    buttonLayout->addWidget(m_clearButton);

    mainLayout->addLayout(buttonLayout);

    // 数据发送区域
    mainLayout->addWidget(new QLabel("发送数据:"));
    m_sendText = new QTextEdit(this);
    m_sendText->setMaximumHeight(60);
    mainLayout->addWidget(m_sendText);

    QPushButton* sendButton = new QPushButton("发送", this);
    mainLayout->addWidget(sendButton);

    // 接收数据区域
    mainLayout->addWidget(new QLabel("接收数据:"));
    m_receiveText = new QTextEdit(this);
    m_receiveText->setReadOnly(true);
    mainLayout->addWidget(m_receiveText);

    setCentralWidget(centralWidget);
    resize(600, 800);
    setWindowTitle("QT6.9 蓝牙管理器示例");
  }

  void setupConnections()
  {
    connect(m_scanButton, &QPushButton::clicked,
      [this]() { m_bluetoothManager.startDiscovery(); });

    connect(m_connectButton, &QPushButton::clicked,
      [this]() {
        QListWidgetItem* item = m_devicesList->currentItem();
        if (item) {
          QString text = item->text();
          // 提取地址 (假设格式为 "设备名 (地址)")
          QString address = text.mid(text.lastIndexOf("(") + 1);
          address = address.left(address.lastIndexOf(")"));
          m_bluetoothManager.connectToDevice(address);
        }
      });

    connect(m_disconnectButton, &QPushButton::clicked,
      [this]() { m_bluetoothManager.disconnectFromDevice(); });

    connect(m_clearButton, &QPushButton::clicked,
      [this]() { m_devicesList->clear(); });

    // 蓝牙管理器信号连接
    connect(&m_bluetoothManager, &BluetoothManager::deviceDiscovered,
      this, &MainWindow::addDiscoveredDevice);

    connect(&m_bluetoothManager, &BluetoothManager::deviceConnected,
      this, &MainWindow::onDeviceConnected);

    connect(&m_bluetoothManager, &BluetoothManager::deviceDisconnected,
      this, &MainWindow::onDeviceDisconnected);

    connect(&m_bluetoothManager, &BluetoothManager::connectionFailed,
      this, &MainWindow::onConnectionFailed);

    connect(&m_bluetoothManager, &BluetoothManager::dataReceived,
      this, &MainWindow::onDataReceived);

    connect(&m_bluetoothManager, &BluetoothManager::stateChanged,
      this, &MainWindow::onStateChanged);
  }

private slots:
  void addDiscoveredDevice(const QString& name, const QString& address)
  {
    m_devicesList->addItem(name + " (" + address + ")");
  }

  void onDeviceConnected(const QString& name)
  {
    m_statusLabel->setText("状态: 已连接到 " + name);
    m_connectButton->setEnabled(false);
    m_disconnectButton->setEnabled(true);
  }

  void onDeviceDisconnected()
  {
    m_statusLabel->setText("状态: 未连接");
    m_connectButton->setEnabled(true);
    m_disconnectButton->setEnabled(false);
  }

  void onConnectionFailed(const QString& error)
  {
    QMessageBox::warning(this, "连接失败", error);
    m_statusLabel->setText("状态: 连接失败");
  }

  void onDataReceived(const QByteArray& data)
  {
    m_receiveText->append("接收: " + QString::fromUtf8(data));
  }

  void onStateChanged(const QString& state)
  {
    
    statusBar()->showMessage(state);
  }

private:
  BluetoothManager m_bluetoothManager;
  QListWidget* m_devicesList;
  QTextEdit* m_sendText;
  QTextEdit* m_receiveText;
  QLabel* m_statusLabel;
  QPushButton* m_scanButton;
  QPushButton* m_connectButton;
  QPushButton* m_disconnectButton;
  QPushButton* m_clearButton;
};
