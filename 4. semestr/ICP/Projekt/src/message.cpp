/** @file message.cpp
 *
 * @brief Implementation of an MQTT message.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <QPixmap>
#include <fstream>
#include "message.h"

Message::Message(std::string data, direction messageDirection_) {
    content = std::move(data);
    messageDirection = messageDirection_;
    messageType = parse_type();
    std::time_t t = std::time(nullptr);
    time = *std::localtime(&t);
}

Message::type Message::parse_type() {
    image = QImage();
    QByteArray data(content.c_str(), content.length());
    if (image.loadFromData(data, "PNG")) {
        return type::IMAGE_PNG;
    } else if (image.loadFromData(data, "JPG")) {
        return type::IMAGE_JPG;
    }
    // If there is a nullbyte somewhere but the end, it is probably binary.
    for (auto character : content) {
        if (character == '\0') {
            return type::BINARY;
        }
    }
    // Do not try to parse the whole JSON, just check the initial character.
    if (content.length() > 0 && (content[0] == '[' || content[0] == '{')) {
        return type::JSON;
    }
    return type::STRING;
}

void Message::save(const std::string &directory) {
    std::string target = directory + '/' + "payload";
    switch (messageType) {
        case type::BINARY:
            target += ".bin";
            break;
        case type::JSON:
            target += ".json";
            break;
        case type::STRING:
            target += ".txt";
            break;
        case type::IMAGE_JPG:
            target += ".jpg";
            break;
        case type::IMAGE_PNG:
            target += ".png";
            break;
    }
    if (messageType == type::IMAGE_JPG || messageType == type::IMAGE_PNG) {
        image.save(QString::fromStdString(target));
    } else {
        std::ofstream stream(target);
        stream << content;
        stream.close();
    }
}

void MessageHistory::addMessage(std::string data, Message::direction direction) {
    history.emplace_back(Message(std::move(data), direction));
    if (history.size() > limit) {
        history.erase(history.begin());
    }
}

void MessageHistory::saveLatestMessage(const std::string &directory) {
    if (!history.empty()) {
        history.front().save(directory);
    }
}