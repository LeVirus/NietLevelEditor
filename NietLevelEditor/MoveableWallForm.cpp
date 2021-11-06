#include "MoveableWallForm.hpp"
#include "LineWallMove.hpp"
#include "LevelDataManager.hpp"
#include "GridEditor.hpp"
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
void MoveableWallForm::loadTriggerDisplay(const LevelDataManager &levelDataManager,
                                          const QString &installDir)
{
    const std::map<QString, QString> &triggersMap = levelDataManager.getTriggerData();
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, QString>::const_iterator it = triggersMap.begin(); it != triggersMap.end(); ++it)
    {
        spriteData = levelDataManager.getPictureData(it->second);
        assert(spriteData);
        ui->comboBoxTriggerAppearence->addItem(getSprite(*spriteData, levelDataManager, installDir), "");
    }
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
    if(child)
    {
        m_scrollLayout->removeItem(child);
        delete child;
    }
}

//======================================================================
void MoveableWallForm::moveItemUp(int index)
{

}

//======================================================================
void MoveableWallForm::moveItemDown(int index)
{

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
    LineWallMove *lineLayout = new LineWallMove(m_scrollLayout->children().count());
    lineLayout->setProperties(static_cast<Direction_e>(ui->comboBoxDirection->currentIndex()),
                              ui->spinBoxMoveNumber->value());
    m_scrollLayout->addLayout(lineLayout);
    QObject::connect(lineLayout, SIGNAL(sigRemove(int)),
                     this, SLOT(removeItemAt(int)));
}

//======================================================================
MoveableWallForm::~MoveableWallForm()
{
    delete ui;
}

