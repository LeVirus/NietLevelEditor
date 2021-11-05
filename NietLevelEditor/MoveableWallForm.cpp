#include "MoveableWallForm.hpp"
#include "ui_MoveableWallForm.h"
#include <QVBoxLayout>
#include <QPushButton>

//======================================================================
MoveableWallForm::MoveableWallForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MoveableWallForm)
{
    ui->setupUi(this);
    initUI();
}

//======================================================================
void MoveableWallForm::initUI()
{
    m_scrollLayout = new QVBoxLayout(this);
    ui->scrollArea->setLayout(m_scrollLayout);
    QObject::connect(ui->comboBoxTriggerBehaviourType, SIGNAL(currentIndexChanged(int)), this,
                     SLOT(treatComboBoxTriggerBehaviour(int)));
    QObject::connect(ui->pushButtonCancel, &QPushButton::clicked, this, &MoveableWallForm::close);
    QObject::connect(ui->pushButtonOK, &QPushButton::clicked, this, &MoveableWallForm::setConfirmed);
}

//======================================================================
void MoveableWallForm::treatComboBoxTriggerBehaviour(int index)
{
    assert(index <= 3);
    ui->comboBoxTriggerType->setEnabled(static_cast<TriggerBehaviourType_e>(index) ==
                                        TriggerBehaviourType_e::AUTO);
}

//======================================================================
void MoveableWallForm::setConfirmed()
{
//    if()
    m_confirmed = true;
    close();
}

//======================================================================
MoveableWallForm::~MoveableWallForm()
{
    delete ui;
}

