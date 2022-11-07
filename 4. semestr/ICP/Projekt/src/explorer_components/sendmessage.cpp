/** @file sendmessage.cpp
 *
 * @brief Send message dialog implementation.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <QFileDialog>
#include "sendmessage.h"
#include "ui_sendmessage.h"

SendMessage::SendMessage(std::string *file, std::string *message, QWidget *parent) :
        QDialog(parent), ui(new Ui::SendMessage), file(file), message(message) {
    ui->setupUi(this);
}

void SendMessage::on_fileButton_clicked() {
    QString selected = QFileDialog::getOpenFileName(this, tr("Select a file to send"), "/home");
    if (selected.isEmpty()) {
        return;
    }
    *file = selected.toStdString();
    accept();
}

void SendMessage::on_sendButton_clicked() {
    *message = ui->lineEdit->text().toStdString();
    accept();
}

SendMessage::~SendMessage() {
    delete ui;
}
