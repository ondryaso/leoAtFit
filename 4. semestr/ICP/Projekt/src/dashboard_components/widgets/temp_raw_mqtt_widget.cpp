/** @file temp_raw_mqtt_widget.cpp
 *
 * @brief Temperature (raw) presenting MQTT widget definition.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include "temp_raw_mqtt_widget.h"

RawTempMqttWidget::RawTempMqttWidget(const QString &name, const QString &topic, QWidget *parent) :
        TextMqttWidget(name, topic, false, ":/widgets/temperature.png", parent) {
    textLabel->setFont(QFont(textLabel->font().family(), 20));
    textLabel->setText("–");
}

MqttWidgetType RawTempMqttWidget::getWidgetType() {
    return MqttWidgetType::TEMP_TEXT;
}
