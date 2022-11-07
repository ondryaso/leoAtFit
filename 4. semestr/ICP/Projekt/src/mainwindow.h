/** @file mainwindow.h
 *
 * @brief Main window declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_MAINWINDOW_H
#define ICP_MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Represents the initial welcome screen in the application.
 */
class MainWindow : public QMainWindow {
Q_OBJECT

public:
    /**
     * @brief Creates a new main window.
     * @param parent Parent widget of the window, NULL by default.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destroys the main window.
     */
    ~MainWindow() override;

private slots:
    /**
     * @brief Opens a new window for the MQTT explorer.
     */
    void on_explorerButton_clicked();

    /**
     * @brief Opens a new window for the MQTT dashboard.
     */
    void on_dashboardButton_clicked();

    /**
     * @brief Opens a new window for the simulator.
     */
    void on_simulatorButton_clicked();

private:
    Ui::MainWindow *ui; /**< Pointer to the window UI. */
};

#endif //ICP_MAINWINDOW_H
