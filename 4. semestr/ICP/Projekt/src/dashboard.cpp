/** @file dashboard.cpp
 *
 * @brief Dashboard implementation.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <QFileDialog>
#include "dashboard.h"
#include "ui_dashboard.h"
#include "dashboard_components/dashboard_connect.h"
#include "dashboard_components/widgets/mqtt_widget_base.h"
#include <iostream>
#include <QTime>
#include <QMessageBox>

Dashboard::Dashboard(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::Dashboard), widgetCount(0), widgetManager(this) {
    ui->setupUi(this);

    // Configure the permanent status indicator in status bar
    statusBarLabel = new QLabel();
    statusBarLabel->setContentsMargins(0, 0, 5, 0);
    ui->statusBar->addPermanentWidget(statusBarLabel);
    ui->statusBar->setSizeGripEnabled(false);
    resize(890, 432);
    setMinimumSize(size());

    // The "filled with at least one topic" layout will be created dynamically later
    scrollArea = nullptr;

    // Connect the widget manager
    connect(&widgetManager, &MqttWidgetManager::widgetAdded, this, &Dashboard::widgetAdded);
    connect(&widgetManager, &MqttWidgetManager::widgetRemoved, this, &Dashboard::widgetRemoved);

    auto client = widgetManager.getClient();

    // Connect the MQTT client with the corresponding events that handle layout changes of Dashboard
    connect(client, &DashboardMqttClient::connectSuccess, this, &Dashboard::clientConnected);
    connect(client, &DashboardMqttClient::connectError, this, &Dashboard::clientConnectionError);
    connect(client, &DashboardMqttClient::disconnected, this, &Dashboard::clientDisconnected);

    // Use the "Disconnected" layout
    useDisconnectedLayout();
}

Dashboard::~Dashboard() {
    delete ui;
}

/* ---- Action Handlers ---- */
void Dashboard::on_actionConnect_triggered(bool checked) {
    auto connectDialog = new DashboardConnect;
    auto con = connect(connectDialog, &DashboardConnect::dialogConfirmed, this, &Dashboard::connectAddressConfirmed);
    ui->actionConnect->setEnabled(false);
    ui->statusIndicator->setEnabled(false);
    connectDialog->exec();
    ui->actionConnect->setEnabled(true);
    ui->statusIndicator->setEnabled(true);
    delete connectDialog;
}

void Dashboard::on_actionDisconnect_triggered(bool checked) {
    if (widgetManager.getClient()->isConnected()) {

        auto result = QMessageBox::question(this, "Confirmation", "Do you really want to disconnect?",
                                            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (result == QMessageBox::Yes) {
            widgetManager.getClient()->disconnect();
        }
    }
}

void Dashboard::on_actionAddTopic_triggered(bool checked) {
    widgetManager.showAddWidgetDialog();
}

void Dashboard::on_actionLoadConfig_triggered(bool checked) {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Dashboard configuration", QString(),
                                                    "Dashboard Configuration (*.dashconf);;All files (*.*)");
    if (fileName.isEmpty()) return;

    try {
        widgetManager.loadConfig(fileName);
        lastFile = fileName;
    } catch (const std::runtime_error &e) {
        QMessageBox::warning(this, "Warning", QString("Cannot open file: ").append(e.what()));
    }
}

void Dashboard::on_actionSave_triggered(bool checked) {
    if (lastFile.isEmpty()) {
        lastFile = QFileDialog::getSaveFileName(this, "Save Dashboard configuration", QString(),
                                                "Dashboard Configuration (*.dashconf);;All files (*.*)");
    }

    if (lastFile.isEmpty()) return;

    try {
        widgetManager.saveConfig(lastFile);
    } catch (const std::runtime_error &e) {
        QMessageBox::warning(this, "Warning", QString("Cannot save configuration: ").append(e.what()));
    }
}

void Dashboard::on_actionSaveAs_triggered(bool checked) {
    auto file = QFileDialog::getSaveFileName(this, "Save Dashboard configuration", QString(),
                                             "Dashboard Configuration (*.dashconf);;All files (*.*)");
    if (file.isEmpty()) return;

    try {
        widgetManager.saveConfig(file);
        lastFile = file;
    } catch (const std::runtime_error &e) {
        QMessageBox::warning(this, "Warning", QString("Cannot save configuration: ").append(e.what()));
    }
}

/* ---- MQTT Client event handlers ---- */
void Dashboard::clientConnected() {
    useEmptyConnectedLayout();
    statusBarLabel->setText("Connected to " + widgetManager.getClient()->getCurrentRemoteAddress());
}

void Dashboard::clientConnectionError(const QString &cause) {
    useDisconnectedLayout();

    QMessageBox msgBox;
    QString text = "Connection error: ";
    text.append(cause);
    msgBox.setText(text);
    msgBox.exec();
}

void Dashboard::clientDisconnected(bool forcefully) {
    useDisconnectedLayout();

    if (forcefully) {
        QMessageBox msgBox;
        msgBox.setText("Connection lost");
        msgBox.exec();
    }
}

/* ---- MQTT Widget manager event handlers ---- */
void Dashboard::widgetAdded(QWidget *widget) {
    if (widgetCount == 0) {
        useFilledConnectedLayout();
    }

    contentLayout->addWidget(widget);
    widgetCount++;
    scrollArea->update();
}

void Dashboard::widgetRemoved(QWidget *widget) {
    delete widget;
    scrollArea->update();
    widgetCount--;
    if (widgetCount == 0) {
        useEmptyConnectedLayout();
    }
}

/* ---- Dialog event handlers ---- */
void Dashboard::connectAddressConfirmed(const QString &address, const QString &userName, const QString &password) {
    ui->actionConnect->setEnabled(false);
    ui->statusIndicator->setEnabled(false);

    ui->statusIndicator->setText("Connecting");
    statusBarLabel->setText("Connecting");

    widgetManager.getClient()->connect(address, userName, password);
}

/* ---- Functions for managing the current layout of this window ---- */
void Dashboard::useDisconnectedLayout() {
    ui->centralWidget->setCurrentWidget(ui->disconnectedWidget);
    widgetCount = 0;

    if (scrollArea != nullptr) {
        ui->centralWidget->removeWidget(scrollArea);
        scrollArea->deleteLater();
        scrollArea = nullptr;
    }

    // Disconnect big status indicator from whatever it's connected to
    disconnect(currentIndicatorConnection);
    // and connect it to the same signal that handles the "Connect" menu action
    currentIndicatorConnection = connect(ui->statusIndicator, &QPushButton::clicked, this,
                                         &Dashboard::on_actionConnect_triggered);

    ui->statusIndicator->setText("Disconnected");
    ui->statusIndicator->setEnabled(true);
    statusBarLabel->setText("Disconnected");

    ui->actionDisconnect->setVisible(false);
    ui->actionConnect->setEnabled(true); // Connect might be disabled after handling connectAddressConfirmed
    ui->actionConnect->setVisible(true);
    ui->actionAddTopic->setVisible(false);
    ui->actionLoadConfig->setEnabled(false);
    ui->actionSave->setEnabled(false);
    ui->actionSaveAs->setEnabled(false);
}

void Dashboard::useEmptyConnectedLayout() {
    ui->centralWidget->setCurrentWidget(ui->disconnectedWidget);

    if (scrollArea != nullptr) {
        ui->centralWidget->removeWidget(scrollArea);
        scrollArea->deleteLater();
        scrollArea = nullptr;
    }

    ui->actionDisconnect->setVisible(true);
    ui->actionConnect->setVisible(false);
    ui->actionAddTopic->setVisible(true);
    ui->actionLoadConfig->setEnabled(true);
    ui->actionSave->setEnabled(true);
    ui->actionSaveAs->setEnabled(true);

    // Disconnect big status indicator from whatever it's connected to
    disconnect(currentIndicatorConnection);
    // and connect it to the Add Topic dialog
    currentIndicatorConnection = connect(ui->statusIndicator, &QPushButton::clicked, this,
                                         &Dashboard::on_actionAddTopic_triggered);

    ui->statusIndicator->setEnabled(true);
    ui->statusIndicator->setText("No topics added");
}

void Dashboard::useFilledConnectedLayout() {
    if (scrollArea != nullptr) return;

    ui->actionDisconnect->setVisible(true);
    ui->actionConnect->setVisible(false);
    ui->actionAddTopic->setVisible(true);
    ui->actionLoadConfig->setEnabled(true);
    ui->actionSave->setEnabled(true);
    ui->actionSaveAs->setEnabled(true);

    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    contentWidget = new QWidget;
    scrollArea->setWidget(contentWidget);

    contentLayout = new FlowLayout();

    ui->centralWidget->addWidget(scrollArea);
    ui->centralWidget->setCurrentWidget(scrollArea);

    contentWidget->setLayout(contentLayout);
}



