/** @file mqtt_client.h
 *
 * @brief Dashboard MQTT client declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_MQTT_CLIENT_H
#define ICP_MQTT_CLIENT_H

#include <QObject>
#include <QString>
#include "mqtt/async_client.h"

/**
 * @brief Represents an MQTT client used by a Dashboard window.
 *
 * This class is a "data-access" layer to the Dashboard component.
 * It encapsulates an mqtt::async_client and provides a Qt-compatible interface to its primary functionality.
 * Its instances are managed by the Dashboard's controller, MqttWidgetManager.
 */
class DashboardMqttClient : public QObject, public virtual mqtt::callback, public virtual mqtt::iaction_listener {
Q_OBJECT

public:
    const int MAX_BUFFERED_MESSAGES = 16;
    const int MQTT_VERSION = MQTTVERSION_3_1_1;
    const int CONNECT_TIMEOUT = 5000;
    const int DISCONNECT_TIMEOUT = 1000;

    explicit DashboardMqttClient(QObject *parent = nullptr);

    ~DashboardMqttClient() override;

    /**
     * Determines if this client is currently connected to a server.
     * @return True if connected, false otherwise.
     */
    bool isConnected();

    /**
     * Returns the address that has been used during the last connection attempt.
     * @return A server address, specified as a URI.
     */
    const QString &getCurrentRemoteAddress() const;

    /**
     * Begins a connection attempt to the specified server; using the specified user name and password to authenticate.
     * @param address The address of the server to connect to, specified as a URI.
     * @param userName The user name to use to authenticate with the server. May be empty.
     * @param password The password to use to authenticate with the server. May be empty.
     */
    void connect(QString const &address, QString const &userName, QString const &password);

    /**
     * Synchronously disconnects from the server.
     * Waits DISCONNECT_TIMEOUT milliseconds at most.
     * If this client is not connected, returns immediately.
     */
    void disconnect();

    /**
     * Subscribes to the specified topic.
     * If this client is not connected, returns immediately.
     * @param topic The topic to subscribe to.
     */
    void subscribe(QString const &topic);

    /**
     * Unsubscribes from the specified topic.
     * If this client is not connected, returns immediately.
     * @param topic The topic to unsubscribe from.
     */
    void unsubscribe(QString const &topic);

    /**
     * Publishes the specified message.
     * @param message A pointer to the message to publish.
     */
    void publishMessage(const mqtt::message_ptr &message);


signals:

    /**
     * Emitted when the client successfully connects to the MQTT server.
     * This happens in reaction to a connect() call.
     */
    void connectSuccess();

    /**
     * Emitted when a connection attempt fails.
     * This happens in reaction to a connect() call.
     * @param cause An error message.
     */
    void connectError(QString const &cause);

    /**
     * Emitted when the client is disconnected.
     * @param forcefully Signalises whether the connection was closed on the
     * user's request (false) or because of an error (true).
     */
    void disconnected(bool forcefully);

    /**
     * Emitted when a message is received.
     * This can happen anytime.
     * @param ptr Pointer to an object holding metadata and the contents of the message.
     */
    void messageReceived(mqtt::const_message_ptr ptr);

    /**
     * Emitted when a message is sent.
     * This happens in reaction to a publishMessage() call.
     * @param ptr Pointer to an object holding metadata of the message.
     */
    void messageSent(mqtt::const_message_ptr ptr);

    /**
     * Emitted when a subscription is set up.
     * This happens in reaction to a subscribe() call.
     * @param topic A string with the topic that the client is now subscribed to.
     */
    void subscribed(QString const &topic);

    /**
     * Emitted when a subscription is terminated.
     * This happens in reaction to an unsubscribe() call.
     * @param topic A string with the topic that the client is now subscribed to.
     */
    void unsubscribed(QString const &topic);

private:
    mqtt::async_client *client;
    QString currentRemoteAddress;

    void connected(const mqtt::string &string) override;

    void connection_lost(const mqtt::string &string) override;

    void message_arrived(mqtt::const_message_ptr ptr) override;

    void delivery_complete(mqtt::delivery_token_ptr ptr) override;

    void on_failure(const mqtt::token &asyncActionToken) override;

    void on_success(const mqtt::token &asyncActionToken) override;
};


#endif //ICP_MQTT_CLIENT_H
