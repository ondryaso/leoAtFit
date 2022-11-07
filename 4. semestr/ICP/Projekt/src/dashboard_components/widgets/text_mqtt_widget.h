/** @file text_mqtt_widget.h
 *
 * @brief Raw text presenting MQTT widget declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_TEXT_MQTT_WIDGET_H
#define ICP_TEXT_MQTT_WIDGET_H

#include <QPlainTextEdit>
#include "mqtt_widget_base.h"

/**
 * @brief Represents an MQTT widget that shows the received text.
 *
 * If a "multi-line" mode is enabled,
 * a button is presented to open a dialog with the full body of the message. Otherwise, only
 * the first line of the message, cropped to 32 characters, is shown.
 */
class TextMqttWidget : public MqttWidgetBase {
public:
    TextMqttWidget(const QString &name, const QString &topic, bool multiLine = false,
                   const QString &iconUri = ":/widgets/text.png",
                   QWidget *parent = nullptr);

    MqttWidgetType getWidgetType() override;
public slots:

    void processMessage(mqtt::const_message_ptr message) override;

protected:
    QLabel *textLabel;
    QString lastMessage;
    QDialog *dialog;
    QPlainTextEdit *dialogTextEdit;
    bool multiLine;

signals:

    void showMoreClicked(bool checked);

};


#endif //ICP_TEXT_MQTT_WIDGET_H
