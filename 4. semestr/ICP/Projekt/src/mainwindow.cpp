/** @file mainwindow.cpp
 *
 * @brief Main window implementation.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "explorer.h"
#include "dashboard.h"
#include "simulator.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);
    this->setFixedSize(this->width(), this->height());

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_explorerButton_clicked() {
    auto *window = new Explorer;
    window->show();
}

void MainWindow::on_dashboardButton_clicked() {
    auto *dashboard = new Dashboard;
    dashboard->show();
}

void MainWindow::on_simulatorButton_clicked() {
    auto *simulator = new Simulator;
    simulator->show();
}