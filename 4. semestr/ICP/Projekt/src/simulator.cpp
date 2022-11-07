/** @file simulator.cpp
 *
 * @brief Simulator implementation.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <QMessageBox>
#include <QFileDialog>
#include <QThread>
#include <fstream>
#include <stdexcept>
#include "explorer_components/rundialog.h"
#include "simulator.h"
#include "ui_simulator.h"
#include "simulator_components/mqtt_widget.h"
#include "simulator_components/mqtt_worker.h"

Simulator::Simulator(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::Simulator) {
    ui->setupUi(this);
    ui->plainTextEdit->setReadOnly(true);
    ui->actionStop->setVisible(false);
}

Simulator::~Simulator() {
    // Handle the worker thread
    stop();
    delete ui;
}

void Simulator::stop() {
    done.lock();
    if (worker != nullptr) {
        worker->wait();
        delete worker;
        worker = nullptr;
    }
    done.unlock();
}

void Simulator::on_actionClose_triggered() {
    delete this;
}

void Simulator::on_actionLoad_triggered() {
    QString selected = QFileDialog::getOpenFileName(this, tr("Select config"), "/home");
    if (selected.isEmpty()) {
        return;
    }
    ui->plainTextEdit->document()->clear();
    widgets.clear();
    int i = 0;
    std::ifstream infile(selected.toStdString());
    std::string line;

    // Variables for the new object
    std::string widget;
    std::string topic;
    unsigned period;
    std::string fullConfig;
    while (std::getline(infile, line)) {
        fullConfig += (line + '\n');
        switch (i) {
            case 0:
                widget = line;
                break;
            case 1:
                topic = line;
                break;
            case 2:
                try {
                    period = std::stol(line);
                    if (period < 1) {
                        throw std::invalid_argument("");
                    }
                } catch(...) {
                    QMessageBox::critical(this, "Error", "Period must be a number greater than 0");
                    return;
                }
        }
        // Everything loaded, create the object
        if (i == 2) {
            try {
                widgets.emplace_back(MqttWidget(widget, topic, cameraImage, period));
            } catch (...) {
                QMessageBox::critical(this, "Error", "Invalid configuration for widget");
                return;
            }
        }

        i = (i + 1) % 3;
    }
    ui->plainTextEdit->document()->setPlainText(QString::fromStdString(fullConfig));
}

void Simulator::on_actionCameraImage_triggered() {
    QString selected = QFileDialog::getOpenFileName(this, tr("Select image"), "/home",
                                                    tr("Image Files (*.png *.jpg)"));
    if (!selected.isEmpty()) {
        cameraImage = selected.toStdString();
    }
}

void Simulator::on_actionRun_triggered() {
    if (widgets.empty()) {
        QMessageBox::critical(this, "Error", "No config loaded");
        return;
    }
    std::string broker;
    std::string user;
    std::string pass;
    // This is not needed for simulator but is required to reuse RunDialog
    unsigned message;
    auto *dialog = new RunDialog(&broker, &user, &pass, &message, this);
    dialog->hideMessageCount();
    if (dialog->exec() == RunDialog::Accepted) {
        ui->actionRun->setVisible(false);
        ui->actionLoad->setVisible(false);
        ui->actionCameraImage->setVisible(false);
        ui->actionStop->setVisible(true);
        worker = new MqttWorker(std::ref(widgets), std::ref(done), broker, user, pass);
        connect(worker, &MqttWorker::didNotConnect, this, &Simulator::on_didNotConnect);
        worker->start();
    }
}

void Simulator::on_didNotConnect() {
    QMessageBox::critical(this, "Error", "Failed to connect to the broker");
    on_actionStop_triggered();
}

void Simulator::on_actionStop_triggered() {
    stop();
    ui->actionRun->setVisible(true);
    ui->actionLoad->setVisible(true);
    ui->actionCameraImage->setVisible(true);
    ui->actionStop->setVisible(false);
}