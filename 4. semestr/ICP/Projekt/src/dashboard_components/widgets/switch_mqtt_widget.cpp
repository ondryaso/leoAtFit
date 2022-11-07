/** @file switch_mqtt_widget.cpp
 *
 * @brief Boolean value presenting MQTT widget with support for state changes definition.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <QPushButton>
#include "switch_mqtt_widget.h"

SwitchMqttWidget::SwitchMqttWidget(const QString &name, const QString &topic, const QString &iconUri, QWidget *parent)
        : MqttWidgetBase(iconUri, name, topic, parent) {
    statusLabel = new QLabel("Off");

    auto largeFont = QFont(statusLabel->font().family(), 20);
    statusLabel->setFont(largeFont);

    auto *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(statusLabel, 0, Qt::AlignCenter);
    layout->addStretch();

    auto *sendOnBtn = new QPushButton("On", this);
    sendOnBtn->setMaximumSize(32, 28);
    connect(sendOnBtn, &QPushButton::clicked, this, &SwitchMqttWidget::sendOnPushed);

    auto *sendOffBtn = new QPushButton("Off", this);
    sendOffBtn->setMaximumSize(32, 28);
    connect(sendOffBtn, &QPushButton::clicked, this, &SwitchMqttWidget::sendOffPushed);

    layout->addWidget(sendOnBtn);
    layout->addWidget(sendOffBtn);

    contentWidget->setLayout(layout);
}

void SwitchMqttWidget::processMessage(mqtt::const_message_ptr message) {
    auto msg = message->to_string();
    bool result = false;

    for (int i = 0; i < 3; i++) {
        auto mode = static_cast<enum Mode>((static_cast<int>(currentMode) + i) % 3);

        switch (mode) {
            case TRUE_FALSE:
                if (tryFind(msg, "true", "false", result)) {
                    // Breaking out of a loop in a nested statement is the one and only legitimate use of goto
                    currentMode = TRUE_FALSE;
                    goto end;
                }
                break;
            case ONE_ZERO:
                if (tryFind(msg, "1", "0", result)) {
                    currentMode = ONE_ZERO;
                    goto end;
                }
                break;
            case YES_NO:
                if (tryFind(msg, "yes", "no", result)) {
                    currentMode = YES_NO;
                    goto end;
                }
                break;
        }
    }

    end:
    statusLabel->setText(result ? "On" : "Off");
}

void SwitchMqttWidget::sendOffPushed(bool checked) {
    emit publishMessage(makeMessage(false));
}

void SwitchMqttWidget::sendOnPushed(bool checked) {
    emit publishMessage(makeMessage(true));
}

mqtt::message_ptr SwitchMqttWidget::makeMessage(bool state) {
    std::string payload;
    switch (currentMode) {
        case TRUE_FALSE:
            payload = state ? "true" : "false";
            break;
        case ONE_ZERO:
            payload = state ? "1" : "0";
            break;
        case YES_NO:
            payload = state ? "yes" : "no";
            break;
    }

    auto msg = mqtt::message_ptr_builder().topic(this->topic.toStdString())
            .payload(payload).qos(0).finalize();

    return msg;
}

bool SwitchMqttWidget::tryFind(const std::string &haystack, const std::string &yes, const std::string &no,
                               bool &result) {
    // Source: https://stackoverflow.com/a/19839371
    // in yes and no arguments, we expect a lowercase string

    auto it = std::search(
            haystack.begin(), haystack.end(),
            yes.begin(), yes.end(),
            [](char ch1, char ch2) { return std::tolower(ch1) == ch2; }
    );
    bool hasYes = (it != haystack.end());

    it = std::search(
            haystack.begin(), haystack.end(),
            no.begin(), no.end(),
            [](char ch1, char ch2) { return std::tolower(ch1) == ch2; }
    );
    bool hasNo = (it != haystack.end());

    result = hasYes || !hasNo;
    return hasYes || hasNo;
}

MqttWidgetType SwitchMqttWidget::getWidgetType() {
    return MqttWidgetType::BOOL;
}
