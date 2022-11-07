/** @file dashboard_add_topic.cpp
 *
 * @brief Add topic dialog definition.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#include "dashboard_add_topic.h"
#include "ui_dashboard_add_topic.h"
#include "widget_type.h"
#include <QLineEdit>

const std::map<MqttWidgetType, QString> DashboardAddTopic::typeNames = {
        {MqttWidgetType::BOOL,           "Switch"},
        {MqttWidgetType::TEMP_FLOAT,     "Temperature (prefer °C)"},
        {MqttWidgetType::TEMP_FLOAT_F,   "Temperature (prefer °F)"},
        {MqttWidgetType::TEMP_TEXT,      "Temperature (raw)"},
        {MqttWidgetType::TEXT_BASIC,     "Single-line text"},
        {MqttWidgetType::TEXT_MULTILINE, "Multi-line text"}
};

DashboardAddTopic::DashboardAddTopic(QWidget *parent) : QDialog(parent), ui(new Ui::DashboardAddTopic),
                                                        selectedType(MqttWidgetType::TEXT_BASIC) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowStaysOnTopHint);
    setMinimumWidth(400);

    widgetSelector = new QComboBox();
    widgetSelector->setEditable(false);

    int i = 0;
    for (auto &v : typeNames) {
        widgetSelector->addItem(v.second, QVariant(v.first));

        if (v.first == MqttWidgetType::TEXT_BASIC) {
            widgetSelector->setCurrentIndex(i);
        } else {
            i++;
        }
    }

    topicEdit = new QLineEdit();
    nameEdit = new QLineEdit();

    ui->formLayout->addRow("Topic type:", widgetSelector);
    ui->formLayout->addRow("Topic:", topicEdit);
    ui->formLayout->addRow("Block name:", nameEdit);

    ui->buttons->addButton("Connect", QDialogButtonBox::AcceptRole);
    ui->buttons->setEnabled(false);

    setWindowModality(Qt::WindowModal);

    connect(widgetSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &DashboardAddTopic::widgetChanged);
    connect(topicEdit, &QLineEdit::textChanged, this, &DashboardAddTopic::topicChanged);
    connect(ui->buttons, &QDialogButtonBox::accepted, this, &DashboardAddTopic::confirmed);
}

DashboardAddTopic::~DashboardAddTopic() {
    delete ui;
}

void DashboardAddTopic::widgetChanged(int index) {
    selectedType = static_cast<MqttWidgetType>(widgetSelector->itemData(index).toInt());
}

void DashboardAddTopic::topicChanged() {
    if (topicEdit->text().length() == 0) {
        ui->buttons->setEnabled(false);
    } else {
        ui->buttons->setEnabled(true);
    }
}

void DashboardAddTopic::confirmed() {
    emit dialogConfirmed(selectedType, topicEdit->text(), nameEdit->text());
    close();
}