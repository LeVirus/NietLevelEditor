#include "GridEditor.hpp"
#include "ui_GridEditor.h"
#include <QFile>
#include <QStandardItem>
#include <QPainter>
#include <iostream>
#include "TableModel.hpp"

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
    QTableView *tableView = findChild<QTableView*>("tableView");
    assert(tableView);
    m_tableModel = new TableModel(tableView);
    m_tableModel->setLevelSize(levelWidth, levelHeight);

    QPixmap pixmap = QPixmap(installDir + "/Ressources/Textures/walltest.jpg").copy(30, 30, 100, 100).scaled(CASE_SIZE_PX, CASE_SIZE_PX);
    QModelIndex index = m_tableModel->index(0, 0, QModelIndex());
    bool ok;
    ok = m_tableModel->setData(index, QVariant(pixmap));
    assert(ok);
    adjustTableSize();
    tableView->setModel(m_tableModel);
//    adjustTableSize();
    setStdTableSize();
    return true;
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

