cmake_minimum_required(VERSION 3.16)
project(ICP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Widgets)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Widgets)

find_package(PahoMqttCpp REQUIRED)

add_executable(${PROJECT_NAME}
        src/icp.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/mainwindow.ui
        src/explorer.cpp
        src/explorer.h
        src/explorer.ui
        src/dashboard_components/dashboard_connect.cpp
        src/dashboard_components/dashboard_connect.h
        src/dashboard_components/dashboard_connect.ui
        src/dashboard.cpp
        src/dashboard.h
        src/dashboard.ui
        assets/icons.qrc
        src/explorer_components/rundialog.cpp
        src/explorer_components/rundialog.h
        src/explorer_components/rundialog.ui
        src/message.cpp
        src/message.h
        src/explorer_components/mqtt_tree_model.cpp
        src/explorer_components/mqtt_tree_model.h
        src/explorer_components/topicdialog.cpp
        src/explorer_components/topicdialog.h
        src/explorer_components/topicdialog.ui
        src/explorer_components/sendmessage.cpp
        src/explorer_components/sendmessage.h
        src/explorer_components/sendmessage.ui
        src/explorer_components/messagebutton.cpp
        src/explorer_components/messagebutton.h
        src/simulator.cpp
        src/simulator.h
        src/simulator.ui
        src/simulator_components/mqtt_widget.cpp
        src/simulator_components/mqtt_widget.h
        src/simulator_components/mqtt_worker.cpp
        src/simulator_components/mqtt_worker.h
        src/message.cpp
        src/message.h
        src/dashboard_components/mqtt_client.cpp
        src/dashboard_components/mqtt_client.h
        src/dashboard_components/widget_type.h
        src/dashboard_components/widget_manager.cpp
        src/dashboard_components/widget_manager.h
        src/dashboard_components/widgets/mqtt_widget_base.cpp
        src/dashboard_components/widgets/mqtt_widget_base.h
        src/dashboard_components/flow_layout.cpp
        src/dashboard_components/flow_layout.h
        src/dashboard_components/widgets/text_mqtt_widget.cpp
        src/dashboard_components/widgets/text_mqtt_widget.h
        src/dashboard_components/widgets/temp_mqtt_widget.cpp
        src/dashboard_components/widgets/temp_mqtt_widget.h
        src/dashboard_components/dashboard_add_topic.cpp
        src/dashboard_components/dashboard_add_topic.h
        src/dashboard_components/dashboard_add_topic.ui
        src/dashboard_components/widgets/switch_mqtt_widget.cpp
        src/dashboard_components/widgets/switch_mqtt_widget.h
        src/dashboard_components/widgets/temp_raw_mqtt_widget.cpp
        src/dashboard_components/widgets/temp_raw_mqtt_widget.h)

find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)
target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED} PahoMqttCpp::paho-mqttpp3-static)
