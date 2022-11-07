/** @file widget_type.h
 *
 * @brief Dashboard MQTT widget type enumeration definition.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */


#ifndef ICP_WIDGET_TYPE_H
#define ICP_WIDGET_TYPE_H

/**
 * @brief Represents possible types of MQTT widgets that can be added to the dashboard.
 */
enum MqttWidgetType {
    TEXT_BASIC,     /**< Show a single-line raw text message. */
    TEXT_MULTILINE, /**< Show a multi-line raw text message. */
    BOOL,           /**< Show a toggle. */
    TEMP_FLOAT,     /**< Show a temperature reading (if no units are found, use °C). */
    TEMP_FLOAT_F,   /**< Show a temperature reading (if no units are found, use °F). */
    TEMP_TEXT       /**< Show a temperature reading (as received). */
};

#endif //ICP_WIDGET_TYPE_H
