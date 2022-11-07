/** @file explorer.cpp
 *
 * @brief Explorer implementation
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <QFileDialog>
#include <QMessageBox>
#include <chrono>
#include <fstream>
#include "explorer.h"
#include "ui_explorer.h"
#include "explorer_components/topicdialog.h"
#include "explorer_components/rundialog.h"
#include "explorer_components/sendmessage.h"
#include "explorer_components/messagebutton.h"

Explorer::Explorer(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::Explorer) {
    ui->setupUi(this);
    messageContainer = new QWidget();
    ui->scrollArea->setWidget(messageContainer);
}

Explorer::~Explorer() {
    delete ui;
    delete model;
    delete client;
}

void Explorer::on_actionClose_triggered() {
    delete this;
}

void Explorer::on_actionSave_triggered() {
    if (model == nullptr) {
        QMessageBox::critical(this, "Error", "The client must first be connected");
        return;
    }
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select a directory"), "/home",
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (directory.isEmpty() == 0) {
        if (!model->saveSnapshot(directory.toStdString())) {
            QMessageBox::critical(this, "Error", "Snapshot creation failed");
        }
    }
}

void Explorer::on_actionRun_triggered() {
    std::string user;
    std::string pass;
    auto *dialog = new RunDialog(&broker, &user, &pass, &messages, this);
    if (dialog->exec() == QDialog::Accepted) {
        if (model != nullptr) {
            delete model;
            model = nullptr;
        }
        if (client != nullptr) {
            try {
                client->disconnect()->wait();
            } catch (...) {}
            delete client;
            client = nullptr;
        }
        clearRightSide();
        // Setup MQTT client
        client = new mqtt::async_client(broker, CLIENT_ID);
        mqtt::connect_options opts;
        opts.set_user_name(user);
        opts.set_password(pass);
        opts.set_clean_session(true);
        opts.set_connect_timeout(std::chrono::seconds(CLIENT_TIMEOUT));

        if (topic.empty()) {
            // Listen to all
            topic = "#";
        }
        model = new MqttTreeModel(client, opts, messages, this);
        ui->treeView->setModel(model);
        // Bind the selection on view
        connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &Explorer::updateRightSide);
        connect(model, &MqttTreeModel::newMessage, this, &Explorer::updateRightSide);
        connect(model, &MqttTreeModel::didNotConnect, this, &Explorer::handleNoConnection);

        client->set_callback(*model);
        try {
            client->connect(opts, nullptr, *model);
        } catch (const mqtt::exception &exc) {
            std::string message = "Failed to connect to the MQTT broker: " + exc.to_string();
            QMessageBox::critical(this, "Error", message.c_str());
            return;
        }
    }
    delete dialog;
}

void Explorer::on_actionAdd_triggered() {
    if (model == nullptr) {
        return;
    }
    std::string newTopic;
    auto *dialog = new TopicDialog(&newTopic, false, this);
    if (dialog->exec() == QDialog::Accepted) {
        const QModelIndex index = ui->treeView->selectionModel()->currentIndex();
        model->insertTopic(index, newTopic);
    }
}

void Explorer::on_actionNewMessage_triggered() {
    if (ui->treeView->selectionModel() == nullptr) {
        QMessageBox::critical(this, "Error", "The client must first be connected");
        return;
    }
    const QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    if (model != nullptr && index.isValid()) {
        if (model->getItem(index) == nullptr) {
            return;
        }
        std::string topic = model->getItem(index)->getTopic();
        std::string file;
        std::string message;
        auto *dialog = new SendMessage(&file, &message, this);
        if (dialog->exec() == QDialog::Accepted) {
            if (!file.empty()) {
                // Read the file as bytes and send, inspired by
                std::ifstream stream(file, std::ios::in | std::ifstream::binary);
                std::vector<char> bytes(
                        (std::istreambuf_iterator<char>(stream)),
                        (std::istreambuf_iterator<char>()));
                stream.close();
                client->publish(topic, &bytes[0], bytes.size(), QOS, false, nullptr, *model);
            } else {
                client->publish(topic, message.c_str(), message.size(), QOS, false, nullptr, *model);
            }
        }
        delete dialog;
    } else {
        QMessageBox::critical(this, "Error", "A topic must be selected");
    }
}

void Explorer::on_actionChangeTopic_triggered() {
    if (model == nullptr) {
        QMessageBox::critical(this, "Error", "The client must first be connected");
    } else {
        auto dialog = new TopicDialog(&topic, true, this);
        dialog->setWindowTitle("Change to a topic");
        dialog->changeText("New topic (can include wildcards)");
        if (dialog->exec() == QDialog::Accepted) {
            model->changeTopic(topic);
        }
    }
}

void Explorer::clearRightSide() {
    if (messageLayout != nullptr) {
        // Delete the previous layout
        QLayoutItem *item;
        while ((item = messageLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete messageLayout;
        messageLayout = nullptr;
    }
}

void Explorer::updateRightSide() {
    ui->treeView->expandAll();
    const QModelIndex index = ui->treeView->selectionModel()->currentIndex();
    if (model != nullptr && index.isValid()) {
        TreeItem *currentItem = model->getItem(index);
        if (currentItem == nullptr) {
            return;
        }
        clearRightSide();
        // Insert a new layout
        messageLayout = new QVBoxLayout(messageContainer);
        const MessageHistory &history = currentItem->getHistory();
        for (int i = 0; i < history.messages(); i++) {
            auto *button = new MessageButton(history.getMessage(i), messageContainer);
            connect(button, &QPushButton::clicked, this, &Explorer::showMessage);
            messageLayout->addWidget(button);
        }
        messageLayout->addStretch();
    }
}

void Explorer::showMessage() {
    auto *button = qobject_cast<MessageButton *>(sender());
    button->show(this);
}

void Explorer::handleNoConnection() {
    QMessageBox::critical(this, "Error", "Failed to connect to the MQTT client");
    delete model;
    delete client;
    model = nullptr;
    client = nullptr;
}