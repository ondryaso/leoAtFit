/** @file text_mqtt_widget.cpp
 *
 * @brief Raw text presenting MQTT widget definition.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */


#include <QPushButton>
#include <QDialog>
#include "text_mqtt_widget.h"

TextMqttWidget::TextMqttWidget(const QString &name, const QString &topic, bool multiLine,
                               const QString &iconUri, QWidget *parent)
        : MqttWidgetBase(iconUri, name, topic, parent), multiLine(multiLine) {
    textLabel = new QLabel("waiting for message");

    auto *layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(textLabel, 0, Qt::AlignCenter);

    if (multiLine) {
        auto showMoreBtn = new QPushButton("Show");
        connect(showMoreBtn, &QPushButton::clicked, this, &TextMqttWidget::showMoreClicked);
        layout->addStretch();
        layout->addWidget(showMoreBtn);

        dialog = new QDialog(this, Qt::Dialog);
        dialog->setWindowTitle(QString("Last message from ").append(topic));
        auto dialogLayout = new QHBoxLayout(dialog);
        dialogTextEdit = new QPlainTextEdit(dialog);
        dialogLayout->addWidget(dialogTextEdit);
        dialog->setLayout(dialogLayout);
        dialog->resize(500, 400);
    }

    contentWidget->setLayout(layout);
}

void TextMqttWidget::processMessage(mqtt::const_message_ptr message) {
    lastMessage = QString::fromStdString(message->to_string());

    QString m = lastMessage;
    int spaceIndex = lastMessage.indexOf('\n');
    if (spaceIndex != -1) {
        m = m.left(spaceIndex);
    }

    if (multiLine) {
        if (m.length() > 20) {
            m = m.left(20).append("…");
        }

        dialogTextEdit->setPlainText(lastMessage);
    } else {
        // In the single-line mode, we can show more characters
        if (m.length() > 32) {
            m = m.left(32).append("…");
        }
    }

    textLabel->setText(m);
}

void TextMqttWidget::showMoreClicked(bool checked) {
    dialog->show();
}

MqttWidgetType TextMqttWidget::getWidgetType() {
    return multiLine ? MqttWidgetType::TEXT_MULTILINE : MqttWidgetType::TEXT_BASIC;
}
