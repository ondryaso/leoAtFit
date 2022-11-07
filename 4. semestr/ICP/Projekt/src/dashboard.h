/** @file dashboard.h
 *
 * @brief Dashboard declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_DASHBOARD_H
#define ICP_DASHBOARD_H

#include <QMainWindow>
#include <QLabel>
#include <QGridLayout>
#include <QScrollArea>
#include "dashboard_components/mqtt_client.h"
#include "dashboard_components/widget_manager.h"
#include "dashboard_components/flow_layout.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dashboard; }
QT_END_NAMESPACE

/**
 * @brief Represents an MQTT Dashboard window.
 */
class Dashboard : public QMainWindow {
Q_OBJECT

public:
    /**
     * @brief Creates a new MQTT dashboard window.
     * @param parent Parent widget, defaults to nullptr.
     */
    explicit Dashboard(QWidget *parent = nullptr);

    /**
     * @brief Destroys the dashboard window.
     */
    ~Dashboard() override;

private slots:

    /* ---- Action Handlers ---- */
    /**
     * @brief Opens a connection dialog.
     */
    void on_actionConnect_triggered(bool checked);

    /**
     * @brief Closes the current connection.
     */
    void on_actionDisconnect_triggered(bool checked);

    /**
     * @brief Opens an "Add Topic" dialog.
     */
    void on_actionAddTopic_triggered(bool checked);

    /**
     * @brief Opens a "Load Config" dialog.
     */
    void on_actionLoadConfig_triggered(bool checked);

    /**
     * @brief Opens a "Save Config" dialog or saves directly to the last opened file.
     */
    void on_actionSave_triggered(bool checked);

    /**
      * @brief Opens a "Save Config" dialog.
      */
    void on_actionSaveAs_triggered(bool checked);

    /* ---- MQTT Client event handlers ---- */

    /**
     * @brief Signalised when the client connects to an MQTT server.
     */
    void clientConnected();

    /**
     * @brief Signalised when an error occurrs while connecting.
     * @param cause A string with an optional error description.
     */
    void clientConnectionError(const QString &cause);

    /**
     * @brief Signalised when the client is disconnected.
     * @param forcefully Signalises whether the connection was closed on the
     * user's request (false) or because of an error (true).
     */
    void clientDisconnected(bool forcefully);

    /* ---- Dialog event handlers ---- */
    /**
     * @brief Called by a connection dialog when the "Connect" button is clicked.
     * @param address The IP address or hostname received from the user.
     * @param userName The user name to identify with.
     * @param password The password to identify with.
     */
    void connectAddressConfirmed(const QString &address, const QString &userName, const QString &password);

    /* ---- MQTT Widget manager event handlers ---- */

    /**
     * @brief Signalised when an MQTT widget should be added to the grid.
     * @param widget A pointer to the new widget.
     */
    void widgetAdded(QWidget *widget);

    /**
     * @brief Signalised when an MQTT widget should be removed from the grid.
     * @param widget A pointer to the widget to remove.
     */
    void widgetRemoved(QWidget *widget);

private:
    Ui::Dashboard *ui; /**< Pointer to explorer UI. */
    QLabel *statusBarLabel; /**< Pointer to a label that shows a permanent message in the statusbar. */
    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QString lastFile;
    FlowLayout *contentLayout;
    QMetaObject::Connection currentIndicatorConnection; /**< A connection between the indicator button and an action. */
    MqttWidgetManager widgetManager; /**< A manager object responsible for handling the data blocks in the grid. */
    int widgetCount;

    /* ---- Functions for managing the current layout of this window ---- */

    /**
     * @brief Used to change the Dashboard to the "disconnected" layout.
     */
    void useDisconnectedLayout();

    /**
     * @brief Used to change the Dashboard to the "connected but no widgets added" layout.
     */
    void useEmptyConnectedLayout();

    /**
      * @brief Used to change the Dashboard to the "connected" layout.
      */
    void useFilledConnectedLayout();

};

#endif //ICP_DASHBOARD_H
