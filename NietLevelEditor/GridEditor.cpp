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
#include <MoveableWallForm.hpp>
#include "TableModel.hpp"
#include "SelectableLineLayout.hpp"
#include "EventFilter.hpp"
#include "TeleportForm.hpp"

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
    m_wallMoveableMode = false;
    m_wallDrawMode = WallDrawMode_e::LINE_AND_RECT;
    m_elementSelected = false;
    loadIconPictures(installDir);
    initSelectableWidgets();
    initButtons();
    QTableView *tableView = findChild<QTableView*>("tableView");
    assert(tableView);
    if(!m_tableModel)
    {
        m_tableModel = new TableModel(tableView);
    }
    if(!m_teleportForm)
    {
        m_teleportForm = new TeleportForm(this);
    }
    m_teleportForm->conf(levelWidth, levelHeight);
    if(!m_moveableWallForm)
    {
        m_moveableWallForm = new MoveableWallForm(this);
        m_moveableWallForm->setTriggerIcons(m_drawData[static_cast<int>(LevelElement_e::TRIGGER)]);
    }
    m_tableModel->setLevelSize(levelWidth, levelHeight);
    tableView->setModel(m_tableModel);
    connectSlots();
    setStdTableSize();
    m_displayPreview = false;
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
                     this, &GridEditor::mouseReleaseSelection);
}

//======================================================================
void GridEditor::setCaseIcon(int x, int y, bool deleteMode)
{
    QModelIndex index = m_tableModel->index(x, y, QModelIndex());
    bool ok;
    if(!deleteMode)
    {
        if(m_currentElementType == LevelElement_e::TELEPORT)
        {
            m_teleportForm->init();
            m_teleportForm->exec();
            if(!m_teleportForm->valid())
            {
                return;
            }
        }
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
void GridEditor::setPlayerDeparture(int x, int y)
{
    QModelIndex index = m_tableModel->index(x, y, QModelIndex());
    QPixmap pix(CASE_SPRITE_SIZE, CASE_SPRITE_SIZE);
    pix.fill(Qt::darkBlue);
    m_tableModel->setData(index, QVariant(pix));
    m_tableModel->setIdData(index, CaseData{LevelElement_e::PLAYER_DEPARTURE, ""});
    updateGridView();
}

//======================================================================
QIcon GridEditor::getCurrentSelectedIcon() const
{
    uint32_t index = static_cast<uint32_t>(m_currentElementType);
    assert(index < m_drawData.size());
    assert(m_currentSelection < m_drawData[index].size());
    return m_drawData[index][m_currentSelection];
}

//======================================================================
void GridEditor::updateGridView()
{
    emit ui->tableView->model()->dataChanged(QModelIndex(), QModelIndex());
}

//======================================================================
void GridEditor::setLineSelectableEnabled(bool enable)
{
    const QObjectList &children = ui->SelectableLayout->children();
    m_memWallSelectLayout->setWallWidgetsEnabled(enable);
    for(int i = 0; i < children.size(); ++i)
    {
        static_cast<SelectableLineLayout*>(children[i])->setRadioButtonEnabled(enable);
    }
}

//======================================================================
void GridEditor::initSelectableWidgets()
{
    LevelElement_e currentEnum;
    for(uint32_t i = 0; i < static_cast<uint32_t>(LevelElement_e::TOTAL); ++i)
    {
        currentEnum = static_cast<LevelElement_e>(i);
        if(currentEnum == LevelElement_e::TRIGGER)
        {
            continue;
        }
        SelectableLineLayout *selectLayout = new SelectableLineLayout(getStringFromLevelElementEnum(currentEnum), currentEnum, this);
        ui->SelectableLayout->addLayout(selectLayout);
        selectLayout->setIcons(m_drawData[i]);
        if(currentEnum == LevelElement_e::WALL)
        {
            m_memWallSelectLayout = selectLayout;
            selectLayout->confWallSelectWidget(this);
        }
        QObject::connect(selectLayout, &SelectableLineLayout::lineSelected, this, &GridEditor::setElementSelected);
    }
}

//======================================================================
void GridEditor::initButtons()
{
//    QPushButton *button = new QPushButton("Player Departure", this);
//    QObject::connect(button, &QPushButton::clicked, this, &GridEditor::setElementSelected);
//    ui->SelectableLayout->addWidget(button);
}

//======================================================================
void GridEditor::loadIconPictures(const QString &installDir)
{
    loadWallsPictures(installDir);
    loadDoorsPictures(installDir);
    loadTeleportsPictures(installDir);
    loadEnemiesPictures(installDir);
    loadObjectsPictures(installDir);
    loadTriggerDisplay(installDir);
    loadStaticCeilingElementPictures(installDir);
    loadStaticGroundElementPictures(installDir);
    loadBarrelsPictures(installDir);
    loadExitsPictures(installDir);
}

//======================================================================
void GridEditor::loadTriggerDisplay(const QString &installDir)
{
    const std::map<QString, QString> &triggersMap = m_levelDataManager.getTriggerData();
    std::optional<ArrayFloat_t> spriteData;
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::TRIGGER);
    m_drawData[currentIndex].reserve(triggersMap.size());
    for(std::map<QString, QString>::const_iterator it = triggersMap.begin(); it != triggersMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back(getSprite(*spriteData, m_levelDataManager, installDir));
    }
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
        m_drawData[currentIndex].push_back(getSprite(*spriteData, m_levelDataManager, installDir));
    }
}

//======================================================================
void GridEditor::loadDoorsPictures(const QString &installDir)
{
    const std::map<QString, DoorData> &doorsMap = m_levelDataManager.getDoorData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::DOOR);
    m_drawData[currentIndex].reserve(doorsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    bool vertical;
    const int DOOR_POS = (CASE_SPRITE_SIZE / 2 - CASE_SPRITE_SIZE / 10),
            DOOR_WIDTH = CASE_SPRITE_SIZE / 5;
    for(std::map<QString, DoorData>::const_iterator it = doorsMap.begin(); it != doorsMap.end(); ++it)
    {
        QPixmap final(CASE_SPRITE_SIZE, CASE_SPRITE_SIZE);
        final.fill(Qt::transparent);
        QPainter paint(&final);
        vertical = it->second.m_vertical;
        spriteData = m_levelDataManager.getPictureData(it->second.m_sprite);
        assert(spriteData);
        QPixmap baseDoorSprite = getSprite(*spriteData, m_levelDataManager, installDir);
        if(vertical)
        {
            QTransform rotate_disc;
            rotate_disc.rotate(90.0);
            baseDoorSprite  = baseDoorSprite .transformed(rotate_disc);
            paint.drawPixmap(DOOR_POS, 0, DOOR_WIDTH, CASE_SPRITE_SIZE, baseDoorSprite);
        }
        else
        {
            paint.drawPixmap(0, DOOR_POS, CASE_SPRITE_SIZE , DOOR_WIDTH, baseDoorSprite);
        }
        if(it->second.m_cardID)
        {
            std::optional<ArrayFloat_t> cardSpriteData = m_levelDataManager.getPictureData(*it->second.m_cardID);
            assert(cardSpriteData);
            QPixmap cardSprite = getSprite(*cardSpriteData, m_levelDataManager, installDir);
            paint.drawPixmap(CASE_SPRITE_SIZE / 5 * 4, CASE_SPRITE_SIZE / 10,
                             CASE_SPRITE_SIZE / 4, CASE_SPRITE_SIZE / 3, cardSprite);
        }
        m_drawData[currentIndex].push_back(final);
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
        m_drawData[currentIndex].push_back(getSprite(*spriteData, m_levelDataManager, installDir));
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
        m_drawData[currentIndex].push_back(getSprite(*spriteData, m_levelDataManager, installDir));
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
        m_drawData[currentIndex].push_back(getSprite(*spriteData, m_levelDataManager, installDir));
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
        m_drawData[currentIndex].push_back(getSprite(*spriteData, m_levelDataManager, installDir));
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
        m_drawData[currentIndex].push_back(getSprite(*spriteData, m_levelDataManager, installDir));
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
        m_drawData[currentIndex].push_back(getSprite(*spriteData, m_levelDataManager, installDir));
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
        m_drawData[currentIndex].push_back(getSprite(*spriteData, m_levelDataManager, installDir));
    }
}

//======================================================================
bool GridEditor::setWallShape(bool preview)
{
    if(preview)
    {
        if(!m_displayPreview ||
                ui->tableView->selectionModel()->selection().indexes().empty())
        {
            return false;
        }
        m_wallSecondCaseSelection = ui->tableView->selectionModel()->selection().indexes()[0];
    }
    int minX = std::min(m_wallFirstCaseSelection.column(), m_wallSecondCaseSelection.column()),
            maxX = std::max(m_wallFirstCaseSelection.column(), m_wallSecondCaseSelection.column()),
            minY = std::min(m_wallFirstCaseSelection.row(), m_wallSecondCaseSelection.row()),
            maxY = std::max(m_wallFirstCaseSelection.row(), m_wallSecondCaseSelection.row());
    if(preview)
    {
        m_tableModel->clearPreview();
    }
    bool ret = true;
    switch (m_wallDrawMode)
    {
    case WallDrawMode_e::LINE_AND_RECT:
    {
        setWallLineRectShape({minX, minY}, {maxX, maxY}, preview);
    }
        break;
    case WallDrawMode_e::DIAGONAL_LINE:
    {
        setWallDiagLineShape({minX, minY}, {maxX, maxY}, preview);
    }
        break;
    case WallDrawMode_e::DIAGONAL_RECT:
    {
        ret = setWallDiagRectShape({minX, minY}, {maxX, maxY}, preview);
    }
        break;
    }
    //quick fix
    updateGridView();
    return ret;
}

//======================================================================
void GridEditor::setWallLineRectShape(const QPair<int, int> &topLeftIndex,
                                      const QPair<int, int> &bottomRightIndex,
                                      bool preview)
{
    for(int i = topLeftIndex.first; i < bottomRightIndex.first + 1; ++i)
    {
        if(preview)
        {
            m_tableModel->setPreviewCase(i, topLeftIndex.second);
            m_tableModel->setPreviewCase(i, bottomRightIndex.second);
        }
        else
        {
            setCaseIcon(i, topLeftIndex.second);
            setCaseIcon(i, bottomRightIndex.second);
        }
    }
    for(int i = topLeftIndex.second + 1; i < bottomRightIndex.second; ++i)
    {
        if(preview)
        {
            m_tableModel->setPreviewCase(topLeftIndex.first, i);
            m_tableModel->setPreviewCase(bottomRightIndex.first, i);
        }
        else
        {
            setCaseIcon(topLeftIndex.first, i);
            setCaseIcon(bottomRightIndex.first, i);
        }
    }
}

//======================================================================
void GridEditor::setWallDiagLineShape(const QPair<int, int> &topLeftIndex,
                                      const QPair<int, int> &bottomRightIndex,
                                      bool preview)
{
    int j = m_wallFirstCaseSelection.row(), modY, i = m_wallFirstCaseSelection.column();
    modY = (topLeftIndex.second == m_wallFirstCaseSelection.row()) ? 1 : -1;
    int modX = (topLeftIndex.first == m_wallFirstCaseSelection.column()) ? 1 : -1;
    for(;((modY == 1 && j < bottomRightIndex.second + 1) || (modY == -1 && j > topLeftIndex.second - 1)) &&
        ((modX == 1 && i < bottomRightIndex.first + 1) || (modX == -1 && i > topLeftIndex.first - 1)); i += modX, j += modY)
    {
        if(preview)
        {
            m_tableModel->setPreviewCase(i, j);
        }
        else
        {
            setCaseIcon(i, j);
        }
    }
}

//======================================================================
bool GridEditor::setWallDiagRectShape(const QPair<int, int> &topLeftIndex,
                                      const QPair<int, int> &bottomRightIndex,
                                      bool preview)
{
    bool modX = (topLeftIndex.first != m_wallFirstCaseSelection.column()),
            modY = (topLeftIndex.second != m_wallFirstCaseSelection.row());
    QPair bottomRightIndexCpy = bottomRightIndex;
    int diffX = bottomRightIndexCpy.first - topLeftIndex.first,
            diffY = bottomRightIndexCpy.second - topLeftIndex.second;
    if(diffX < 2 || diffY < 2)
    {
        return false;
    }
    bool impairX = diffX % 2, impairY = diffY % 2;
    if(impairX)
    {
        --diffX;
        --bottomRightIndexCpy.first;
    }
    if(impairY)
    {
        --diffY;
        --bottomRightIndexCpy.second;
    }
    int mainDiff = std::min(diffX, diffY);
    if(modX)
    {
        bottomRightIndexCpy.first = m_wallFirstCaseSelection.column();
    }
    else
    {
        bottomRightIndexCpy.first = topLeftIndex.first + mainDiff;
    }
    bottomRightIndexCpy.second = topLeftIndex.second + mainDiff;

    int midX, midY, i;
    if(modX)
    {
        i = bottomRightIndex.first - mainDiff;
        midX = bottomRightIndex.first - (mainDiff / 2);
    }
    else
    {
        i = topLeftIndex.first;
        midX = topLeftIndex.first + (mainDiff / 2);
    }
    if(modY)
    {
        midY = bottomRightIndex.second - (mainDiff / 2);
    }
    else
    {
        midY = topLeftIndex.second + (mainDiff / 2);
    }

    int currentYA = midY, currentYB = midY;
    for(; i < midX + 1; ++i, ++currentYA, --currentYB)
    {
        if(preview)
        {
            m_tableModel->setPreviewCase(i, currentYA);
            m_tableModel->setPreviewCase(i, currentYB);
        }
        else
        {
            setCaseIcon(i, currentYA);
            setCaseIcon(i, currentYB);
        }
    }
    --currentYA;
    ++currentYB;
    for(i = midX ; i < bottomRightIndexCpy.first + 1; ++i, --currentYA, ++currentYB)
    {
        if(preview)
        {
            m_tableModel->setPreviewCase(i, currentYA);
            m_tableModel->setPreviewCase(i, currentYB);
        }
        else
        {
            setCaseIcon(i, currentYA);
            setCaseIcon(i, currentYB);
        }
    }
    return true;
}

//======================================================================
QPixmap getSprite(const ArrayFloat_t &spriteData, const LevelDataManager &levelDataManager,
                  const QString &installDir)
{
    QString pathToCurrentTexture = installDir + "/Ressources/Textures/" +
            levelDataManager.getTexturePaths()[static_cast<uint32_t>(spriteData[0])];
    QPixmap image(pathToCurrentTexture);
    QSize textureSize = image.size();
    return image.copy(spriteData[1] * textureSize.width(), spriteData[2] * textureSize.height(),
            spriteData[3] * textureSize.width(), spriteData[4] *
            textureSize.height()).scaled(CASE_SPRITE_SIZE, CASE_SPRITE_SIZE);
}

//======================================================================
void GridEditor::setElementSelected(LevelElement_e num, int currentSelect)
{
    if(num != LevelElement_e::WALL)
    {
        m_wallMoveableMode = false;
        m_memWallSelectLayout->uncheckMoveableWall();
    }
    m_currentElementType = num;
    m_currentSelection = currentSelect;
    m_elementSelected = true;
}

//======================================================================
void GridEditor::stdElementCaseSelectedChanged(const QModelIndex &current, const QModelIndex &previous)
{
//    if(!m_elementSelected || m_currentElementType == LevelElement_e::TRIGGER)
//    {
//        return;
//    }
    if(m_currentElementType == LevelElement_e::WALL)
    {
        setWallShape(true);
        return;
    }
}

//======================================================================
void GridEditor::wallSelection(const QModelIndex &index)
{
    if(m_currentElementType != LevelElement_e::WALL)
    {
        return;
    }
    m_wallFirstCaseSelection = index;
    m_displayPreview = true;
}

//======================================================================
void GridEditor::mouseReleaseSelection()
{
    QModelIndex caseIndex = ui->tableView->selectionModel()->selection().indexes()[0];
    if(m_currentElementType == LevelElement_e::TRIGGER)
    {
        setCaseIcon(caseIndex.column(), caseIndex.row());
        std::optional<int> index = m_memWallSelectLayout->getSelected();
        assert(index);
        setElementSelected(LevelElement_e::WALL, *index);
        setLineSelectableEnabled(true);
    }
    else if(m_currentElementType == LevelElement_e::WALL)
    {
        assert(ui->tableView->selectionModel()->selection().indexes().size() == 1);
        m_wallSecondCaseSelection = ui->tableView->selectionModel()->selection().indexes()[0];
        bool draw = setWallShape();
        if(m_wallMoveableMode)
        {
            if(draw)
            {
                m_memWallSelectLayout->uncheckMoveableWall();
                if(m_moveableWallForm->isDistantTriggerMode())
                {
                    m_currentElementType = LevelElement_e::TRIGGER;
                    setLineSelectableEnabled(false);
                    m_currentSelection = m_moveableWallForm->getCurrentTriggerAppearence();
                    m_wallMoveableMode = false;
                }
            }
        }
        m_tableModel->clearPreview();
        m_displayPreview = false;
    }
    else
    {
        if(m_currentElementType == LevelElement_e::PLAYER_DEPARTURE)
        {
            setPlayerDeparture(caseIndex.column(), caseIndex.row());
            return;
        }
        setCaseIcon(caseIndex.column(), caseIndex.row(), m_currentElementType == LevelElement_e::DELETE);
    }
    updateGridView();
}

//======================================================================
void GridEditor::setWallDrawModeSelected(int wallDrawMode)
{
    m_wallDrawMode = static_cast<WallDrawMode_e>(wallDrawMode);
}

//======================================================================
void GridEditor::setWallMoveableMode(int moveableMode)
{
    m_wallMoveableMode = moveableMode;
    if(m_wallMoveableMode)
    {
        m_moveableWallForm->init();
        m_moveableWallForm->exec();
        if(!m_moveableWallForm->confirmed())
        {
            m_wallMoveableMode = false;
            m_memWallSelectLayout->uncheckMoveableWall();
        }
    }
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
    case LevelElement_e::WALL:
        return "Wall";
    case LevelElement_e::TRIGGER:
        return "Trigger";
    case LevelElement_e::DELETE:
        return "Delete";
    case LevelElement_e::PLAYER_DEPARTURE:
        return "Player Departure";
    case LevelElement_e::TOTAL:
        assert(false);
    }
    return "";
}
