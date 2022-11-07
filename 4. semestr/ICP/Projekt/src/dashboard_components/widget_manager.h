/** @file widget_manager.h
 *
 * @brief Dashboard MQTT widget manager definition.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_WIDGET_MANAGER_H
#define ICP_WIDGET_MANAGER_H

#include <QObject>
#include <QWidget>
#include "mqtt/message.h"
#include "mqtt_client.h"
#include "widgets/mqtt_widget_base.h"
#include "dashboard_add_topic.h"

/**
 * @brief Dashboard controller. Manages MQTT widgets and routes messages to them.
 *
 * An instance of this class is primarily responsible for creating and removing "MQTT widgets".
 *
 * This class can be perceived as a controller to a Dashboard window (view). It handles the Dashboard's logic
 * that isn't strictly UI-related.
 *
 * @see MqttWidgetBase
 */
class MqttWidgetManager : public QObject {
Q_OBJECT

public:
    explicit MqttWidgetManager(QObject *parent = nullptr);

    ~MqttWidgetManager() override;

    /**
     * @brief Shows an "Add Topic" dialog.
     *
     * If the dialog is confirmed by the user, the makeWidget() slot is called.
     * It then creates a new MQTT widget based on the user's selection and emits the widgetAdded() signal.
     */
    void showAddWidgetDialog();

    /**
     * @return A pointer to this Dashboard's MQTT client instance.
     */
    DashboardMqttClient *getClient();

    void saveConfig(QString const &path);

    void loadConfig(QString const &path);

signals:

    /**
     * Emitted when an MQTT widget should be added (to the Dashboard's grid).
     * @param widget A pointer to the newly created widget.
     */
    void widgetAdded(QWidget *widget);

    /**
     * Emitted when an MQTT widget should be removed (from the Dashboard's grid).
     * @param widget A pointer to the widget that should be removed.
     */
    void widgetRemoved(QWidget *widget);

public slots:

    /**
     * Signalised by a widget that asks for removal.
     */
    void widgetRequiresRemoval();

    /**
     * Signalised by a widget that asks to publish a MQTT message.
     * @param message A pointer to the message to publish.
     */
    void widgetRequiresMessageSend(mqtt::message_ptr message);

private:
    DashboardMqttClient *client; /**< A client used for handling the MQTT connection. */
    std::map<mqtt::string, std::vector<MqttWidgetBase *>> targets; /**< A map of MQTT topics and their corresponding widgets. */
    std::vector<MqttWidgetBase *> widgets; /**< A linear list of widgets. */

private slots:

    /**
     * Signalised by the DashboardMqttClient when a message is received.
     * Sends the message to the corresponding MQTT widget(s).
     * @param messagePtr A pointer to the received message.
     */
    void messageReceived(mqtt::const_message_ptr messagePtr);

    /**
     * Signalised by the DashboardMqttClient when a message has been sent successfully.
     * @param ptr A pointer to the sent message.
     */
    void messageSent(mqtt::const_message_ptr ptr);

    /**
     * Signalised by the DashboardMqttClient when a subscription has been set up.
     * @param topic A string with the topic that the client is now subscribed to.
     */
    void subscribed(QString const &topic);

    /**
      * Signalised by the DashboardMqttClient when a subscription has been terminated.
      * @param topic A string with the topic that the client is now unsubscribed from.
      */
    void unsubscribed(QString const &topic);

    /**
     * Signalised by an "Add Topic" dialog when user confirms topic addition.
     * @param widgetType An integer corresponding to an MqttWidgetType enum value.
     * @param topic The topic to subscribe the created widget to.
     */
    void makeWidget(int widgetType, QString const &topic, QString const &name);

    /**
     * Signalised by the DashboardMqttClient when it disconnects.
     * @param forcefully Signalises whether the connection was closed on the
     * user's request (false) or because of an error (true).
     */
    void clientDisconnected(bool forcefully);
};


#endif //ICP_WIDGET_MANAGER_H
