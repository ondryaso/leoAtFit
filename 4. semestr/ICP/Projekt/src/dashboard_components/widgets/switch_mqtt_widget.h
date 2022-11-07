/** @file switch_mqtt_widget.h
 *
 * @brief Boolean value presenting MQTT widget with support for state changes declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_SWITCH_MQTT_WIDGET_H
#define ICP_SWITCH_MQTT_WIDGET_H

#include "mqtt_widget_base.h"

/**
 * @brief Represents an MQTT widget that shows or publishes an "on/off" state – a boolean value.
 *
 * It supports three means of representing a boolean value: true/false, yes/no and 1/0.
 * It also allows the user to send a "yes" or a "no" back to the server
 * (in the same representation as the last received message's one).
 */
class SwitchMqttWidget : public MqttWidgetBase {
public:
    SwitchMqttWidget(const QString &name, const QString &topic,
                     const QString &iconUri = ":/widgets/power.png",
                     QWidget *parent = nullptr);
    MqttWidgetType getWidgetType() override;

public slots:

    void processMessage(mqtt::const_message_ptr message) override;

private:
    enum Mode {
        TRUE_FALSE,
        ONE_ZERO,
        YES_NO
    } currentMode = TRUE_FALSE;

    QLabel *statusLabel;

    mqtt::message_ptr makeMessage(bool state);

    bool tryFind(std::string const &haystack, std::string const &yes, std::string const &no, bool &result);

private slots:

    void sendOnPushed(bool checked);

    void sendOffPushed(bool checked);

};


#endif //ICP_SWITCH_MQTT_WIDGET_H
