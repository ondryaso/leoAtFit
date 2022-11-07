/** @file temp_mqtt_widget.cpp
 *
 * @brief Temperature presenting MQTT widget definition.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */


#include "temp_mqtt_widget.h"

TempMqttWidget::TempMqttWidget(const QString &name, const QString &topic, bool preferC, const QString &iconUri,
                               QWidget *parent)
        : MqttWidgetBase(iconUri, name, topic, parent) {
    textLabel = new QLabel("waiting for message");
    smallFont = QFont(textLabel->font().family(), textLabel->font().pointSize());
    largeFont = QFont(textLabel->font().family(), 20);

    preferCelsius = preferC;

    auto *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(textLabel, 0, Qt::AlignCenter);
    contentWidget->setLayout(layout);
}

void TempMqttWidget::processMessage(mqtt::const_message_ptr message) {
    auto msg = message->to_string();
    double value;

    const auto oldLocale = std::setlocale(LC_NUMERIC, nullptr);

    try {
        std::setlocale(LC_NUMERIC, "C");
        value = std::stod(msg);
    } catch (...) {
        if (msg.empty()) {
            textLabel->setText("empty");
        } else {
            textLabel->setText(QString("cannot parse data:\n").append(QString::fromStdString(msg)));
        }
        textLabel->setFont(smallFont);
        std::setlocale(LC_NUMERIC, oldLocale);
        return;
    }

    std::setlocale(LC_NUMERIC, oldLocale);
    bool useC = (msg.find('C') != std::string::npos)
                || ((msg.find('F') == std::string::npos) && preferCelsius);
    if (useC) {
        textLabel->setText(QString("%1 °C").arg(value, 0, 'f', 2));
    } else {
        textLabel->setText(QString("%1 °F").arg(value, 0, 'f', 2));
    }
    textLabel->setFont(largeFont);
}

MqttWidgetType TempMqttWidget::getWidgetType() {
    return preferCelsius ? MqttWidgetType::TEMP_FLOAT : MqttWidgetType::TEMP_FLOAT_F;
}