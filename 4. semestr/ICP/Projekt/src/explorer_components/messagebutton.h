/** @file messagebutton.h
 *
 * @brief Declaration of button for showing messages.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */


#ifndef ICP_MESSAGEBUTTON_H
#define ICP_MESSAGEBUTTON_H

#include <QPushButton>
#include "../message.h"

/** @brief Number of characters to show in the button. */
#define BUTTON_CHARS 50

/**
 * @brief Represents a button that shows a part of a received message.
 *
 * This button stores a reference to the message it shows which simplifies
 * showing the full message in a separate window.
 */
class MessageButton: public QPushButton {
    Q_OBJECT
public:
    /**
     * @brief Creates a button from a message.
     * @param message_ Message to save in the button.
     * @param parent Parent widget
     */
    MessageButton(const Message &message_, QWidget *parent);

    /**
     * @brief Shows the full message in a new window.
     * @param parent The parent widget to tie the new window to.
     */
    void show(QWidget *parent);

private:
    /**
     * @brief Returns a message to show.
     *
     * This message takes the type and length limit into consideration.
     * @param newMessage The message to show (must be passed due to the order of
     *  initialization - the base class constructor is called first and hence
     *  the message member is not defined)
     * @return The message to show in the button.
     */
    std::string getMessage(const Message &newMessage);

    const Message&message; /**< Reference to the underlying message. */
};


#endif //ICP_MESSAGEBUTTON_H
