/** @file mqtt_client.cpp
 *
 * @brief Dashboard MQTT client definition.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include "mqtt_client.h"

DashboardMqttClient::DashboardMqttClient(QObject *parent) : QObject(parent), client(nullptr) {
}

bool DashboardMqttClient::isConnected() {
    return client != nullptr && client->is_connected();
}

void DashboardMqttClient::connect(QString const &address, QString const &userName, QString const &password) {
    if (isConnected()) return; // We are already connected
    delete client; // If the client exists but it isn't connected, delete it first

    auto clientCreateOptions = mqtt::create_options_builder()
            .send_while_disconnected(true, false)
            .delete_oldest_messages(true)
            .max_buffered_messages(MAX_BUFFERED_MESSAGES)
            .mqtt_version(MQTT_VERSION)
            .restore_messages(false)
            .finalize();

    auto connectOptions = mqtt::connect_options_builder()
            .mqtt_version(MQTT_VERSION)
            .automatic_reconnect(false)
            .clean_session(true)
            .connect_timeout(std::chrono::milliseconds(CONNECT_TIMEOUT))
            .user_name(userName.toStdString())
            .password(password.toStdString())
            .finalize();

    auto time = std::chrono::system_clock::now().time_since_epoch().count();
    std::string clientId = "ICP_MQTT_Dashboard" + std::to_string(time);
    currentRemoteAddress = address;

    try {
        client = new mqtt::async_client(address.toStdString(), clientId, clientCreateOptions);
        client->set_callback(*this);
        client->connect(connectOptions, nullptr, *this);
    } catch (const mqtt::exception &e) {
        emit connectError(QString::fromStdString(e.to_string()));
        delete client;
    }
}

DashboardMqttClient::~DashboardMqttClient() {
    if (client != nullptr) {
        if (client->is_connected()) {
            client->stop_consuming();
            client->disable_callbacks();
            client->disconnect(DISCONNECT_TIMEOUT)->wait();
        }

        delete client;
    }
}

void DashboardMqttClient::connected(const mqtt::string &string) {
    // Handled by on_success
}

void DashboardMqttClient::connection_lost(const mqtt::string &string) {
    emit disconnected(true);
}

void DashboardMqttClient::message_arrived(mqtt::const_message_ptr ptr) {
    emit messageReceived(ptr);
}

void DashboardMqttClient::delivery_complete(mqtt::delivery_token_ptr ptr) {
    switch (ptr->get_type()) {
        case mqtt::token::PUBLISH:
            emit messageSent(ptr->get_message());
            break;
        case mqtt::token::SUBSCRIBE:
            emit subscribed("TODO"); // TODO
            break;
        case mqtt::token::UNSUBSCRIBE:
            emit unsubscribed("TODO"); // TODO
            break;
    }
}

void DashboardMqttClient::on_failure(const mqtt::token &asyncActionToken) {
    if (asyncActionToken.get_type() == mqtt::token::CONNECT) {
        emit connectError("Server unavailable");
        client->stop_consuming();
        client->disable_callbacks();
    }

    if (asyncActionToken.get_type() == mqtt::token::DISCONNECT) {
        emit disconnected(false);
        std::cerr << "Error when disconnecting (code " << asyncActionToken.get_reason_code() << ")\n";
    }
}

void DashboardMqttClient::on_success(const mqtt::token &asyncActionToken) {
    if (asyncActionToken.get_type() == mqtt::token::CONNECT) {
        emit connectSuccess();

        client->subscribe("#", 0);
    }

    if (asyncActionToken.get_type() == mqtt::token::DISCONNECT) {
        emit disconnected(false);
    }
}

const QString &DashboardMqttClient::getCurrentRemoteAddress() const {
    return currentRemoteAddress;
}

void DashboardMqttClient::disconnect() {
    if (!this->isConnected()) return;

    client->stop_consuming();

    try {
        client->disconnect(DISCONNECT_TIMEOUT, nullptr, *this);
    } catch (const mqtt::exception &e) {
        std::cerr << "Error when disconnecting: " << e << "\n";
        emit disconnected(false);
    }
}

void DashboardMqttClient::subscribe(const QString &topic) {
    if (!this->isConnected()) return;
    client->subscribe(topic.toStdString(), 0);
}

void DashboardMqttClient::unsubscribe(const QString &topic) {
    if (!this->isConnected()) return;
    client->unsubscribe(topic.toStdString());
}

void DashboardMqttClient::publishMessage(const mqtt::message_ptr &message) {
    if (!this->isConnected()) return;
    client->publish(message);
}
