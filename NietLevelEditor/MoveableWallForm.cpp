#include "MoveableWallForm.hpp"
#include "LineWallMove.hpp"
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
    QObject::connect(ui->pushButtonAddMove, &QPushButton::clicked, this, &MoveableWallForm::addMove);
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

