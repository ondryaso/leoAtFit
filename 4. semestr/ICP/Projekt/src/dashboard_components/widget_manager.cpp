#include "widget_manager.h"
#include "widgets/text_mqtt_widget.h"
#include "widgets/temp_mqtt_widget.h"
#include "widgets/switch_mqtt_widget.h"
#include "widgets/temp_raw_mqtt_widget.h"
#include <iostream>
#include <QMessageBox>
#include <QTextStream>
#include <QTime>
#include <QCoreApplication>

/** @file widget_manager.cpp
 *
 * @brief Dashboard MQTT widget manager declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

MqttWidgetManager::MqttWidgetManager(QObject *parent) : QObject(parent) {
    client = new DashboardMqttClient(this);

    connect(client, &DashboardMqttClient::messageSent, this, &MqttWidgetManager::messageSent);
    connect(client, &DashboardMqttClient::messageReceived, this, &MqttWidgetManager::messageReceived);
    connect(client, &DashboardMqttClient::subscribed, this, &MqttWidgetManager::subscribed);
    connect(client, &DashboardMqttClient::unsubscribed, this, &MqttWidgetManager::unsubscribed);
    connect(client, &DashboardMqttClient::disconnected, this, &MqttWidgetManager::clientDisconnected);
}

void MqttWidgetManager::showAddWidgetDialog() {
    auto topicDialog = new DashboardAddTopic;
    connect(topicDialog, &DashboardAddTopic::dialogConfirmed, this, &MqttWidgetManager::makeWidget);
    topicDialog->exec();
    delete topicDialog;
}

void MqttWidgetManager::makeWidget(int widgetType, QString const &topic, QString const &name) {
    MqttWidgetBase *newWidget;

    switch (widgetType) {
        case MqttWidgetType::BOOL:
            newWidget = new SwitchMqttWidget(name, topic);
            break;
        case MqttWidgetType::TEXT_BASIC:
            newWidget = new TextMqttWidget(name, topic);
            break;
        case MqttWidgetType::TEXT_MULTILINE:
            newWidget = new TextMqttWidget(name, topic, true);
            break;
        case MqttWidgetType::TEMP_TEXT:
            newWidget = new RawTempMqttWidget(name, topic);
            break;
        case MqttWidgetType::TEMP_FLOAT:
            newWidget = new TempMqttWidget(name, topic);
            break;
        case MqttWidgetType::TEMP_FLOAT_F:
            newWidget = new TempMqttWidget(name, topic, false);
            break;
    }

    auto topicStdString = topic.toStdString();
    auto v = targets.find(topicStdString);
    if (v == targets.end()) {
        auto newVec = std::vector<MqttWidgetBase *>{newWidget};
        targets.insert({topicStdString, newVec});
    } else {
        v->second.push_back(newWidget);
    }

    // Also add to the end of our linear list
    widgets.push_back(newWidget);

    client->subscribe(topic);
    connect(newWidget, &MqttWidgetBase::requestRemoval, this, &MqttWidgetManager::widgetRequiresRemoval);
    connect(newWidget, &MqttWidgetBase::publishMessage, this, &MqttWidgetManager::widgetRequiresMessageSend);
    emit widgetAdded(newWidget);
}

void MqttWidgetManager::widgetRequiresRemoval() {
    auto widget = qobject_cast<MqttWidgetBase *>(sender());

    // Iterate through the targets map
    for (auto it = targets.begin(); it != targets.end(); it++) {
        // Each element of the map contains a vector of widgets, so iterate through that
        auto &widgets = it->second;
        bool found = false;

        for (auto widgetIt = widgets.begin(); widgetIt != widgets.end(); widgetIt++) {
            if (*widgetIt == widget) {
                widgets.erase(widgetIt);
                // A single widget won't be in the vector multiple times, break the inner for
                // and take a note that the widget has been found
                found = true;
                break;
            }
        }

        if (found) {
            if (widgets.empty()) {
                // If no more widgets are associated with this topic, unsubscribe and remove it from the targets map altogether
                client->unsubscribe(QString::fromStdString(it->first));
                targets.erase(it);
            }

            // A widget will always be only associated with one topic
            // If we found it, we don't have to go through the rest of the targets map
            break;
        }
    }

    // Remove from our linear list
    auto position = std::find(widgets.begin(), widgets.end(), widget);
    if (position != widgets.end())
        widgets.erase(position);

    emit widgetRemoved(widget);
}

void MqttWidgetManager::widgetRequiresMessageSend(mqtt::message_ptr message) {
    client->publishMessage(message);
}

void MqttWidgetManager::messageReceived(mqtt::const_message_ptr message) {
    auto v = targets.find(message->get_topic()); // Find the list of widgets bound to the incoming message's topic
    if (v == targets.end()) return; // This topic has no associated widgets, return.
    auto targetWidgets = v->second;
    for (auto &w : targetWidgets) {
        // Invoke the widget's processing method in its event loop
        QMetaObject::invokeMethod(w, "processMessage", Qt::QueuedConnection, Q_ARG(mqtt::const_message_ptr, message));
    }
}

void MqttWidgetManager::messageSent(mqtt::const_message_ptr ptr) {

}

void MqttWidgetManager::subscribed(const QString &topic) {

}

void MqttWidgetManager::unsubscribed(const QString &topic) {

}

DashboardMqttClient *MqttWidgetManager::getClient() {
    return client;
}

MqttWidgetManager::~MqttWidgetManager() {
    delete client;
}

void MqttWidgetManager::saveConfig(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QFile::Text)) {
        throw std::runtime_error(file.errorString().toStdString());
    }

    QTextStream out(&file);

    // Iterate through the linear list of widgets
    for (auto &widget : widgets) {
        out << QString::number(widget->getWidgetType()) << '\n'
            << widget->getTopic() << '\n'
            << widget->getName() << '\n';
    }

    file.close();
}

void MqttWidgetManager::loadConfig(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        throw std::runtime_error(file.errorString().toStdString());
    }

    QTextStream in(&file);

    // 0 - type, 1 - topic, 2 - name
    int state = 0;
    int type;
    QString topic;
    QString name;

    while (!in.atEnd()) {
        auto line = in.readLine();
        if (line.isEmpty()) continue; // skip empty lines

        switch (state) {
            case 0: {
                bool ok;
                type = line.toInt(&ok);
                if (type < 0 || type > 5) {
                    file.close();
                    throw std::runtime_error("Invalid file format");
                }

                state = 1;
                break;
            }
            case 1:
                topic = QString(line);
                state = 2;
                break;
            case 2:
                name = QString(line);
                state = 0;

                makeWidget(type, topic, name);
                break;
        }
    }

    if (state != 0) {
        throw std::runtime_error("Invalid file format");
    }
}

void MqttWidgetManager::clientDisconnected(bool forcefully) {
    if (widgets.empty()) return;
    for (auto &widget : widgets) {
        delete widget;
    }
    widgets.clear();
    targets.clear();
}

