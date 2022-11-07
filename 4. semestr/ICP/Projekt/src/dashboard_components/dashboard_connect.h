/** @file dashboard_connect.h
 *
 * @brief Server connection dialog declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_DASHBOARD_CONNECT_H
#define ICP_DASHBOARD_CONNECT_H

#include <QDialog>
#include <QLineEdit>


QT_BEGIN_NAMESPACE
namespace Ui { class DashboardConnect; }
QT_END_NAMESPACE

/**
 * @brief Represents a dialog used for connecting to an MQTT server.
 */
class DashboardConnect : public QDialog {
Q_OBJECT

public:
    explicit DashboardConnect(QWidget *parent = nullptr);

    ~DashboardConnect() override;

signals:

    /**
     * Emitted when the user confirms this dialog by clicking "Connect".
     * @param address The address to connect to.
     * @param userName The user name to authenticate with the server.
     * @param password The password to authenticate with the server.
     */
    void dialogConfirmed(const QString &address, const QString &userName, const QString &password);

private slots:

    /**
     * Called when the contents of the address field changes.
     * Used to enable or disable the "Connect" button when no address is typed.
     */
    void textChanged();

    /**
     * Called when the "Connect" button is clicked.
     * Emits dialogConfirmed() and closes the dialog.
     */
    void connectAddressConfirmed();


private:
    Ui::DashboardConnect *ui;
    QLineEdit *addressEdit;
    QLineEdit *userNameEdit;
    QLineEdit *passwordEdit;
};


#endif //ICP_DASHBOARD_CONNECT_H
