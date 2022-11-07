/** @file mqtt_worker.cpp
 *
 * @brief MQTT worker implementation
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <mqtt/async_client.h>
#include <chrono>
#include "mqtt_worker.h"

MqttWorker::MqttWorker(std::vector<MqttWidget> &widgets, std::mutex &done, std::string broker,
                       std::string user, std::string pass):
                       widgets(widgets), done(done), broker(broker), user(user), pass(pass) {}

void MqttWorker::run() {
    mqtt::async_client client(broker, SIMULATOR_ID);
    mqtt::connect_options opts;
    opts.set_user_name(user);
    opts.set_password(pass);
    opts.set_clean_session(true);
    opts.set_connect_timeout(std::chrono::seconds(SIMULATOR_TIMEOUT));
    try {
        client.connect(opts)->wait();
    } catch (...) {
        emit didNotConnect();
        return;
    }
    while (true) {
        if (!done.try_lock()) {
            break;
        }
        done.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (auto &widget: widgets) {
            widget.trySend(client);
        }
    }
    client.disconnect();
}