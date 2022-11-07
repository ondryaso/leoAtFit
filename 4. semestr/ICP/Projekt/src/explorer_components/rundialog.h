/** @file rundialog.h
 *
 * @brief Run dialog declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_RUNDIALOG_H
#define ICP_RUNDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class RunDialog; }
QT_END_NAMESPACE

/**
 * @brief Represents a run dialog.
 */
class RunDialog : public QDialog {
Q_OBJECT

public:
    /**
     * @brief Creates a new run dialog.
     * @param broker Pointer where IP of the broker will be stored.
     * @param username Pointer where username will be stored.
     * @param password Pointer where password will be stored.
     * @param messages Pointer where message history limit will be stored.
     * @param parent Widget parent of the dialog.
     */
    explicit RunDialog(std::string *broker, std::string *username, std::string *password,
                       unsigned *messages, QWidget *parent = nullptr);

    /**
     * @brief Destroys the run dialog.
     */
    ~RunDialog() override;

    /**
     * @brief Hides the selection for message limit.
     */
    void hideMessageCount();

private slots:
    /**
     * @brief Gathers and validates the data from the dialog and closes it.
     */
    void on_runButton_clicked();

private:
    Ui::RunDialog *ui; /**< Pointer to run dialog UI. */
    std::string *ip; /**< Pointer where IP of the broker will be stored. */
    std::string *username; /**< Pointer where username will be stored. */
    std::string *password; /**< Pointer where password will be stored. */
    unsigned *messageCount; /**< Pointer where max messages will be stored. */
};

#endif //ICP_RUNDIALOG_H
