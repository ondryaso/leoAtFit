/** @file message.h
 *
 * @brief Contains declaration of an MQTT message.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <string>
#include <vector>
#include <ctime>

#ifndef ICP_MESSAGE_H
#define ICP_MESSAGE_H

/**
 * @brief Encapsulates an MQTT message.
 */
class Message {
public:
    /**
     * @brief An enum representing the direction of the message.
     *
     * The direction is taken from the point of view of the explorer.
     */
    enum class direction {
        INCOMING, /**< Message that the MQTT explorer received. */
        OUTGOING, /**< Message that the MQTT explorer sent. */
    };

    /**
     * @brief An enum representing the data type of the message.
     */
    enum class type {
        STRING, /**< Message is a simple string. */
        JSON, /**< Message is a JSON. */
        BINARY, /**< Message is binary (not an image). */
        IMAGE_PNG, /**< Message is an png image. */
        IMAGE_JPG, /**< Message is a jpg image. */
    };

    /**
     * @brief Creates a new message.
     * @param data Message data.
     * @param messageDirection_  Message direction (outgoing/incoming).
     */
    explicit Message(std::string data, direction messageDirection_);

    /**
     * @brief Tries to parse a type of the message.
     *
     * First checks if the message contains null bytes, which is a sign of
     * binary data. Then tries to load the data as image, if it fails, the
     * data is considered to be a string. Finally, the first character
     * is checked against { and [ to check for JSON (this isn't perfect
     * but C++ stdlib doesn't have a JSON parser and the application doesn't
     * utilise JSON any further).
     * @return The guessed type.
     */
    type parse_type();

    /**
     * @brief Saves the message to the directory.
     *
     * Saves it as payload file with an extension depending on the type of
     * the message.
     * @param directory Directory to save into.
     */
    void save(const std::string &directory);

    std::string content; /**< The content of the message. */
    direction messageDirection; /**< The direction of the message. */
    type messageType; /**< The type of the message. */
    QImage image; /**< The parsed image. */
    std::tm time; /**< The time it was received/sent. */
};

/**
 * @brief Gathers message history for one particular topic.
 *
 * Its goal is to provide an abstraction around the number of stored messages
 * to the explorer.
 */
class MessageHistory {
public:
    /**
     * @brief Creates a message history with the given limit.
     * @param max The maximum number of messages stored at a time.
     */
    explicit MessageHistory(unsigned max):
        limit(max) {}

    /**
     * @brief Adds a new message to the history.
     * Considers the message history limit while doing so.
     * @param data Content of the message.
     * @param direction Direction of the message.
     */
    void addMessage(std::string data, Message::direction direction);

    /**
     * @brief Saves the latest message to the given directory.
     * @param directory Directory to save into.
     */
    void saveLatestMessage(const std::string &directory);

    /** @brief Gets the number of messages. */
    int messages() const { return history.size(); }

    /** @brief Returns a reference to a message at index. */
    const Message &getMessage(int index) const { return history.at(index); }

private:
    unsigned limit; /**< The maximum number of messages stored. */
    std::vector<Message> history; /**< The message storage. */
};


#endif //ICP_MESSAGE_H
