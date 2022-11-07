/** @file mqtt_widget.h
 *
 * @brief MQTT widget declaration
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_MQTT_WIDGET_H
#define ICP_MQTT_WIDGET_H

#include <string>
#include <mqtt/async_client.h>

/**
 * @brief Represents an MQTT widget.
 *
 * Such a widget can be of multiple types, e.g. a temperature meter, watt meter,
 * camera, relay, etc. It is used for simulating MQTT traffic.
 */
class MqttWidget {
public:
    /**
     * @brief An enum containing various device types.
     */
    enum class type {
        WATTS, /**< Watt meter. */
        TEMPERATURE, /**< Temperature meter. */
        RELAY, /**< Relay. */
        CAMERA, /**< Camera. */
        TEMPCONTROL, /**< Temperature control. */
    };

    /**
     * @brief Creates the widget.
     * @param widgetType_ Type of the widget in a string to parse.
     * @param topic Topic the widget will publish on.
     * @param cameraImage Path to the image which will be used for camera widget type.
     * @param period Period in seconds to use for publishing.
     * @throws std::invalid_argument if widgetType is invalid.
     */
    MqttWidget(std::string widgetType_, std::string topic, std::string cameraImage, unsigned period);

    /**
     * @brief Check if the widget should publish.
     * @param client Client to publish through.
     */
    void trySend(mqtt::async_client &client);

private:
    /**
     * @brief Generates a random number in a given range.
     * @param start Start of the range.
     * @param end End of the range.
     * @param result String where the result will be stored.
     * @see The implementation was inspired by user's Cubbi (https://stackoverflow.com/users/273767/cubbi)
     *  response on Stack Exchange Network (https://stackoverflow.com/a/7560564) in response to
     *  https://stackoverflow.com/q/7560114
     */
    static void generateRandomNumber(int start, int end, std::string &result);

    /**
     * @brief Loads a camera image into a vector of bytes.
     * @param result The result vector.
     */
    void loadCameraImage(std::vector<char> &result);

    type widgetType; /**< Type of the widget. */
    std::string topic; /**< Topic to publish to. */
    std::string cameraImagePath; /**< Path to the image used for camera data. */
    unsigned period; /**< Period to use for publishing. */
    unsigned timer = 0; /**< Inner timer used to check whether the widget should publish. */
};

#endif //ICP_MQTT_WIDGET_H
