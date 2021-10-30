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
    loadIconPictures(installDir);
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
    QVBoxLayout *selectableLayout = findChild<QVBoxLayout*>("SelectableLayout");
    assert(selectableLayout);
    LevelElement_e currentEnum;
    for(uint32_t i = 0; i < static_cast<uint32_t>(LevelElement_e::TOTAL); ++i)
    {
        currentEnum = static_cast<LevelElement_e>(i);
        SelectableLineLayout *selectLayout = new SelectableLineLayout(getStringFromLevelElementEnum(currentEnum), currentEnum, this);
        selectableLayout->addLayout(selectLayout);
        if(currentEnum == LevelElement_e::WALL)
        {
            selectLayout->setIcons(m_drawData[static_cast<uint32_t>(LevelElement_e::WALL)]);
        }
        QObject::connect(selectLayout, &SelectableLineLayout::lineSelected, this, &GridEditor::setElementSelected);
    }
}


//======================================================================
void GridEditor::loadIconPictures(const QString &installDir)
{
    const std::map<QString, QStringList> &wallMap = m_levelDataManager.getWallData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::WALL);
    m_drawData[currentIndex].reserve(wallMap.size());
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, QStringList>::const_iterator it = wallMap.begin(); it != wallMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second[0]);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, installDir));
    }
}

//======================================================================
QPixmap GridEditor::getSprite(const ArrayFloat_t &spriteData, const QString &installDir)
{
    QString pathToCurrentTexture = installDir + "/Ressources/Textures/" +
            m_levelDataManager.getTexturePaths()[static_cast<uint32_t>(spriteData[0])];
    QPixmap image(pathToCurrentTexture);
    QSize textureSize = image.size();
    return image.copy(spriteData[1] * textureSize.width(), spriteData[2] * textureSize.height(),
            spriteData[3] * textureSize.width(), spriteData[4] * textureSize.height()).scaled(CASE_SIZE_PX, CASE_SIZE_PX);
}

//======================================================================
void GridEditor::setElementSelected(LevelElement_e num)
{
    m_currentElement = num;
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

//======================================================================
QString getStringFromLevelElementEnum(LevelElement_e num)
{
    switch(num)
    {
    case LevelElement_e::BARREL:
        return "Barrels";
    case LevelElement_e::DOOR:
        return "Doors";
    case LevelElement_e::ENEMY:
        return "Enemies";
    case LevelElement_e::EXIT:
        return "Exits";
    case LevelElement_e::OBJECT:
        return "Objects";
    case LevelElement_e::STATIC_CEILING:
        return "Static ceiling objects";
    case LevelElement_e::STATIC_GROUND:
        return "Static ground objects";
    case LevelElement_e::TELEPORT:
        return "Teleport";
    case LevelElement_e::TRIGGER:
        return "Trigger";
    case LevelElement_e::WALL:
        return "Wall";
    case LevelElement_e::DELETE:
        return "Delete";
    case LevelElement_e::TOTAL:
        assert(false);
    }
    return "";
}
