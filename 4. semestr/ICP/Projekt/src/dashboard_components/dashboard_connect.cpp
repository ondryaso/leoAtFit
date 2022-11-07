/** @file dashboard_connect.cpp
 *
 * @brief Server connection dialog definition.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include "dashboard_connect.h"
#include "ui_dashboard_connect.h"
#include <QLineEdit>


DashboardConnect::DashboardConnect(QWidget *parent) :
        QDialog(parent), ui(new Ui::DashboardConnect) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowStaysOnTopHint);
    setMinimumWidth(400);
    addressEdit = new QLineEdit("tcp://");
    userNameEdit = new QLineEdit;
    passwordEdit = new QLineEdit;
    passwordEdit->setEchoMode(QLineEdit::Password);
    ui->formLayout->addRow("Server address:", addressEdit);
    ui->formLayout->addRow("User name:", userNameEdit);
    ui->formLayout->addRow("Password:", passwordEdit);
    ui->buttons->addButton("Connect", QDialogButtonBox::AcceptRole);
    ui->buttons->setEnabled(false);
    setWindowModality(Qt::WindowModal);
    connect(addressEdit, &QLineEdit::textChanged, this, &DashboardConnect::textChanged);
    connect(ui->buttons, &QDialogButtonBox::accepted, this, &DashboardConnect::connectAddressConfirmed);
}

DashboardConnect::~DashboardConnect() {
    delete ui;
}

void DashboardConnect::textChanged() {
    if (addressEdit->text().length() == 0 || addressEdit->text().endsWith("://")) {
        ui->buttons->setEnabled(false);
    } else {
        ui->buttons->setEnabled(true);
    }
}

void DashboardConnect::connectAddressConfirmed() {
    emit dialogConfirmed(addressEdit->text(), userNameEdit->text(), passwordEdit->text());
    close();
}
