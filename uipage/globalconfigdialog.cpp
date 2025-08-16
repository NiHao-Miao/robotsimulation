#include "globalconfigdialog.h"
#include "ui_globalconfigdialog.h"
#include "utils/systemsetting.h"
#include "robotcommunication.h"

GlobalConfigDialog::GlobalConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalConfigDialog)
{
    ui->setupUi(this);
    ui->buttonGroup->setId(ui->tcp_checkBox,1);
    ui->buttonGroup->setId(ui->udp_checkBox,0);
    connect(ui->buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [this](int id) {
        ui->stackedWidget->setCurrentIndex(id);
    });
    this->setWindowTitle("全局配置");
    QStringList m_value_list;
    m_value_list.append("无");
    for(int i = 0;i<64;i++){
        m_value_list.append("M"+QString::number(i/32)+QString::number(i%32+8,8));
    }
    for(int i= 0;i<m_value_list.size();i++){
        ui->comboBox_en->addItem(m_value_list[i]);
        ui->comboBox_red->addItem(m_value_list[i]);
        ui->comboBox_green->addItem(m_value_list[i]);
        ui->comboBox_blue->addItem(m_value_list[i]);
        ui->comboBox_yellow->addItem(m_value_list[i]);
    }
    ui->buttonGroup_2->setId(ui->read_cmd,1);
    ui->buttonGroup_2->setId(ui->read_feedback,0);
    ui->buttonGroup_2->setId(ui->read_worldcmd,2);
}

GlobalConfigDialog::~GlobalConfigDialog()
{
    delete ui;
}

void GlobalConfigDialog::open()
{
    QDialog::open();
    InitConfigPara();
}

void GlobalConfigDialog::accept()
{
    QDialog::accept();
    SystemSetting::Instance()->setNetworkPort(ui->network_port_lineEdit->text().toUInt());
    SystemSetting::Instance()->setNetworkComFormat(ui->communication_protocol->text());
    SystemSetting::Instance()->setViewRefreshCycle(ui->refreshCycleLineedit->text().toUInt());
    SystemSetting::Instance()->setUdpNetEnable(ui->udp_checkBox->isChecked());
    SystemSetting::Instance()->setTcpNetIpAddr(ui->tcp_ip_addr->text());
    SystemSetting::Instance()->setTcpNetIpPort(ui->tcp_port->text().toUInt());
    SystemSetting::Instance()->setMaxPointNum(ui->max_point_num_lineedit->text().toUInt());
    SystemSetting::Instance()->setRobotCommandJoint(ui->buttonGroup_2->checkedId());
    SystemSetting::Instance()->setTrajectorEnableBindMValue(ui->comboBox_en->currentIndex());
    SystemSetting::Instance()->setTrajectorRedBindMValue(ui->comboBox_red->currentIndex());
    SystemSetting::Instance()->setTrajectorBlueBindMValue(ui->comboBox_blue->currentIndex());
    SystemSetting::Instance()->setTrajectorYellowBindMValue(ui->comboBox_yellow->currentIndex());
    SystemSetting::Instance()->setTrajectorGreenBindMValue(ui->comboBox_green->currentIndex());
    SystemSetting::Instance()->setNetworkAutoConnect(ui->auto_connect_checkBox->isChecked());
    SystemSetting::Instance()->setNetworkAutoConnectTime(ui->auto_connect_lineEdit->text().toUInt());
}

void GlobalConfigDialog::reject()
{
    QDialog::reject();
}

void GlobalConfigDialog::InitConfigPara()
{
    QString port = QString::number(SystemSetting::Instance()->getNetworkPort());
    ui->network_port_lineEdit->setText(port);
    ui->communication_protocol->setText(SystemSetting::Instance()->getNetworkComFormat());
    ui->open_last_project->setChecked(SystemSetting::Instance()->getOpenLastProject());
    ui->refreshCycleLineedit->setText(QString::number(SystemSetting::Instance()->getViewRefreshCycle()));
    if(SystemSetting::Instance()->getUdpNetEnable()){
        ui->udp_checkBox->setChecked(true);
        ui->stackedWidget->setCurrentIndex(0);
    }else{
        ui->tcp_checkBox->setChecked(true);
        ui->stackedWidget->setCurrentIndex(1);
    }
    ui->tcp_ip_addr->setText(SystemSetting::Instance()->getTcpNetIpAddr());
    ui->tcp_port->setText(QString::number(SystemSetting::Instance()->getTcpNetPort()));
    ui->max_point_num_lineedit->setText(QString::number(SystemSetting::Instance()->getMaxPointNum()));
    ui->debug_log_checkBox->setChecked(SystemSetting::Instance()->getDebugLogEnable());
    ui->buttonGroup_2->button(SystemSetting::Instance()->getRobotCommandJoint())->setChecked(true);
    ui->comboBox_en->setCurrentIndex(SystemSetting::Instance()->getTrajectorEnableBindMValue());
    ui->comboBox_red->setCurrentIndex(SystemSetting::Instance()->getTrajectorRedBindMValue());
    ui->comboBox_green->setCurrentIndex(SystemSetting::Instance()->getTrajectorGreenBindMValue());
    ui->comboBox_blue->setCurrentIndex(SystemSetting::Instance()->getTrajectorBlueBindMValue());
    ui->comboBox_yellow->setCurrentIndex(SystemSetting::Instance()->getTrajectorYellowBindMValue());
    ui->auto_connect_checkBox->setChecked(SystemSetting::Instance()->getNetworkAutoConnect());
    ui->auto_connect_lineEdit->setText(QString::number(SystemSetting::Instance()->getNetworkAutoConnectTime()));
}

void GlobalConfigDialog::on_open_last_project_clicked()
{
    SystemSetting::Instance()->setOpenLastProject(ui->open_last_project->isChecked());
}

void GlobalConfigDialog::on_refreshCycleLineedit_editingFinished()
{

}

void GlobalConfigDialog::on_reset_clicked()
{
    SystemSetting::Instance()->reset();
}

void GlobalConfigDialog::on_debug_log_checkBox_clicked()
{
    SystemSetting::Instance()->setDebugLogEnable(ui->debug_log_checkBox->isChecked());
}

void GlobalConfigDialog::on_auto_connect_checkBox_stateChanged(int arg1)
{
    ui->auto_connect_lineEdit->setEnabled(ui->auto_connect_checkBox->isChecked());
}
