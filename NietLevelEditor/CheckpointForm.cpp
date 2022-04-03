#include "CheckpointForm.hpp"
#include "ui_CheckpointForm.h"

//======================================================================
CheckpointForm::CheckpointForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CheckpointForm)
{
    ui->setupUi(this);
    linkWidgets();
}

//======================================================================
void CheckpointForm::linkWidgets()
{
    QObject::connect(ui->OkButton, SIGNAL(clicked()), this, SLOT(confirm()));
    QObject::connect(ui->CancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

//======================================================================
void CheckpointForm::setMenuEntries(const QPair<uint32_t, Direction_e> &data)
{
    ui->comboBox->setCurrentIndex(static_cast<int>(data.second));
    ui->spinBox->setValue(data.first);
}

//======================================================================
void CheckpointForm::setSpinBoxSize(uint32_t size)
{
    ui->spinBox->setMaximum(size);
}

//======================================================================
QPair<uint32_t, Direction_e> CheckpointForm::getCheckpointData()const
{
    return {ui->spinBox->value(), static_cast<Direction_e>(ui->comboBox->currentIndex())};
}


//======================================================================
void CheckpointForm::confirm()
{
    m_valid = true;
    close();
}

//======================================================================
void CheckpointForm::cancel()
{
    close();
}

//======================================================================
CheckpointForm::~CheckpointForm()
{
    delete ui;
}
