/** @file sendmessage.h
 *
 * @brief Send message dialog declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_SENDMESSAGE_H
#define ICP_SENDMESSAGE_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class SendMessage; }
QT_END_NAMESPACE

/**
 * @brief Represents a dialog for sending messages.
 */
class SendMessage : public QDialog {
Q_OBJECT

public:
    /**
     * @brief Creates the dialog
     * @param file Pointer to where file path will be stored.
     * @param message Pointer to where message to send will be stored.
     * @param parent Widget parent of the dialog.
     */
    explicit SendMessage(std::string *file, std::string *message, QWidget *parent = nullptr);

    /**
     * @brief Destroys the dialog.
     */
    ~SendMessage() override;

private slots:
    /**
     * @brief Accepts the given message.
     */
    void on_sendButton_clicked();

    /**
     * @brief Returns a file to send.
     *
     * Opens up a QFileDialog to the user to select a file, then validates
     * whether a file has been entered and accepts if it has. Otherwise
     * keeps the dialog window up.
     */
    void on_fileButton_clicked();

private:
    Ui::SendMessage *ui; /**< Brief pointer to the dialog UI. */
    std::string *file; /**< Pointer to where file path will be stored. */
    std::string *message; /**< Pointer to where message will be stored. */
};

#endif //ICP_SENDMESSAGE_H
