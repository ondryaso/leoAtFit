/** @file mqtt_widget_base.cpp
 *
 * @brief MQTT widget base class definition.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <QLabel>
#include <QPainter>
#include "mqtt_widget_base.h"
#include <iostream>
#include <utility>
#include <QPushButton>

MqttWidgetBase::MqttWidgetBase(QString const &iconUri, QString const &name, QString topicA, QWidget *parent)
        : QWidget(parent), topic(std::move(topicA)), name(name) {
    auto *main = new QVBoxLayout();

    auto *top = new QHBoxLayout();
    auto *topLabelsWidget = new QWidget();
    auto *topLabels = new QVBoxLayout();

    // Top left: icon
    auto *imgLabel = new QLabel();
    auto img = QPixmap(iconUri);
    imgLabel->setPixmap(img.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    // Top right: labels
    nameLabel = new QLabel(name);
    nameLabel->setFont(QFont(nameLabel->font().family(), 16));
    topLabels->addWidget(nameLabel);
    topicLabel = new QLabel(topic);
    topLabels->addWidget(topicLabel);
    topLabels->setSpacing(0);
    topicLabel->setFont(QFont(topicLabel->font().family(), 8));
    topLabelsWidget->setLayout(topLabels);

    // Top right: remove button
    auto *removeBtn = new QPushButton("×", this);
    removeBtn->setMaximumSize(28, 28);
    connect(removeBtn, &QPushButton::clicked, this, &MqttWidgetBase::requestRemoval);

    // Add to top
    top->addWidget(imgLabel);
    top->addWidget(topLabelsWidget);
    top->addStretch();
    top->addWidget(removeBtn);

    // Window
    main->setContentsMargins(10, 0, 10, 0);
    main->addLayout(top);

    contentWidget = new QWidget(this);
    main->addWidget(contentWidget);

    this->setFixedSize(280, 120);
    this->setLayout(main);
}

void MqttWidgetBase::paintEvent(QPaintEvent *event) {
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawRoundedRect(rect(), 3, 3);
}

const QString &MqttWidgetBase::getTopic() const {
    return topic;
}

const QString &MqttWidgetBase::getName() const {
    return name;
}
