/** @file explorer.h
 *
 * @brief Explorer declaration
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_EXPLORER_H
#define ICP_EXPLORER_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <mqtt/async_client.h>
#include "explorer_components/mqtt_tree_model.h"

/** @brief The ID to use for MQTT client. */
#define CLIENT_ID "ICP_Explorer"

/** @brief Seconds before connection times out. */
#define CLIENT_TIMEOUT 2

QT_BEGIN_NAMESPACE
namespace Ui { class Explorer; }
QT_END_NAMESPACE

/**
 * @brief Represents a MQTT Explorer.
 */
class Explorer : public QMainWindow {
Q_OBJECT

public:
    /**
     * @brief Creates a new MQTT explorer.
     * @param parent Parent widget, defaults to NULL.
     */
    explicit Explorer(QWidget *parent = nullptr);

    /**
     * @brief Destroys the explorer.
     */
    ~Explorer() override;

private slots:
    /**
     * @brief Closes the window.
     */
    void on_actionClose_triggered();

    /**
     * @brief Saves a snapshot of the current state.
     *
     * Requests a directory from a user to save the state into and
     * then recreates the tree directory structure along with payloads.
     */
    void on_actionSave_triggered();

    /**
     * @brief Start listening to the MQTT broker.
     *
     * First creates a dialog requesting broker IP and other parameters for
     * the communication - topic and message history limit. Then clears the
     * current state and starts listening to the MQTT broker.
     */
    void on_actionRun_triggered();

    /**
     * @brief Adds a new topic to the tree.
     */
    void on_actionAdd_triggered();

    /**
     * @brief Sends a message on the currently selected topic
     */
     void on_actionNewMessage_triggered();

     /**
      * @brief Changes a topic to listen to.
      */
     void on_actionChangeTopic_triggered();

    /**
     * @brief Handles a state when MQTT client could not connect.
     */
    void handleNoConnection();

    /**
     * @brief Updates the right side of the explorer after topic selection.
     */
    void updateRightSide();

    /**
     * @brief Shows the full message on click.
     */
    void showMessage();


private:
    /**
     * @brief Clears the right side of the window.
     */
    void clearRightSide();

    Ui::Explorer *ui; /**< Pointer to explorer UI. */
    std::string broker; /**< Broker IP currently in use. */
    std::string topic; /**< Topic that is currently filtered by. */
    unsigned messages; /**< The number of messages stored for each topic. */
    MqttTreeModel *model = nullptr; /**< The current model in use. */
    mqtt::async_client *client = nullptr; /**< The current MQTT client in use. */
    QWidget *messageContainer = nullptr; /**< Inner container of the scroll area on the right side. */
    QVBoxLayout *messageLayout = nullptr; /**< The inner layout of the widget encapsulating messages. */
};

#endif //ICP_EXPLORER_H
