/** @file mqtt_worker.h
 *
 * @brief MQTT worker declaration
 *
 * This is used as the working/listening thread that manages the running
 * MQTT widgets and periodically asks them to check if they should send.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_MQTT_WORKER_H
#define ICP_MQTT_WORKER_H

#include <QObject>
#include <QThread>
#include <vector>
#include <mutex>
#include "mqtt_widget.h"

/** @brief ID to use for connection to the broker. */
#define SIMULATOR_ID "ICP_Simulator"

/** @brief Timeout in seconds on MQTT connect. */
#define SIMULATOR_TIMEOUT 5

/**
 * @brief An MQTT worker periodically asking widgets to send data.
 */
class MqttWorker: public QThread {
    Q_OBJECT
public:
    /**
     * @brief Creates a new worker.
     * @param widgets Vector of widgets to use in the simulation.
     * @param done Mutex to signal the simulation to stop.
     * @param broker IP of the broker to connect to.
     * @param user Username to use for connection.
     * @param pass Password to use for connection.
     */
    MqttWorker(std::vector<MqttWidget> &widgets, std::mutex &done,
               std::string broker, std::string user, std::string pass);

    /**
     * @brief Runs the worker with.
     *
     * First, creates an async MQTT client that will be used for all sending.
     * Then starts calling trySend for every widget every second.
     *
     */
    void run() Q_DECL_OVERRIDE;

signals:
    /**
     * @brief A signal signalling that the client failed to connect.
     */
    void didNotConnect();

private:
    std::vector<MqttWidget> &widgets; /**< Reference to the widgets. */
    std::mutex &done; /**< Reference to the synchronization mutex. */
    std::string broker; /**< Broker IP. */
    std::string user; /**< Username for the broker. */
    std::string pass; /**< Password for the broker. */
};

#endif //ICP_MQTT_WORKER_H
