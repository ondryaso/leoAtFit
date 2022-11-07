/** @file icp.cpp
 *
 * @brief The ICP project entrypoint.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <qapplication.h>
#include "mainwindow.h"
#include "mqtt/message.h"

// Enables passing mqtt message pointers between signals and slots.
Q_DECLARE_METATYPE(mqtt::const_message_ptr);

/**
 * @brief Program entrypoint, starts the main window.
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 * @return The program exit code.
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Enables passing mqtt message pointers between signals and slots.
    qRegisterMetaType<mqtt::const_message_ptr>();

    MainWindow window;
    window.show();
    return app.exec();
}
