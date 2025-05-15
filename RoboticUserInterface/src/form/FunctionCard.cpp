#include "robotic_user_interface/form/FunctionCard.h"

FunctionCard::FunctionCard(QWidget* parent /*= nullptr*/) : QWidget(parent)
{
    setFixedSize(360, 90);

    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    auto rightLayout = new QVBoxLayout();

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(50, 50);
    QPixmap pixmap = QPixmap("../res/ControlImages/Placeholder.png");
    pixmap = pixmap.scaled(50, 50, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_iconLabel->setPixmap(pixmap);

    m_titleLabel = new QLabel;
    m_titleLabel->setText("AnimatedIcon");

    m_contextLabel = new QLabel;
    m_contextLabel->setText("An element that displays and controls an icon that animates when the user interacts with the controls.");

    m_titleLabel->setWordWrap(true);
    m_contextLabel->setWordWrap(true);

    m_iconLabel->setObjectName("iconLabel");
    m_titleLabel->setObjectName("titleLabel");
    m_contextLabel->setObjectName("contextLabel");

    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_iconLabel);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(rightLayout);
    rightLayout->addWidget(m_titleLabel);
    rightLayout->addWidget(m_contextLabel);
}

FunctionCard::FunctionCard(const QIcon& icon, const QString& titleText, const QString& contextText, QWidget* parent /*= nullptr*/) : FunctionCard(parent)
{
    QPixmap pixmap = icon.pixmap(50, 50);
    m_iconLabel->setPixmap(pixmap);
    m_titleLabel->setText(titleText);
    m_contextLabel->setText(contextText);

    this->setStyleSheet(
      "FunctionCard {"
      "background-color: rgba(50, 50, 50, 0.95);"
      "border-radius: 8px;"
      "border: 0px;"
      "}"

      "FunctionCard:hover {"
      "background-color: rgba(62, 62, 62, 0.95);"
      "border: 1px solid rgb(35, 35, 35);"
      "}"

      "#titleLabel {"
      "font-size: 15px;"
      "color: rgb(225, 225, 225);"
      "}"

      "#contextLabel {"
      "font-size: 11px;"
      "color: rgb(185, 185, 185);"
      "}"
    );

}

void FunctionCard::setKey(QString key)
{
    m_key = key;
}

QString FunctionCard::getKey()
{
    return m_key;
}

void FunctionCard::mouseReleaseEvent(QMouseEvent* event)
{
    emit clicked(m_key);
}

void FunctionCard::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}