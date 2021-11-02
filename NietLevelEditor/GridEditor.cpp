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
#include "EventFilter.hpp"

//======================================================================
GridEditor::GridEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GridEditor),
    m_eventFilter(new EventFilter(this))
{
    ui->setupUi(this);
    layout()->setSizeConstraint(QLayout::SetNoConstraint);
    ui->horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
}

//======================================================================
bool GridEditor::initGrid(const QString &installDir, int levelWidth, int levelHeight)
{
    if(!m_levelDataManager.loadLevelData(installDir))
    {
        return false;
    }
    m_wallDrawMode = WallDrawMode_e::LINE_AND_RECT;
    m_elementSelected = false;
    loadIconPictures(installDir);
    initSelectableWidgets();
    QTableView *tableView = findChild<QTableView*>("tableView");
    assert(tableView);
    if(!m_tableModel)
    {
        m_tableModel = new TableModel(tableView);
    }
    m_tableModel->setLevelSize(levelWidth, levelHeight);
    tableView->setModel(m_tableModel);
    connectSlots();
    setStdTableSize();
    return true;
}

//======================================================================
void GridEditor::connectSlots()
{
    QObject::connect(ui->tableView->selectionModel(),
                     &QItemSelectionModel::currentChanged,
                     this, &GridEditor::stdElementCaseSelectedChanged);
    ui->tableView->viewport()->installEventFilter(m_eventFilter);
    QObject::connect(ui->tableView, &QAbstractItemView::pressed,
                     this, &GridEditor::wallSelection);
    QObject::connect(m_eventFilter, &EventFilter::mouseReleased,
                     this, &GridEditor::wallMouseReleaseSelection);
}

//======================================================================
void GridEditor::setCaseIcon(int x, int y, bool deleteMode)
{
    QModelIndex index = m_tableModel->index(x, y, QModelIndex());
    bool ok;
    if(!deleteMode)
    {
        QIcon currentIcon = getCurrentSelectedIcon();
        ok = m_tableModel->setData(index, QVariant(currentIcon.pixmap({CASE_SPRITE_SIZE, CASE_SPRITE_SIZE})));
    }
    else
    {
        ok = m_tableModel->removeData(index);
    }
    assert(ok);
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
        selectLayout->setIcons(m_drawData[i]);
        if(currentEnum == LevelElement_e::WALL)
        {
            selectLayout->confWallSelectWidget(this);
        }
        QObject::connect(selectLayout, &SelectableLineLayout::lineSelected, this, &GridEditor::setElementSelected);
    }
}


//======================================================================
void GridEditor::loadIconPictures(const QString &installDir)
{
    loadWallsPictures(installDir);
    loadDoorsPictures(installDir);
    loadTriggersPictures(installDir);
    loadTeleportsPictures(installDir);
    loadEnemiesPictures(installDir);
    loadObjectsPictures(installDir);
    loadStaticCeilingElementPictures(installDir);
    loadStaticGroundElementPictures(installDir);
    loadBarrelsPictures(installDir);
    loadExitsPictures(installDir);
}

//======================================================================
void GridEditor::loadWallsPictures(const QString &installDir)
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
void GridEditor::loadDoorsPictures(const QString &installDir)
{
    const std::map<QString, DoorData> &doorsMap = m_levelDataManager.getDoorData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::DOOR);
    m_drawData[currentIndex].reserve(doorsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, DoorData>::const_iterator it = doorsMap.begin(); it != doorsMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second.m_sprite);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, installDir));
    }
}

//======================================================================
void GridEditor::loadTriggersPictures(const QString &installDir)
{
    const std::map<QString, QString> &triggersMap = m_levelDataManager.getTriggerData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::TRIGGER);
    m_drawData[currentIndex].reserve(triggersMap.size());
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, QString>::const_iterator it = triggersMap.begin(); it != triggersMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, installDir));
    }
}

//======================================================================
void GridEditor::loadTeleportsPictures(const QString &installDir)
{
    const std::map<QString, QString> &teleportsMap = m_levelDataManager.getTeleportData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::TELEPORT);
    m_drawData[currentIndex].reserve(teleportsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, QString>::const_iterator it = teleportsMap.begin(); it != teleportsMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, installDir));
    }
}

//======================================================================
void GridEditor::loadEnemiesPictures(const QString &installDir)
{
    const std::map<QString, QString> &enemiesMap = m_levelDataManager.getEnemyData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::ENEMY);
    m_drawData[currentIndex].reserve(enemiesMap.size());
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, QString>::const_iterator it = enemiesMap.begin(); it != enemiesMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, installDir));
    }
}

//======================================================================
void GridEditor::loadObjectsPictures(const QString &installDir)
{
    const std::map<QString, QString> &objectsMap = m_levelDataManager.getObjectData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::OBJECT);
    m_drawData[currentIndex].reserve(objectsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, QString>::const_iterator it = objectsMap.begin(); it != objectsMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, installDir));
    }
}

//======================================================================
void GridEditor::loadStaticCeilingElementPictures(const QString &installDir)
{
    const std::map<QString, QString> &staticCeilingElementsMap = m_levelDataManager.getStaticCeilingData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::STATIC_CEILING);
    m_drawData[currentIndex].reserve(staticCeilingElementsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, QString>::const_iterator it = staticCeilingElementsMap.begin(); it != staticCeilingElementsMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, installDir));
    }
}

//======================================================================
void GridEditor::loadStaticGroundElementPictures(const QString &installDir)
{
    const std::map<QString, QString> &staticGroundElementsMap = m_levelDataManager.getStaticGroundData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::STATIC_GROUND);
    m_drawData[currentIndex].reserve(staticGroundElementsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, QString>::const_iterator it = staticGroundElementsMap.begin(); it != staticGroundElementsMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, installDir));
    }
}

//======================================================================
void GridEditor::loadBarrelsPictures(const QString &installDir)
{
    const std::map<QString, QString> &barrelsMap = m_levelDataManager.getBarrelData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::BARREL);
    m_drawData[currentIndex].reserve(barrelsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, QString>::const_iterator it = barrelsMap.begin(); it != barrelsMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, installDir));
    }
}

//======================================================================
void GridEditor::loadExitsPictures(const QString &installDir)
{
    const std::map<QString, QString> &exitsMap = m_levelDataManager.getExitData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::EXIT);
    m_drawData[currentIndex].reserve(exitsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    for(std::map<QString, QString>::const_iterator it = exitsMap.begin(); it != exitsMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, installDir));
    }
}

//======================================================================
void GridEditor::setWallShape()
{
    int minX = std::min(m_wallFirstCaseSelection.column(), m_wallSecondCaseSelection.column()),
            maxX = std::max(m_wallFirstCaseSelection.column(), m_wallSecondCaseSelection.column()),
            minY = std::min(m_wallFirstCaseSelection.row(), m_wallSecondCaseSelection.row()),
            maxY = std::max(m_wallFirstCaseSelection.row(), m_wallSecondCaseSelection.row());
    switch (m_wallDrawMode)
    {
    case WallDrawMode_e::LINE_AND_RECT:
    {
        for(int i = minX; i < maxX + 1; ++i)
        {
            setCaseIcon(i, minY);
            setCaseIcon(i, maxY);
        }
        for(int i = minY + 1; i < maxY; ++i)
        {
            setCaseIcon(minX, i);
            setCaseIcon(maxX, i);
        }
    }
        break;
    case WallDrawMode_e::DIAGONAL_LINE:
    {
        int j = m_wallFirstCaseSelection.row(), modY, i = m_wallFirstCaseSelection.column();
        modY = (minY == m_wallFirstCaseSelection.row()) ? 1 : -1;
        int modX = (minX == m_wallFirstCaseSelection.column()) ? 1 : -1;
        std::cerr << i << "  " << j << "\n";
        std::cerr << modX << "  " << modY << " MOD\n";
        std::cerr << minX << "  " << maxX << " XX\n";
        std::cerr << minY << "  " << maxY << " YY\n";
        for(;((modY == 1 && j < maxY + 1) || (modY == -1 && j > minY - 1)) &&
            ((modX == 1 && i < maxX + 1) || (modX == -1 && i > minX - 1)); i += modX, j += modY)
        {
            std::cerr << "DDA\n";
            setCaseIcon(i, j);
        }
    }
        break;
    case WallDrawMode_e::DIAGONAL_RECT:
        break;
    }
    //quick fix
    emit ui->tableView->model()->dataChanged(QModelIndex(), QModelIndex());
}

//======================================================================
QPixmap GridEditor::getSprite(const ArrayFloat_t &spriteData, const QString &installDir)
{
    QString pathToCurrentTexture = installDir + "/Ressources/Textures/" +
            m_levelDataManager.getTexturePaths()[static_cast<uint32_t>(spriteData[0])];
    QPixmap image(pathToCurrentTexture);
    QSize textureSize = image.size();
    return image.copy(spriteData[1] * textureSize.width(), spriteData[2] * textureSize.height(),
            spriteData[3] * textureSize.width(), spriteData[4] * textureSize.height()).scaled(CASE_SPRITE_SIZE, CASE_SPRITE_SIZE);
}

//======================================================================
void GridEditor::setElementSelected(LevelElement_e num, int currentSelect)
{
    m_currentElementType = num;
    m_currentSelection = currentSelect;
    m_elementSelected = true;
}

//======================================================================
void GridEditor::stdElementCaseSelectedChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if(!m_elementSelected || m_currentElementType == LevelElement_e::WALL)
    {
        return;
    }
    setCaseIcon(current.column(), current.row(), m_currentElementType == LevelElement_e::DELETE);
}

//======================================================================
void GridEditor::wallSelection(const QModelIndex &index)
{
    if(m_currentElementType != LevelElement_e::WALL)
    {
        return;
    }
    m_wallFirstCaseSelection = index;
}

//======================================================================
void GridEditor::wallMouseReleaseSelection()
{
    if(m_currentElementType != LevelElement_e::WALL)
    {
        return;
    }
    assert(ui->tableView->selectionModel()->selection().indexes().size() == 1);
    m_wallSecondCaseSelection = ui->tableView->selectionModel()->selection().indexes()[0];
    setWallShape();
}

//======================================================================
void GridEditor::setWallDrawModeSelected(int wallDrawMode)
{
    m_wallDrawMode = static_cast<WallDrawMode_e>(wallDrawMode);
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
