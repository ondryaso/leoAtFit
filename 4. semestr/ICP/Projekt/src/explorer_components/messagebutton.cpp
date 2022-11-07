/** @file messagebutton.cpp
 *
 * @brief Implementation of button for showing messages.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <sstream>
#include <iomanip>
#include "messagebutton.h"

MessageButton::MessageButton(const Message &message_, QWidget *parent):
        message(message_), QPushButton(QString::fromStdString(getMessage(message_)), parent) {
    setStyleSheet("Text-align:left");
    if (message.messageDirection == Message::direction::OUTGOING) {
        setStyleSheet("Text-align:left; background:rgb(184, 184, 184)");
    }
}

std::string MessageButton::getMessage(const Message& newMessage) {
    std::ostringstream stream;
    stream << std::put_time(&newMessage.time, "%H:%M:%S");
    std::string result = stream.str() + " ";
    if (newMessage.messageType == Message::type::BINARY) {
        return result + "[BINARY]";
    } else if (newMessage.messageType == Message::type::IMAGE_JPG ||
            newMessage.messageType == Message::type::IMAGE_PNG) {
        return result + "[IMAGE]";
    } else {
        std::string showText = newMessage.content.substr(0, BUTTON_CHARS);
        if (showText.size() < newMessage.content.size()) {
            showText += "...";
        }
        return result + showText;
    }
}

void MessageButton::show(QWidget *parent) {
    QWidget *widget;
    QPlainTextEdit *editor;
    QLabel *label;
    if (message.messageType == Message::type::STRING || message.messageType == Message::type::JSON) {
        editor = new QPlainTextEdit(message.content.c_str());
        editor->setReadOnly(true);
        widget = editor;
    } else if (message.messageType == Message::type::BINARY) {
        editor = new QPlainTextEdit("Binary data not shown");
        editor->setReadOnly(true);
        widget = editor;
    } else {
        label = new QLabel();
        label->setPixmap(QPixmap::fromImage(message.image));
        widget = label;
    }
    widget->show();
}