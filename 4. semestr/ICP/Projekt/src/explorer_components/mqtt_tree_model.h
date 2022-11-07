/** @file mqtt_tree_model.h
 *
 * @brief MQTT Tree model declaration
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 * @see https://doc.qt.io/qt-5/qtwidgets-itemviews-editabletreemodel-example.html
 */

#ifndef ICP_MQTTTREEMODEL_H
#define ICP_MQTTTREEMODEL_H

#include <QVector>
#include <QVariant>
#include <QAbstractItemModel>
#include <mqtt/async_client.h>
#include <mqtt/callback.h>
#include "../message.h"

/** @brief The number of MQTT connection attempts. */
#define CONNECT_ATTEMPTS 5

/** @brief Sleep for this time before retrying to connect. */
#define WAIT_FOR 1

/** @brief QOS to use for the subscription. */
#define QOS 1

/**
 * @brief Represents one item (topic) in the tree model.
 */
class TreeItem {
public:
    /**
     * @brief Creates a new tree item.
     * @param topic The topic that the node represents.
     * @param limit The message history limit.
     * @param parent The parent tree item, nullptr for root node.
     */
    explicit TreeItem(std::string topic, unsigned limit, TreeItem *parent = nullptr);

    /**
     * @brief Destroys the item and its successors.
     */
    ~TreeItem() {
        qDeleteAll(children);
    }

    /**
     * @brief Returns the child on the given index.
     * @param index Index to get the child at.
     * @return The child on index, nullptr if outside bounds.
     */
    TreeItem *child(int index) const;

    /**
     * @brief Gets the data of this one item.
     * @return The data (topic) of the item.
     */
    QVariant data() const {
        return QVariant(QString::fromStdString(topicComponent));
    }

    /**
     * @brief Returns the number of children.
     * @return Number of children.
     */
    int childCount() const {
        return children.size();
    };

    /**
     * @brief Inserts a new child.
     * @param topic The new topic to insert.
     */
    void insertChild(std::string topic);

    /**
     * @brief Gets the parent of this item.
     * @return The parent of the item.
     */
    TreeItem *parent() const {
        return parentItem;
    }

    /**
     * @brief Gets the index of this item in the parent structure.
     * @return The index of this node in the parent's list of children. -1 for root.
     */
     int childNumber() const;

     /**
      * @brief Gets the topic of the node.
      * @return Full node topic.
      */
     const std::string &getTopic() {
         return fullTopic;
     }

     /**
      * @brief Gets the last component of the topic.
      * @return The last topic component.
      */
     const std::string &getComponent() {
         return topicComponent;
     }

     /**
      * @brief Adds a new message to the history.
      * @param message The message to add.
      * @param direction Direction of the message.
      */
     void addMessage(std::string &message, Message::direction direction) {
         history.addMessage(message, direction);
     }

     /**
      * @brief Saves the snapshot of this one topic and its children.
      * @param start Starting directory.
      * @return Whether it was successful.
      */
     bool saveSnapshot(const std::string &start);

     /**
      * @brief Returns a reference to the message history.
      * @return Reference to the message history.
      */
     const MessageHistory &getHistory() {
         return history;
     }

private:

    QVector<TreeItem *> children; /**< Vector of children items. */
    std::string fullTopic; /**< The topic that the node represents, full path (and address). */
    std::string topicComponent; /**< The last component of the topic that is showed on this level. */
    MessageHistory history; /**< The history of the topic. */
    TreeItem *parentItem; /**< The parent of the node. */
    unsigned limit; /**< Message limit. */
};


/**
 * @brief Model encapsulating the MQTT data.
 *
 * This object acts as an underlying data structure for the main tree view.
 * In order to obtain new data, it also acts as an MQTT callback to
 * update the data stored.
 *
 * @see https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_subscribe.cpp
 */
class MqttTreeModel: public QAbstractItemModel, public mqtt::callback, public mqtt::iaction_listener {
    Q_OBJECT

public:
    /**
     * @brief Creates a new tree model.
     * @param client_ MQTT client instance.
     * @param opts_ MQTT client connect options.
     * @param limit Message limit.
     * @param parent The parent object of the model.
     */
    explicit MqttTreeModel(mqtt::async_client *client_, mqtt::connect_options opts_,
                           unsigned limit, QObject *parent = nullptr);

    /**
     * @brief Destroys the model.
     */
    ~MqttTreeModel() override;

    /**
     * @brief Gets the data from a node.
     * @param index The model index to get data from.
     * @param role Role used for the data.
     * @return The obtained data, empty if something went wrong.
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief Gets the data from the root node (header).
     * @param section The column index, must be 0.
     * @param orientation Orientation of the header, must be Horizontal.
     * @param role Role used for the data, must be DisplayRole.
     * @return The obtained header, empty if preconditions are not satisfied.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    /**
     * @brief Generates a new index with the given row and column.
     * @param row Row index.
     * @param column Column index.
     * @param parent Parent node with its children to use for generation.
     * @return The generated index.
     */
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;

    /**
     * @brief Generates a new index from the parent of the given index.
     * @param index The index to use for generation.
     * @return Parent index.
     */
    QModelIndex parent(const QModelIndex &index) const override;

    /**
     * @brief Gets the number of rows under the index.
     * @param parent The index to get the number of rows of.
     * @return The number of rows.
     */
    int rowCount(const QModelIndex &parent) const override;

    /**
     * @brief Gets the number of columns.
     *
     * In this model, it is always one but this method is implemented to satisfy
     * the abstract interface.
     * @return Always 1.
     */
    int columnCount(const QModelIndex &parent) const override;

    /**
     * @brief Returns flags of the index.
     * @param index Index to get the flags of.
     * @return Flags of the index, NoItemFlags if not valid.
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * @brief Inserts a new topic of a specific name.
     *
     * Also checks if the topic doesn't already exist before inserting.
     * @param parent Parent topic.
     * @param topicName The last component of the topic name.
     */
    void insertTopic(const QModelIndex &parent, std::string topicName);

    /**
     * @brief Gets a TreeItem based on model index.
     *
     * This is a convenience method since the model and view deal with
     * indexes, whereas the internal data structure is based on items
     * and we need to be able to convert between them.
     * @param index The index to convert
     * @return Converted TreeItem object.
     */
    TreeItem *getItem(const QModelIndex &index) const;

    // MQTT methods

    /**
     * @brief Tries to reconnect to the broker.
     */
    void reconnect();

    /**
     * @brief Triggers on connection failure.
     */
    void on_failure(const mqtt::token &tok) override;

    /**
     * @brief Triggers on success.
     */
    void on_success(const mqtt::token &tok) override {}

    /**
     * @brief Triggers on connection lost, tries to reconnect.
     */
    void connection_lost(const std::string &cause) override;

    /**
     * @brief Triggers when a message arrives, saves the message and
     * forces the view to update.
     * @param msg The received message
     */
    void message_arrived(mqtt::const_message_ptr msg) override;

    /**
     * @brief Triggers when a message is successfully sent. Only send the message
     * to the model now.
     * @param token The confirmation token.
     */
    void delivery_complete(mqtt::delivery_token_ptr token) override;

    /**
     * @brief Triggers on connection successful, subscribe.
     */
    void connected(const std::string &cause) override {
        client->subscribe(topic, QOS);
    }

    /**
     * @brief Subscribes to a new topic.
     * @param newTopic The topic to subscribe to.
     */
    void changeTopic(const std::string &newTopic);

    /**
     * @brief Saves a snapshot of the current hiearchy.
     * @param start The beginning directory
     * @return Whether it was successful.
     */
    bool saveSnapshot(const std::string &start);

signals:
    /**
     * @brief Signal emitted when the client does not connect successfully.
     */
    void didNotConnect();

    /**
     * @brief Signal emitted when a new message is inserted.
     */
    void newMessage();


private:
    /**
     * @brief Inserts a new message to the tree.
     *
     * Finds the corresponding node in the tree if it exists and adds the
     * message to it. If it doesn't exist, creates the corresponding path.
     * @param messageTopic Topic of the message.
     * @param message Message text to insert.
     * @param direction Direction of the message.
     */
    void insertMessage(const std::string &messageTopic, std::string &message, Message::direction direction);

    TreeItem *rootItem; /**< Pointer to the root of the tree. */
    mqtt::async_client *client; /**< Pointer to the MQTT client instance. */
    mqtt::connect_options opts; /**< MQTT connection options. */
    std::string topic = "#"; /**< MQTT topic to subscribe to. */
    int retry = 0; /**< The current retry number. */
};


#endif //ICP_MQTTTREEMODEL_H
