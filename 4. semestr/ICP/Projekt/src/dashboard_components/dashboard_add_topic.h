/** @file dashboard_add_topic.h
 *
 * @brief Add topic dialog declaration.
 *
 * @author František Nečas (xnecas27)
 * @author Ondřej Ondryáš (xondry02)
 */

#ifndef ICP_DASHBOARD_ADD_TOPIC_H
#define ICP_DASHBOARD_ADD_TOPIC_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include "widget_manager.h"
#include "widget_type.h"


QT_BEGIN_NAMESPACE
namespace Ui { class DashboardAddTopic; }
QT_END_NAMESPACE

/**
 * @brief Represents a dialog used for adding a new MQTT widget to a dashboard.
 */
class DashboardAddTopic : public QDialog {
Q_OBJECT

public:
    explicit DashboardAddTopic(QWidget *parent = nullptr);

    ~DashboardAddTopic() override;

signals:

    /**
     * Emitted when the user confirms this dialog by clicking "Add".
     */
    void dialogConfirmed(int widgetType, QString const &topic, QString const &name);

private slots:

    void widgetChanged(int index);

    void topicChanged();

    void confirmed();


private:
    Ui::DashboardAddTopic *ui;
    QComboBox *widgetSelector;
    QLineEdit *nameEdit;
    QLineEdit *topicEdit;

    MqttWidgetType selectedType;

    static const std::map<MqttWidgetType, QString> typeNames;
};


#endif //ICP_DASHBOARD_ADD_TOPIC_H
