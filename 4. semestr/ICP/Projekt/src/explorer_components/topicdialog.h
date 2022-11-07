/** @file topicdialog.h
 *
 * @brief Topic dialog declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_TOPICDIALOG_H
#define ICP_TOPICDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class TopicDialog; }
QT_END_NAMESPACE

/**
 * @brief Represents a dialog that asks a user to insert a new topic.
 */
class TopicDialog : public QDialog {
Q_OBJECT

public:
    /**
     * @brief Creates a new dialog.
     * @param result Pointer to string where the result will be stored.
     * @param allowSlashes Whether slashes are allowed in the user input.
     * @param parent Parent widget of the dialog.
     */
    explicit TopicDialog(std::string *result, bool allowSlashes = false, QWidget *parent = nullptr);

    /**
     * @brief Changes the message in the dialog
     * @param newText The text to show.
     */
    void changeText(std::string newText);

    /**
     * @brief Destroys the dialog.
     */
    ~TopicDialog() override;

private slots:
    /**
     * @brief Reads and validates the user input.
     *
     * Checks if the topic was really given and checks if it
     * contains slashes (throws an error in such case). Sets the result.
     */
    void on_okButton_clicked();

private:
    bool allowSlashes; /**< Whether slashes are allowed in the user input. */
    std::string *result;  /**< Pointer to a string where the result will be stored. */
    Ui::TopicDialog *ui; /**< Pointer to dialog UI. */
};

#endif //ICP_TOPICDIALOG_H
