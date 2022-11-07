/** @file simulator.h
 *
 * @brief Simulator declaration
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_SIMULATOR_H
#define ICP_SIMULATOR_H

#include <QMainWindow>
#include <thread>
#include <mutex>
#include "simulator_components/mqtt_widget.h"
#include "simulator_components/mqtt_worker.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Simulator; }
QT_END_NAMESPACE

/**
 * @brief Represents a simulator.
 */
class Simulator : public QMainWindow {
Q_OBJECT

public:

    /**
     * @brief Creates a new MQTT simulator.
     * @param parent Parent widget, defaults to NULL.
     */
    explicit Simulator(QWidget *parent = nullptr);

    /**
     * @brief Destroys the MQTT simulator.
     */
    ~Simulator() override;

private slots:
    /**
     * @brief Closes the simulator window (also stops the running simulation).
     */
    void on_actionClose_triggered();

    /**
     * @brief Loads a config file and parses it.
     *
     * The user is prompted for a file, the file is then read and its format
     * is validated and loaded into internal representation.
     */
    void on_actionLoad_triggered();

    /**
     * @brief Selects an image to use for camera data.
     *
     * The user will be prompted for a .png or .jpg file which will then
     * be used for mocking camera data.
     */
    void on_actionCameraImage_triggered();

    /**
     * @brief Runs the simulation.
     *
     * Asks the user for broker details and runs the simulation based on
     * the current configuration. The simulation is run in a separate thread
     * to avoid blocking the GUI thread.
     */
    void on_actionRun_triggered();

    /**
     * @brief Stops the simulation.
     *
     * The simulation is stopped by locking a mutex in the main thread
     * which will be noticed by the worker thread.
     */
    void on_actionStop_triggered();

    /**
     * @brief Invoked when the client failed to connect.
     *
     * Throws an error to the user.
     */
    void on_didNotConnect();

private:

    /**
     * @brief Stops the simulation.
     *
     * The simulation is stopped by locking a mutex in the main thread
     * which will be noticed by the worker thread.
     */
    void stop();

    Ui::Simulator *ui; /**< Pointer to the UI of the window. */
    MqttWorker *worker = nullptr; /**< Pointer to the worker thread. */
    std::mutex done; /**< Mutex used for signalling simulation stop. */
    std::vector<MqttWidget> widgets; /**< Vector of widgets. */
    std::string cameraImage; /**< Path to image used by camera. */
};

#endif //ICP_SIMULATOR_H
