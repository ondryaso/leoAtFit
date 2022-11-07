/** @file rundialog.cpp
 *
 * @brief Run dialog implementation.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <QMessageBox>
#include <string>
#include "rundialog.h"
#include "ui_rundialog.h"

RunDialog::RunDialog(std::string *broker, std::string *username, std::string *password,
                     unsigned int *messages, QWidget *parent):
        QDialog(parent), ui(new Ui::RunDialog), ip(broker), username(username),
        password(password), messageCount(messages) {
    ui->setupUi(this);
}

RunDialog::~RunDialog() {
    delete ui;
}

void RunDialog::hideMessageCount() {
    ui->label_3->setVisible(false);
    ui->messages->setVisible(false);
    resize(362, 140);
}

void RunDialog::on_runButton_clicked() {
    QString setIP = ui->broker->text();
    unsigned messages = ui->messages->value();
    if (setIP.isEmpty()) {
        QMessageBox::critical(this, "Error", "Broker IP must be specified");
        return;
    }

    *ip = setIP.toStdString();
    *username = ui->username->text().toStdString();
    *password = ui->password->text().toStdString();
    *messageCount = messages;
    accept();
}
