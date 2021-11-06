#include "MoveableWallForm.hpp"
#include "LineWallMove.hpp"
#include "ui_MoveableWallForm.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <iostream>

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
    //set scroll
    m_scrollLayout = new QVBoxLayout(ui->scrollArea->findChild<QWidget*>("scrollAreaWidgetContents"));
    QObject::connect(ui->comboBoxTriggerBehaviourType, SIGNAL(currentIndexChanged(int)), this,
                     SLOT(treatComboBoxTriggerBehaviour(int)));
    QObject::connect(ui->comboBoxTriggerType, SIGNAL(currentIndexChanged(int)), this,
                     SLOT(treatComboBoxTrigger(int)));
    QObject::connect(ui->pushButtonCancel, &QPushButton::clicked, this, &MoveableWallForm::close);
    QObject::connect(ui->pushButtonOK, &QPushButton::clicked, this, &MoveableWallForm::setConfirmed);
    QObject::connect(ui->pushButtonAddMove, &QPushButton::clicked, this, &MoveableWallForm::addMove);
}

//======================================================================
void MoveableWallForm::treatComboBoxTriggerBehaviour(int index)
{
    assert(index <= 3);
    bool enabled = static_cast<TriggerBehaviourType_e>(index) != TriggerBehaviourType_e::AUTO;
    ui->comboBoxTriggerType->setEnabled(enabled);
    if(!enabled)
    {
        ui->comboBoxTriggerAppearence->setEnabled(false);
    }
    else if(static_cast<TriggerType_e>(ui->comboBoxTriggerType->currentIndex()) ==
            TriggerType_e::DISTANT_SWITCH)
    {
        ui->comboBoxTriggerAppearence->setEnabled(true);
    }
}

//======================================================================
void MoveableWallForm::treatComboBoxTrigger(int index)
{
    assert(index <= 3);
//    bool enabled = ;
    ui->comboBoxTriggerAppearence->setEnabled(static_cast<TriggerType_e>(index) == TriggerType_e::DISTANT_SWITCH);
}

//======================================================================
void MoveableWallForm::setConfirmed()
{
    if(!m_scrollLayout->isEmpty())
    {
        m_confirmed = true;
    }
    else
    {
        QMessageBox::warning(this, "Error", "Enter at least one wall movement.");
        return;
    }
    close();
}

//======================================================================
void MoveableWallForm::addMove()
{
    LineWallMove *lineLayout = new LineWallMove();
    lineLayout->setProperties(static_cast<Direction_e>(ui->comboBoxDirection->currentIndex()),
                              ui->spinBoxMoveNumber->value());
    m_scrollLayout->addLayout(lineLayout);
}

//======================================================================
MoveableWallForm::~MoveableWallForm()
{
    delete ui;
}

