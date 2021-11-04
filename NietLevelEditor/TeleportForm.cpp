#include "TeleportForm.hpp"
#include "ui_TeleportForm.h"

//======================================================================
TeleportForm::TeleportForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TeleportForm)
{
    ui->setupUi(this);
    connectSlots();
}

//======================================================================
void TeleportForm::connectSlots()
{
    QObject::connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));
}

//======================================================================
void TeleportForm::conf(int levelWidth, int levelHeight)
{
    ui->spinBoxX->setRange(0, levelWidth);
    ui->spinBoxY->setRange(0, levelHeight);
}

//======================================================================
void TeleportForm::onOkButtonClicked()
{
    m_validateForm = true;
    close();
}

//======================================================================
QPair<int, int> TeleportForm::getSpinValue()const
{
    return {ui->spinBoxX->value(), ui->spinBoxY->value()};
}

//======================================================================
TeleportForm::~TeleportForm()
{
    delete ui;
}
