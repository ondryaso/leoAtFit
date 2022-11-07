/** @file temp_mqtt_widget.h
 *
 * @brief Temperature presenting MQTT widget declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_TEMP_MQTT_WIDGET_H
#define ICP_TEMP_MQTT_WIDGET_H


#include "mqtt_widget_base.h"

/**
 * @brief Represents an MQTT widget that shows temperature.
 *
 * When a message comes, it attempts to find a double value.
 * If the message contains a "C" or "F", the corresponding
 * unit is shown; the constructor parameter preferC is used to control the shown unit if none is
 * found in the message.
 */
class TempMqttWidget : public MqttWidgetBase {
public:
    TempMqttWidget(const QString &name, const QString &topic, bool preferC = true,
                   const QString &iconUri = ":/widgets/temperature.png",
                   QWidget *parent = nullptr);

    MqttWidgetType getWidgetType() override;

public slots:

    void processMessage(mqtt::const_message_ptr message) override;

private:
    QLabel *textLabel;
    QFont largeFont;
    QFont smallFont;
    bool preferCelsius;

};


#endif //ICP_TEMP_MQTT_WIDGET_H
