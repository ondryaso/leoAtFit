/** @file mqtt_widget.cpp
 *
 * @brief MQTT widget implementation
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include <stdexcept>
#include <fstream>
#include <random>
#include "mqtt_widget.h"

MqttWidget::MqttWidget(std::string widgetType_, std::string topic, std::string cameraImage, unsigned int period):
        topic(topic), period(period), cameraImagePath(cameraImage) {
    for (auto &x: widgetType_) {
        x = tolower(x);
    }
    if (widgetType_ == "temperature") {
        widgetType = type::TEMPERATURE;
    } else if (widgetType_ == "watts") {
        widgetType = type::WATTS;
    } else if (widgetType_ == "relay") {
        widgetType = type::RELAY;
    } else if (widgetType_ == "camera") {
        widgetType = type::CAMERA;
    } else if (widgetType_ == "tempcontrol") {
        widgetType = type::TEMPCONTROL;
    } else {
        throw std::invalid_argument("");
    }
    timer = 0;
}

void MqttWidget::generateRandomNumber(int start, int end, std::string &result) {
    std::random_device random;
    std::mt19937 generator(random());
    std::uniform_int_distribution<> distribution(start, end);
    result = std::to_string(distribution(generator));
}

void MqttWidget::loadCameraImage(std::vector<char> &result) {
    if (cameraImagePath.empty()) {
        return;
    }
    std::ifstream stream(cameraImagePath, std::ios::in | std::ifstream::binary);
    char byte;
    while (true) {
        stream.read(&byte, 1);
        if (stream.eof()) {
            break;
        }
        result.push_back(byte);
    }
    stream.close();
}

void MqttWidget::trySend(mqtt::async_client &client) {
    timer++;
    if (timer >= period) {
        std::vector<char> bytes;
        std::string stringData;
        switch (widgetType) {
            case type::TEMPERATURE:
                // Demonstrate JSON
                generateRandomNumber(-5, 30, stringData);
                stringData = "{\"temperature\": \"" + stringData + "\"}";
                break;
            case type::TEMPCONTROL:
                generateRandomNumber(15, 25, stringData);
                break;
            case type::RELAY:
                generateRandomNumber(0, 1, stringData);
                break;
            case type::WATTS:
                generateRandomNumber(0, 1000, stringData);
                break;
            case type::CAMERA:
                loadCameraImage(bytes);
                break;
        }
        if (bytes.empty() && !stringData.empty()) {
            for (auto x : stringData) {
                bytes.push_back(x);
            }
        }
        client.publish(topic, &bytes[0], bytes.size());
        timer = 0;
    }
}