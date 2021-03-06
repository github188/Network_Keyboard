#include "dialogprogress.h"
#include "ui_dialogprogress.h"

#include "iconhelper.h"
#include "appdef.h"
#include <QDesktopWidget>

#include "types.h"

DialogProgress::DialogProgress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProgress)
{
    ui->setupUi(this);
    this->InitStyle();
    //myHelper::FormInCenter(this);
    int frmX = width();
    int frmY = height();
    QDesktopWidget w;
    int deskWidth = w.availableGeometry().width();
    int deskHeight = w.availableGeometry().height();
    QPoint movePoint(deskWidth / 2 - frmX / 2, deskHeight / 2 - frmY / 2);
    move(movePoint);

    ui->progressBar->setValue(0);
    //ui->btnCancel->setVisible(false);//不可取消
}

DialogProgress::~DialogProgress()
{
    delete ui;
}

void DialogProgress::InitStyle()
{
    this->setProperty("Form", true);
    this->setProperty("CanMove", true);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);
    IconHelper::Instance()->SetIcoMain(ui->lab_Ico, FontSize + 2);
    IconHelper::Instance()->SetIcoClose(ui->btnMenu_Close);
    connect(ui->btnMenu_Close, SIGNAL(clicked()), this, SLOT(close()));


    this->setWindowTitle(ui->lab_Title->text());
    connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(close()));
}

void DialogProgress::setTitle(QString str)
{
    ui->lab_Title->setText(str);
}

void DialogProgress::setRange(int minimum, int maximum)
{
    ui->progressBar->setRange(minimum, maximum);
}

void DialogProgress::slotSetProgressBarValue(int value)
{
    ui->progressBar->setValue(value);

    if (ui->btnCancel->isEnabled())
    {
        if (value > 50)
            ui->btnCancel->setEnabled(false);
    }
}

void DialogProgress::slotClose(int flag) //0 success, 1 user cancel
{
    DBG_PRINT("flag: %d\n", flag);
    done(flag);
}

void DialogProgress::on_btnCancel_clicked()
{
    DBG_PRINT("\n");
    done(1);
}


