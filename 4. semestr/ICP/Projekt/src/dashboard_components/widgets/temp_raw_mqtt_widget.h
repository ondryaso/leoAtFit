/** @file temp_raw_mqtt_widget.h
 *
 * @brief Temperature (raw) presenting MQTT widget declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */


#ifndef ICP_TEMP_RAW_MQTT_WIDGET_H
#define ICP_TEMP_RAW_MQTT_WIDGET_H


#include "text_mqtt_widget.h"

/**
 * @brief Represents an MQTT widget that shows temperature in the exact same form as received.
 *
 * It's noting else but a TextMqttWidget with a temperature icon and bigger font.
 */
class RawTempMqttWidget : public TextMqttWidget {
public:
    RawTempMqttWidget(const QString &name, const QString &topic, QWidget *parent = nullptr);

    MqttWidgetType getWidgetType() override;
};


#endif //ICP_TEMP_RAW_MQTT_WIDGET_H
