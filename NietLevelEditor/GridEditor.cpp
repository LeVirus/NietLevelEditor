#include "GridEditor.hpp"
#include "ui_GridEditor.h"
#include <QFile>
#include <iostream>

//======================================================================
GridEditor::GridEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GridEditor)
{
    ui->setupUi(this);
}

//======================================================================
bool GridEditor::initGrid(const QString &installDir)
{
    if(!m_levelDataManager.loadLevelData(installDir))
    {
        return false;
    }
    return true;
}

//======================================================================
GridEditor::~GridEditor()
{
    delete ui;
}

