#include "MoveableWallForm.hpp"
#include "LineWallMove.hpp"
#include "LevelDataManager.hpp"
#include "GridEditor.hpp"
#include "TableModel.hpp"
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
int MoveableWallForm::getCurrentTriggerAppearence()
{
    return ui->comboBoxTriggerAppearence->currentIndex();
}

//======================================================================
void MoveableWallForm::setData(const CaseData &data)
{
    clear();
    for(int i = 0; i < data.m_moveWallData->m_memMoveWallData.size(); ++i)
    {
        m_memMove = data.m_moveWallData->m_memMoveWallData[i];
        addMove();
    }
    m_memMove = {};
    ui->comboBoxTriggerType->setCurrentIndex(static_cast<int>(data.m_moveWallData->m_triggerType));
    ui->comboBoxTriggerBehaviourType->setCurrentIndex(static_cast<int>(data.m_moveWallData->m_triggerBehaviour));
    ui->spinBoxVelocity->setValue(data.m_moveWallData->m_velocity);
}

//======================================================================
void MoveableWallForm::setTriggerIcons(const QVector<QIcon> &vectIcon)
{
    for(int i = 0; i < vectIcon.size(); ++i)
    {
        ui->comboBoxTriggerAppearence->addItem(vectIcon[i], "");
    }
}

//======================================================================
void MoveableWallForm::init()
{
    m_confirmed = false;
    clear();
}

//======================================================================
const QObjectList &MoveableWallForm::getWallMove()const
{
    return m_scrollLayout->children();
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
    QObject::connect(ui->spinBoxVelocity, SIGNAL(valueChanged(int)), this, SLOT(modifyVelocity(int)));
    m_triggerType = static_cast<TriggerType_e>(ui->comboBoxTriggerType->currentIndex());
    m_triggerBehaviour = static_cast<TriggerBehaviourType_e>(ui->comboBoxTriggerBehaviourType->currentIndex());
}

//======================================================================
void MoveableWallForm::updateMoveLine()
{
    for(int i = 0; i < m_scrollLayout->children().count(); ++i)
    {
        static_cast<LineWallMove*>(m_scrollLayout->children()[i])->setIndex(i);
    }
}

//======================================================================
void MoveableWallForm::clear()
{
    QLayoutItem* child;
    do
    {
        child = m_scrollLayout->layout()->takeAt(0);
        if(child)
        {
            m_scrollLayout->removeItem(child);
            delete child;
        }
        else
        {
            break;
        }
    }
    while(true);
}

//======================================================================
void MoveableWallForm::removeItemAt(int index)
{
    QLayoutItem* child = m_scrollLayout->layout()->takeAt(index);
    assert(child);
    m_scrollLayout->removeItem(child);
    delete child;
    updateMoveLine();
}

//======================================================================
void MoveableWallForm::moveItemUp(int index)
{
    if(index == 0)
    {
        return;
    }
    LineWallMove* childA = static_cast<LineWallMove*>(m_scrollLayout->layout()->itemAt(index));
    assert(childA);
    LineWallMove* childB = static_cast<LineWallMove*>(m_scrollLayout->layout()->itemAt(index - 1));
    assert(childB);
    swapContent(childA, childB);
}

//======================================================================
void MoveableWallForm::moveItemDown(int index)
{
    if(index == m_scrollLayout->layout()->count() - 1)
    {
        return;
    }
    LineWallMove* childA = static_cast<LineWallMove*>(m_scrollLayout->layout()->itemAt(index));
    assert(childA);
    LineWallMove* childB = static_cast<LineWallMove*>(m_scrollLayout->layout()->itemAt(index + 1));
    assert(childB);
    swapContent(childA, childB);
}

//======================================================================
void MoveableWallForm::treatComboBoxTriggerBehaviour(int index)
{
    assert(index <= 3);
    m_triggerBehaviour = static_cast<TriggerBehaviourType_e>(index);
    bool enabled = m_triggerBehaviour != TriggerBehaviourType_e::AUTO;
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
    m_triggerType = static_cast<TriggerType_e>(index);
    ui->comboBoxTriggerAppearence->setEnabled(m_triggerType == TriggerType_e::DISTANT_SWITCH);
}

//======================================================================
void MoveableWallForm::modifyVelocity(int value)
{
    m_velocity = value;
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
    LineWallMove *lineLayout = new LineWallMove(m_scrollLayout->children().count());
    if(m_memMove)
    {
        lineLayout->setProperties(m_memMove->first, m_memMove->second);
    }
    else
    {
        lineLayout->setProperties(static_cast<Direction_e>(ui->comboBoxDirection->currentIndex()), ui->spinBoxMoveNumber->value());
    }
    m_scrollLayout->addLayout(lineLayout);
    QObject::connect(lineLayout, SIGNAL(sigRemove(int)), this, SLOT(removeItemAt(int)));
    QObject::connect(lineLayout, SIGNAL(sigMoveDown(int)), this, SLOT(moveItemDown(int)));
    QObject::connect(lineLayout, SIGNAL(sigMoveUp(int)), this, SLOT(moveItemUp(int)));
}

//======================================================================
MoveableWallForm::~MoveableWallForm()
{
    delete ui;
}

//======================================================================
void swapContent(LineWallMove *lineWallA, LineWallMove *lineWallB)
{
    assert(lineWallA);
    assert(lineWallB);
    QString str = lineWallA->getQString();
    lineWallA->setQString(lineWallB->getQString());
    lineWallB->setQString(str);
}
