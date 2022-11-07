/** @file mqtt_widget_base.h
 *
 * @brief MQTT widget base class declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_MQTT_WIDGET_BASE_H
#define ICP_MQTT_WIDGET_BASE_H

#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <mqtt/message.h>
#include "../widget_type.h"

/**
 * @brief Represents the base of an MQTT widget.
 *
 * An MQTT widget is a single block of the dashboard grid
 * that is bound to a single MQTT topic and presents messages received from this topic to the user;
 * and optionally also allows the user to publish some kind of data to the topic.
 *
 * This base constructs a widget with an icon, a title and a topic in its top half.
 * Subclasses then implement the incoming message processing and presenting logic.
 */
class MqttWidgetBase : public QWidget {
Q_OBJECT
protected:
    explicit MqttWidgetBase(QString const &iconUri, QString const &name, QString topic,
                            QWidget *parent = nullptr);

    QLabel *nameLabel; /**< Pointer to the label with the user-defined name for this widget. */
    QLabel *topicLabel; /**< Pointer to the label with the topic this widget consumes. */
    QWidget *contentWidget; /**< Pointer to a generic QWidget that is populated by the presented data. */
    const QString topic; /**< A QString with the topic this widget consumes. */
    const QString name; /**< A QString with this widget's user-defined name. */
    void paintEvent(QPaintEvent *event) override;

signals:

    /**
     * Emitted when the remove button is clicked.
     */
    void requestRemoval();

    /**
     * Emitted when this widget requires publishing an MQTT message.
     * @param message A pointer to the message to publish.
     */
    void publishMessage(mqtt::message_ptr message);

public:
    virtual MqttWidgetType getWidgetType() = 0;

    const QString &getTopic() const;

    const QString &getName() const;

public slots:

    /**
     * Signalised by the widget manager when a message is received with this widget's topic.
     * @param message
     */
    virtual void processMessage(mqtt::const_message_ptr message) = 0;
};


#endif //ICP_MQTT_WIDGET_BASE_H
