/** @file mqtt_tree_model.cpp
 *
 * @brief MQTT Tree model implementation
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 * @see https://doc.qt.io/qt-5/qtwidgets-itemviews-editabletreemodel-example.html
 */

#include <QMessageBox>
#include <filesystem>
#include "mqtt_tree_model.h"
#include "../message.h"
#include <mqtt/async_client.h>

TreeItem::TreeItem(std::string topic, unsigned int limit, TreeItem *parent):
        limit(limit), history(MessageHistory(limit)), parentItem(parent) {
    topicComponent = topic;
    fullTopic = topic;
    // Construct the whole topic
    if (parent != nullptr) {
        TreeItem *current = parent;
        while (current->parent() != nullptr) {
            fullTopic = current->getComponent() + '/' + fullTopic;
            current = current->parent();
        }
    }
}

TreeItem *TreeItem::child(int index) const {
    if (index < 0 || index >= children.size()) {
        return nullptr;
    }
    return children.at(index);
}

void TreeItem::insertChild(std::string topic) {
    children.push_back(new TreeItem(topic, limit, this));
}

int TreeItem::childNumber() const {
    if (parentItem) {
        return parentItem->children.indexOf(const_cast<TreeItem*>(this));
    }
    return -1;
}

bool TreeItem::saveSnapshot(const std::string &start) {
    std::string current = start + '/' + topicComponent;
    if (!std::filesystem::exists(current)) {
        if (!std::filesystem::create_directories(current)) {
            return false;
        }
    }

    history.saveLatestMessage(current);
    for (auto child : children) {
        child->saveSnapshot(current);
    }
    return true;
}

MqttTreeModel::MqttTreeModel(mqtt::async_client *client_, mqtt::connect_options opts_,
                             unsigned int limit, QObject *parent):
                             QAbstractItemModel(parent),
                             opts(opts_),
                             client(client_) {
    rootItem = new TreeItem("Topics", limit);
}

MqttTreeModel::~MqttTreeModel() {
    delete rootItem;
}

TreeItem * MqttTreeModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        auto *item = static_cast<TreeItem *>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

QVariant MqttTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    TreeItem *item = getItem(index);
    return item->data();
}

QVariant MqttTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0) {
        return rootItem->data();
    }
    return QVariant();
}

int MqttTreeModel::rowCount(const QModelIndex &parent) const {
    const TreeItem *parentItem = getItem(parent);
    return parentItem == nullptr ? 0 : parentItem->childCount();
}

int MqttTreeModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

Qt::ItemFlags MqttTreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractItemModel::flags(index);
}

QModelIndex MqttTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && parent.column() != 0) {
        return QModelIndex();
    }
    TreeItem *parentItem = getItem(parent);
    if (parentItem == nullptr) {
        return QModelIndex();
    }

    TreeItem *child = parentItem->child(row);
    if (child == nullptr) {
        return QModelIndex();
    }
    return createIndex(row, column, child);
}

QModelIndex MqttTreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) {
        return QModelIndex();
    }

    TreeItem *child = getItem(index);
    TreeItem *parent = child == nullptr ? nullptr : child->parent();
    if (parent == rootItem || parent == nullptr) {
        return QModelIndex();
    }
    return createIndex(parent->childNumber(), 0, parent);
}

void MqttTreeModel::insertTopic(const QModelIndex &parent, std::string topicName) {
    TreeItem *item = getItem(parent);
    for (int i = 0; i < item->childCount(); i++) {
        if (item->child(i)->getComponent() == topicName) {
            return;
        }
    }
    beginInsertRows(parent, 0, 0);
    item->insertChild(topicName);
    endInsertRows();
}

void MqttTreeModel::reconnect() {
    try {
        client->connect(opts, nullptr, *this);
    } catch (...) {
        return;
    }
}

void MqttTreeModel::on_failure(const mqtt::token &tok) {
    std::this_thread::sleep_for(std::chrono::seconds(WAIT_FOR));
    if (++retry > CONNECT_ATTEMPTS) {
        emit didNotConnect();
        return;
    }
    reconnect();
}

void MqttTreeModel::connection_lost(const std::string &cause) {
    retry = 0;
    reconnect();
}

void MqttTreeModel::message_arrived(mqtt::const_message_ptr msg) {
    std::string messageCopy = msg->to_string();
    insertMessage(msg->get_topic(), messageCopy, Message::direction::INCOMING);
}

/**
 * @brief Splits topic into components.
 * @param components The result vector.
 * @param topic The topic to split.
 */
static void split_into_components(std::vector<std::string> &components, const std::string &topic) {
    std::string component;
    for (auto character: topic) {
        if (character == '/') {
            components.push_back(component);
            component = "";
        } else {
            component += character;
        }
    }
    // The last one hasn't been added yet
    components.push_back(component);
}

void MqttTreeModel::insertMessage(const std::string &messageTopic, std::string &message,
                                  Message::direction direction) {
    std::vector<std::string> components;
    split_into_components(components, messageTopic);

    int currentComponent = 0;
    TreeItem *current = rootItem;
    QModelIndex currentIndex = QModelIndex();
    while (currentComponent < components.size()) {
        bool found = false;
        for (int i = 0; i < current->childCount(); i++) {
            if (current->child(i)->getComponent() == components[currentComponent]) {
                // Found on this level, continue in the branch.
                currentIndex = index(i, 0, currentIndex);
                current = current->child(i);
                found = true;
            }
        }
        if (!found) {
            // Must insert the component
            insertTopic(currentIndex, components[currentComponent]) ;
            currentIndex = index(current->childCount() - 1, 0, currentIndex);
            current = current->child(current->childCount() - 1);
        }
        currentComponent++;
    }
    current->addMessage(message, direction);
    emit newMessage();
}

void MqttTreeModel::delivery_complete(mqtt::delivery_token_ptr token) {
    std::string messageCopy = token->get_message()->to_string();
    insertMessage(token->get_message()->get_topic(), messageCopy, Message::direction::OUTGOING);
}

void MqttTreeModel::changeTopic(const std::string &newTopic) {
    if (client->is_connected()) {
        try {
            client->unsubscribe(topic);
        } catch (...) {
            // Probably not subscribed, no problem
        }
        topic = newTopic;
        client->subscribe(topic, QOS);
    }
}

bool MqttTreeModel::saveSnapshot(const std::string &start) {
    std::string canonical = std::filesystem::canonical(start);
    if (!std::filesystem::exists(canonical)) {
        if (!std::filesystem::create_directories(canonical)) {
            return false;
        }
    }
    for (int i = 0; i < rootItem->childCount(); i++) {
        if (!rootItem->child(i)->saveSnapshot(canonical)) {
            return false;
        }
    }
    return true;
}