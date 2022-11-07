/** @file topicdialog.cpp
 *
 * @brief Topic dialog implementation.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <QMessageBox>
#include "topicdialog.h"
#include "ui_topicdialog.h"

TopicDialog::TopicDialog(std::string *result, bool allowSlashes, QWidget *parent) :
        QDialog(parent), allowSlashes(allowSlashes), ui(new Ui::TopicDialog), result(result) {
    ui->setupUi(this);
}

TopicDialog::~TopicDialog() {
    delete ui;
}

void TopicDialog::on_okButton_clicked() {
    QString text = ui->lineEdit->text();
    if (text.isEmpty()) {
        QMessageBox::critical(this, "Error", "Topic must be specified");
        return;
    }
    std::string data = text.toStdString();
    if (!allowSlashes && data.find('/') != std::string::npos) {
        QMessageBox::critical(this, "Error", "Topic must be a simple name without slashes");
        return;
    }
    *result = data;
    accept();
}

void TopicDialog::changeText(std::string newText) {
    ui->label->setText(QString::fromStdString(newText));
}
