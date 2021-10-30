#include "GridEditor.hpp"
#include "ui_GridEditor.h"
#include <QFile>
#include <QStandardItem>
#include <QPainter>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <iostream>
#include "TableModel.hpp"
#include "SelectableLineLayout.hpp"

//======================================================================
GridEditor::GridEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GridEditor)
{
    ui->setupUi(this);
}

//======================================================================
bool GridEditor::initGrid(const QString &installDir, int levelWidth, int levelHeight)
{
    if(!m_levelDataManager.loadLevelData(installDir))
    {
        return false;
    }
    initSelectableWidgets();
    QTableView *tableView = findChild<QTableView*>("tableView");
    assert(tableView);
    m_tableModel = new TableModel(tableView);
    m_tableModel->setLevelSize(levelWidth, levelHeight);

    //TEST
//    QPixmap pixmap = QPixmap(installDir + "/Ressources/Textures/walltest.jpg").copy(30, 30, 100, 100).scaled(CASE_SIZE_PX, CASE_SIZE_PX);
//    QModelIndex index = m_tableModel->index(0, 0, QModelIndex());
//    bool ok;
//    ok = m_tableModel->setData(index, QVariant(pixmap));
//    assert(ok);
    //TEST

    adjustTableSize();
    tableView->setModel(m_tableModel);
//    adjustTableSize();
    setStdTableSize();
    return true;
}

//======================================================================
void GridEditor::initSelectableWidgets()
{
    m_levelDataManager.getWallData();
    QVBoxLayout *selectableLayout = findChild<QVBoxLayout*>("SelectableLayout");
    assert(selectableLayout);
    for(uint32_t i = 0; i < static_cast<uint32_t>(LevelElement_e::TOTAL); ++i)
    {
        selectableLayout->addLayout(new SelectableLineLayout("Radio button " + QString(std::to_string(i).c_str()), this));
    }
}

//======================================================================
void GridEditor::adjustTableSize()
{
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

//======================================================================
void GridEditor::setStdTableSize()
{
    for(int32_t i = 0; i < m_tableModel->getTableWidth(); ++i)
    {
        ui->tableView->setColumnWidth(i, CASE_SIZE_PX);
    }
    for(int32_t i = 0; i < m_tableModel->getTableHeight(); ++i)
    {
        ui->tableView->setRowHeight(i, CASE_SIZE_PX);
    }
}

//======================================================================
GridEditor::~GridEditor()
{
    delete ui;
}

