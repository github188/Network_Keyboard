#ifndef PAGE_DEV_MGT_H
#define PAGE_DEV_MGT_H

#include <list>

#include <QWidget>
#include <QMutex>

#include "biz_config.h"
#include "gui_dev.h"

namespace Ui {
class page_dev_mgt;
}

class page_dev_mgt : public QWidget
{
    Q_OBJECT
    
public:
    explicit page_dev_mgt(QWidget *parent = 0);
    ~page_dev_mgt();

    int GetDevList(EM_DEV_TYPE dev_type, std::list<s32> &dev_ip_list);
    int GetDevInfo(EM_DEV_TYPE dev_type, u32 ip, SGuiDev_t *p);

protected:
    void showEvent( QShowEvent * event );

private slots:
    void on_btn_srh_clicked();
    void cmbSrhChange(int index);
    void cmbAddChange(int index);
    void tableWidgetSrhDBClicked(int row, int column);
    void on_btn_add_clicked();
    void on_btn_info_clicked();

private:
    void init_form();//控件
    void init_data();//设备信息

private:
    Ui::page_dev_mgt *ui;

    QMutex mutex;
    QStringList strlist_devtype;    
    MAP_IP_DEV map_nvr;
    MAP_IP_DEV map_patrol_dec;
    MAP_IP_DEV map_switch_dec;
};

#endif // PAGE_DEV_MGT_H