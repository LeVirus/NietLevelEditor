#include "GridEditor.hpp"
#include "ui_GridEditor.h"
#include <QFile>
#include <QStandardItem>
#include <QPainter>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <iostream>
#include <QDir>
#include <MoveableWallForm.hpp>
#include "TableModel.hpp"
#include "SelectableLineLayout.hpp"
#include "EventFilter.hpp"
#include "LineWallMove.hpp"
#include "BackgroundForm.hpp"

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
    if(!m_moveableWallForm)
    {
        m_moveableWallForm = new MoveableWallForm(this);
        m_moveableWallForm->setTriggerIcons(m_drawData[static_cast<int>(LevelElement_e::TRIGGER)]);
    }
    if(!m_backgroundForm)
    {
        m_backgroundForm = new BackgroundForm(m_drawData, this);
    }
    initSelectableWidgets();
    initButtons();
    initMusicDir(installDir);
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
    QModelIndex index = m_tableModel->index(y, x, QModelIndex());
    bool ok;
    if(deleteMode)
    {
        ok = m_tableModel->removeData(index);
    }
    else
    {
        removeElementCase(index);
        std::optional<CaseData> &caseData = m_tableModel->getDataElementCase(index);
        ok = m_tableModel->setData(index, QVariant(getCurrentSelectedIcon().
                                                   pixmap({CASE_SPRITE_SIZE, CASE_SPRITE_SIZE})));
        if(!caseData || (caseData->m_type != LevelElement_e::TRIGGER && caseData->m_type != LevelElement_e::GROUND_TRIGGER))
        {
            m_tableModel->setIdData(index, CaseData{m_currentElementType,
                                                    m_mapElementID[m_currentElementType][m_currentSelection], {}, {}, {}});
        }
        if(m_currentElementType == LevelElement_e::WALL && m_wallMoveableMode)
        {
            if(m_moveableWallForm->getTriggerType() == TriggerType_e::DISTANT_SWITCH ||
                    m_moveableWallForm->getTriggerType() == TriggerType_e::GROUND)
            {
                m_memCurrentLinkTriggerWall.insert({x, y});
            }
            memWallMove(index);
        }
    }
    assert(ok);
}

//======================================================================
void GridEditor::memWallMove(const QModelIndex &index)
{
    std::optional<CaseData> &caseData = m_tableModel->getDataElementCase(index);
    if(!caseData->m_moveWallData)
    {
        caseData->m_moveWallData = MoveWallData();
    }
    caseData->m_moveWallData->clear();
    assert(caseData);
    QPair<int, int> currentPos = {index.column(), index.row()};
    caseData->m_moveWallData = *m_memcurrentMoveWallData;
    for(int i = 0; i < m_memcurrentMoveWallData->m_memMoveWallData.size(); ++i)
    {
        Direction_e currentDir = caseData->m_moveWallData->m_memMoveWallData[i].first;
        for(int j = 0; j < caseData->m_moveWallData->m_memMoveWallData[i].second; ++j)
        {
            QPair<int, int> tableSize = m_tableModel->getTableSize();
            if(currentDir == Direction_e::NORTH)
            {
                if(currentPos.second < 1)
                {
                    return;
                }
                --currentPos.second;
            }
            else if(currentDir == Direction_e::SOUTH)
            {
                if(currentPos.second > tableSize.second - 2)
                {
                    return;
                }
                ++currentPos.second;
            }
            else if(currentDir == Direction_e::EAST)
            {
                if(currentPos.first > tableSize.first - 2)
                {
                    return;
                }
                ++currentPos.first;
            }
            else if(currentDir == Direction_e::WEST)
            {
                if(currentPos.first < 1)
                {
                    return;
                }
                --currentPos.first;
            }
            caseData->m_moveWallData->m_memMoveWallCases.push_back(currentPos);
        }
    }
}

//======================================================================
void GridEditor::setColorCaseData(int x, int y, LevelElement_e type)
{
    QModelIndex index = m_tableModel->index(y, x, QModelIndex());
    QPixmap pix(CASE_SPRITE_SIZE, CASE_SPRITE_SIZE);
    if(type == LevelElement_e::PLAYER_DEPARTURE)
    {
        pix.fill(Qt::darkBlue);
        m_PlayerDeparture = {x, y};
    }
    else
    {
        pix.fill(Qt::magenta);
    }
    if(type == LevelElement_e::PLAYER_DEPARTURE || !m_tableModel->getDataElementCase(index))
    {
        m_tableModel->setIdData(index, CaseData{type, "", {}, {}, {}});
    }
    m_tableModel->setData(index, QVariant(pix));
    updateGridView();
}

//======================================================================
QIcon GridEditor::getCurrentSelectedIcon() const
{
    uint32_t index = static_cast<uint32_t>(m_currentElementType);
    assert(index < m_drawData.size());
    assert(m_currentSelection < m_drawData[index].size());
    return m_drawData[index][m_currentSelection].second;
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
        if(currentEnum == LevelElement_e::TRIGGER || currentEnum == LevelElement_e::GROUND_TRIGGER ||
                currentEnum == LevelElement_e::TARGET_TELEPORT)
        {
            continue;
        }
        SelectableLineLayout *selectLayout = new SelectableLineLayout(getStringFromLevelElementEnum(currentEnum), currentEnum);
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
    QPushButton *button = new QPushButton("Set Ceiling Background");
    ui->SelectableLayout->addWidget(button);
    QObject::connect(button, &QPushButton::clicked, this, &GridEditor::execConfCeilingBackground);
    button = new QPushButton("Set Ground Background");
    ui->SelectableLayout->addWidget(button);
    QObject::connect(button, &QPushButton::clicked, this, &GridEditor::execConfGroundBackground);
}

//======================================================================
void GridEditor::initMusicDir(const QString &installDir)
{
    QComboBox *musicWidget = new QComboBox();
    QString musicDir = installDir + "/Ressources/Audio/Music/";
    QDir dir(musicDir);
    assert(dir.exists());
    QFileInfoList list = dir.entryInfoList();
    QFileInfo fileInfo;
    musicWidget->addItem("None");
    for(int i = 0; i < list.size(); ++i)
    {
        fileInfo = list[i];
        if(fileInfo.suffix() == "flac")
        {
            musicWidget->addItem(fileInfo.fileName());
        }
    }
    ui->SelectableLayout->addWidget(new QLabel("Music"));
    ui->SelectableLayout->addWidget(musicWidget);
}

//======================================================================
void GridEditor::loadIconPictures(const QString &installDir)
{
    m_mapElementID.clear();
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
    m_mapElementID.insert({LevelElement_e::TRIGGER, QVector<QString>()});
    m_mapElementID[LevelElement_e::TRIGGER].reserve(triggersMap.size());
    for(std::map<QString, QString>::const_iterator it = triggersMap.begin(); it != triggersMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::TRIGGER].push_back(it->first);
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back({it->second, getSprite(*spriteData, m_levelDataManager, installDir)});
    }
}

//======================================================================
void GridEditor::loadWallsPictures(const QString &installDir)
{
    const std::map<QString, QStringList> &wallMap = m_levelDataManager.getWallData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::WALL);
    m_drawData[currentIndex].reserve(wallMap.size());
    std::optional<ArrayFloat_t> spriteData;
    int vectSpriteSize, multiSpriteSectionSize;
    m_mapElementID.insert({LevelElement_e::WALL, QVector<QString>()});
    m_mapElementID[LevelElement_e::WALL].reserve(wallMap.size());
    for(std::map<QString, QStringList>::const_iterator it = wallMap.begin(); it != wallMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::WALL].push_back(it->first);
        vectSpriteSize = it->second.size();
        if(vectSpriteSize == 1)
        {
            spriteData = m_levelDataManager.getPictureData(it->second[0]);
            assert(spriteData);
            m_drawData[currentIndex].push_back({it->second[0], getSprite(*spriteData, m_levelDataManager, installDir)});
        }
        else
        {
            multiSpriteSectionSize = CASE_SPRITE_SIZE / vectSpriteSize;
            QPixmap final(CASE_SPRITE_SIZE, CASE_SPRITE_SIZE);
            final.fill(Qt::transparent);
            QPainter paint(&final);
            int currentPos = 0;
            for(int32_t i = 0; i < vectSpriteSize; ++i, currentPos += multiSpriteSectionSize)
            {
                spriteData = m_levelDataManager.getPictureData(it->second[i]);
                QPixmap wallSprite = getSprite(*spriteData, m_levelDataManager, installDir);
                paint.drawPixmap(currentPos, 0, multiSpriteSectionSize,
                                 CASE_SPRITE_SIZE, wallSprite);
            }
            m_drawData[currentIndex].push_back({"", final});
        }
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
    m_mapElementID.insert({LevelElement_e::DOOR, QVector<QString>()});
    m_mapElementID[LevelElement_e::DOOR].reserve(doorsMap.size());
    for(std::map<QString, DoorData>::const_iterator it = doorsMap.begin(); it != doorsMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::DOOR].push_back(it->first);
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
        m_drawData[currentIndex].push_back({it->second.m_sprite, final});
    }
}

//======================================================================
void GridEditor::loadTeleportsPictures(const QString &installDir)
{
    const std::map<QString, QString> &teleportsMap = m_levelDataManager.getTeleportData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::TELEPORT);
    assert(!teleportsMap.empty());
    m_drawData[currentIndex].reserve(teleportsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    m_mapElementID.insert({LevelElement_e::TELEPORT, QVector<QString>()});
    m_mapElementID[LevelElement_e::TELEPORT].reserve(teleportsMap.size());
    for(std::map<QString, QString>::const_iterator it = teleportsMap.begin(); it != teleportsMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::TELEPORT].push_back(it->first);
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back({it->second, getSprite(*spriteData, m_levelDataManager, installDir)});
    }
}

//======================================================================
void GridEditor::loadEnemiesPictures(const QString &installDir)
{
    const std::map<QString, QString> &enemiesMap = m_levelDataManager.getEnemyData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::ENEMY);
    m_drawData[currentIndex].reserve(enemiesMap.size());
    std::optional<ArrayFloat_t> spriteData;
    m_mapElementID.insert({LevelElement_e::ENEMY, QVector<QString>()});
    m_mapElementID[LevelElement_e::ENEMY].reserve(enemiesMap.size());
    for(std::map<QString, QString>::const_iterator it = enemiesMap.begin(); it != enemiesMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::ENEMY].push_back(it->first);
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back({it->second, getSprite(*spriteData, m_levelDataManager, installDir)});
    }
}

//======================================================================
void GridEditor::loadObjectsPictures(const QString &installDir)
{
    const std::map<QString, QString> &objectsMap = m_levelDataManager.getObjectData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::OBJECT);
    m_drawData[currentIndex].reserve(objectsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    m_mapElementID.insert({LevelElement_e::OBJECT, QVector<QString>()});
    m_mapElementID[LevelElement_e::OBJECT].reserve(objectsMap.size());
    for(std::map<QString, QString>::const_iterator it = objectsMap.begin(); it != objectsMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::OBJECT].push_back(it->first);
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back({it->second, getSprite(*spriteData, m_levelDataManager, installDir)});
    }
}

//======================================================================
void GridEditor::loadStaticCeilingElementPictures(const QString &installDir)
{
    const std::map<QString, QString> &staticCeilingElementsMap = m_levelDataManager.getStaticCeilingData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::STATIC_CEILING);
    m_drawData[currentIndex].reserve(staticCeilingElementsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    m_mapElementID.insert({LevelElement_e::STATIC_CEILING, QVector<QString>()});
    m_mapElementID[LevelElement_e::STATIC_CEILING].reserve(staticCeilingElementsMap.size());
    for(std::map<QString, QString>::const_iterator it = staticCeilingElementsMap.begin(); it != staticCeilingElementsMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::STATIC_CEILING].push_back(it->first);
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back({it->second, getSprite(*spriteData, m_levelDataManager, installDir)});
    }
}

//======================================================================
void GridEditor::loadStaticGroundElementPictures(const QString &installDir)
{
    const std::map<QString, QString> &staticGroundElementsMap = m_levelDataManager.getStaticGroundData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::STATIC_GROUND);
    m_drawData[currentIndex].reserve(staticGroundElementsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    m_mapElementID.insert({LevelElement_e::STATIC_GROUND, QVector<QString>()});
    m_mapElementID[LevelElement_e::STATIC_GROUND].reserve(staticGroundElementsMap.size());
    for(std::map<QString, QString>::const_iterator it = staticGroundElementsMap.begin(); it != staticGroundElementsMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::STATIC_GROUND].push_back(it->first);
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back({it->second, getSprite(*spriteData, m_levelDataManager, installDir)});
    }
}

//======================================================================
void GridEditor::loadBarrelsPictures(const QString &installDir)
{
    const std::map<QString, QString> &barrelsMap = m_levelDataManager.getBarrelData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::BARREL);
    m_drawData[currentIndex].reserve(barrelsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    m_mapElementID.insert({LevelElement_e::BARREL, QVector<QString>()});
    m_mapElementID[LevelElement_e::BARREL].reserve(barrelsMap.size());
    for(std::map<QString, QString>::const_iterator it = barrelsMap.begin(); it != barrelsMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::BARREL].push_back(it->first);
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back({it->second, getSprite(*spriteData, m_levelDataManager, installDir)});
    }
}

//======================================================================
void GridEditor::loadExitsPictures(const QString &installDir)
{
    const std::map<QString, QString> &exitsMap = m_levelDataManager.getExitData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::EXIT);
    m_drawData[currentIndex].reserve(exitsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    m_mapElementID.insert({LevelElement_e::EXIT, QVector<QString>()});
    m_mapElementID[LevelElement_e::EXIT].reserve(exitsMap.size());
    for(std::map<QString, QString>::const_iterator it = exitsMap.begin(); it != exitsMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::EXIT].push_back(it->first);
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back({it->second, getSprite(*spriteData, m_levelDataManager, installDir)});
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
    memStdWallMove();
    QPair<int, int> topLeftPos = {minX, minY}, bottomRight = {maxX, maxY};
    if(!preview && topLeftPos == bottomRight)
    {
        setCaseIcon(minX, minY);
        return true;
    }
    bool ret = true;
    switch (m_wallDrawMode)
    {
    case WallDrawMode_e::LINE_AND_RECT:
    {
        setWallLineRectShape(topLeftPos, bottomRight, preview);
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
void GridEditor::memStdWallMove()
{
    if(!m_memcurrentMoveWallData)
    {
        m_memcurrentMoveWallData = std::make_unique<MoveWallData>();
    }
    const QObjectList &moveList = m_moveableWallForm->getWallMove();
    m_memcurrentMoveWallData->clear();
    m_memcurrentMoveWallData->m_velocity = m_moveableWallForm->getVelocity();
    m_memcurrentMoveWallData->m_triggerType = m_moveableWallForm->getTriggerType();
    m_memcurrentMoveWallData->m_triggerBehaviour = m_moveableWallForm->getTriggerBehaviour();
    QString currentMove;
    QStringList subStr;
    Direction_e currentDir;
    int moveNumber;
    for(int i = 0; i < moveList.count(); ++i)
    {
        currentMove = static_cast<LineWallMove*>(moveList[i])->getQString();
        subStr = currentMove.split(' ');
        assert(subStr.size() >= 2);
        currentDir = getDirEnumFromQString(subStr[0]);
        moveNumber = subStr[2].toInt();
        m_memcurrentMoveWallData->m_memMoveWallData.push_back({currentDir, moveNumber});
    }
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
    if(m_currentElementType == LevelElement_e::WALL)
    {
        setWallShape(true);
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
    m_tableModel->clearPreview();
    if(!m_elementSelected)
    {
        return;
    }
    if(m_currentElementType == LevelElement_e::WALL)
    {
        treatWallDrawing();
    }
    else
    {
        treatElementsDrawing();
    }
    updateGridView();
}

//======================================================================
void GridEditor::treatWallDrawing()
{
    assert(ui->tableView->selectionModel()->selection().indexes().size() == 1);
    m_wallSecondCaseSelection = ui->tableView->selectionModel()->selection().indexes()[0];
    m_memCurrentLinkTriggerWall.clear();
    if(m_wallMoveableMode)
    {
        m_moveableWallForm->init();
        const std::optional<CaseData> &caseData = m_tableModel->getDataElementCase(m_wallFirstCaseSelection);
        if(caseData && caseData->m_type == LevelElement_e::WALL && caseData->m_moveWallData)
        {
            m_moveableWallForm->setData(*caseData);
        }
        m_moveableWallForm->exec();
        if(!m_moveableWallForm->confirmed())
        {
            m_wallMoveableMode = false;
            m_memWallSelectLayout->uncheckMoveableWall();
            return;
        }
    }
    bool draw = setWallShape();
    if(m_wallMoveableMode)
    {
        if(draw)
        {
            m_memWallSelectLayout->uncheckMoveableWall();
            if(m_moveableWallForm->getTriggerType() == TriggerType_e::DISTANT_SWITCH)
            {
                m_currentElementType = LevelElement_e::TRIGGER;
                setLineSelectableEnabled(false);
                m_currentSelection = m_moveableWallForm->getCurrentTriggerAppearence();
                m_wallMoveableMode = false;
            }
            else if(m_moveableWallForm->getTriggerType() == TriggerType_e::GROUND)
            {
                m_currentElementType = LevelElement_e::GROUND_TRIGGER;
                setLineSelectableEnabled(false);
                m_wallMoveableMode = false;
            }
        }
    }
    m_displayPreview = false;
}

//======================================================================
void GridEditor::treatElementsDrawing()
{
    bool deleteMode = m_currentElementType == LevelElement_e::DELETE;
    QModelIndex caseIndex = ui->tableView->selectionModel()->selection().indexes()[0];
    if(m_currentElementType == LevelElement_e::PLAYER_DEPARTURE)
    {
        setColorCaseData(caseIndex.column(), caseIndex.row(), m_currentElementType);
        return;
    }
    if(m_currentElementType == LevelElement_e::GROUND_TRIGGER)
    {
        setColorCaseData(caseIndex.column(), caseIndex.row(), m_currentElementType);
        m_currentElementType = LevelElement_e::WALL;
        setLineSelectableEnabled(true);
        confNewTriggerData(caseIndex);
        return;
    }
    else if(m_currentElementType == LevelElement_e::TARGET_TELEPORT)
    {
        m_tableModel->setTargetTeleport(m_lastPositionAdded, caseIndex);
        m_currentElementType = LevelElement_e::TELEPORT;
        setLineSelectableEnabled(true);
        return;
    }
    else if(m_currentElementType == LevelElement_e::SELECTION)
    {
        treatSelection(caseIndex);
        return;
    }
    else if(deleteMode)
    {
        removeElementCase(caseIndex);
    }
    setCaseIcon(caseIndex.column(), caseIndex.row(), deleteMode);
    if(m_currentElementType == LevelElement_e::TELEPORT)
    {
        m_lastPositionAdded = {caseIndex.column(), caseIndex.row()};
        setLineSelectableEnabled(false);
        m_currentElementType = LevelElement_e::TARGET_TELEPORT;
    }
    else if(m_currentElementType == LevelElement_e::TRIGGER)
    {
        confNewTriggerData(caseIndex);
    }
}

//======================================================================
void GridEditor::removeElementCase(const QModelIndex &caseIndex)
{
    std::optional<CaseData> &caseData = m_tableModel->getDataElementCase(caseIndex);
    if(caseData && caseData->m_type == LevelElement_e::WALL && caseData->m_moveWallData->m_triggerPos)
    {
        QModelIndex triggerIndex = m_tableModel->index(caseData->m_moveWallData->m_triggerPos->second,
                                                       caseData->m_moveWallData->m_triggerPos->first);
        if(!triggerIndex.isValid())
        {
            return;
        }
        std::optional<CaseData> &triggerData = m_tableModel->getDataElementCase(triggerIndex);
        if(triggerData)
        {
            QSet<QPair<int, int>>::iterator it = triggerData->m_triggerLinkWall->find({caseIndex.column(), caseIndex.row()});
            assert(it != triggerData->m_triggerLinkWall->end());
            triggerData->m_triggerLinkWall->erase(it);
            caseData->m_moveWallData->m_triggerPos.reset();
            caseData->m_moveWallData.reset();
        }
    }
    else if(caseData && caseData->m_type == LevelElement_e::PLAYER_DEPARTURE)
    {
        m_PlayerDeparture = {};
    }
}

//======================================================================
void GridEditor::execConfCeilingBackground()
{
    m_backgroundForm->confCeilingOrGroundMode(true);
    m_backgroundForm->unckeckAll();
    m_backgroundForm->exec();
}

//======================================================================
void GridEditor::execConfGroundBackground()
{
    m_backgroundForm->unckeckAll();
    m_backgroundForm->confCeilingOrGroundMode(false);
    m_backgroundForm->exec();
}

//======================================================================
void GridEditor::confNewTriggerData(const QModelIndex &caseIndex)
{
    std::optional<CaseData> &triggerData = m_tableModel->getDataElementCase(caseIndex);
    assert(triggerData);
    std::optional<int> index = m_memWallSelectLayout->getSelected();
    assert(index);
    if(!triggerData->m_triggerLinkWall)
    {
        triggerData->m_triggerLinkWall = QSet<QPair<int, int>>();
    }
    setElementSelected(LevelElement_e::WALL, *index);
    setLineSelectableEnabled(true);
    QModelIndex wallIndex;
    //mem trigger pos for wall
    for(QSet<QPair<int, int>>::iterator it = m_memCurrentLinkTriggerWall.begin(); it != m_memCurrentLinkTriggerWall.end(); ++it)
    {
        triggerData->m_triggerLinkWall->insert(*it);
        wallIndex = m_tableModel->index(it->second, it->first);
        assert(m_tableModel->getDataElementCase(wallIndex));
        if(m_tableModel->getDataElementCase(wallIndex)->m_moveWallData)
        {
            m_tableModel->getDataElementCase(wallIndex)->m_moveWallData->m_triggerPos = {caseIndex.column(), caseIndex.row()};
        }
    }
}

//======================================================================
void GridEditor::treatSelection(const QModelIndex &caseIndex)
{
    std::optional<CaseData> var = m_tableModel->getDataElementCase(caseIndex);
    if(var)
    {
        if(var->m_type == LevelElement_e::TELEPORT)
        {
            assert(var->m_targetTeleport);
            m_tableModel->setPreviewCase(var->m_targetTeleport->first, var->m_targetTeleport->second);
        }
        else if(var->m_type == LevelElement_e::WALL)
        {
            for(int32_t i = 0; i < var->m_moveWallData->m_memMoveWallCases.size(); ++i)
            {
                m_tableModel->setPreviewCase(var->m_moveWallData->m_memMoveWallCases.operator[](i));
            }
        }
        else if(var->m_type == LevelElement_e::TRIGGER || var->m_type == LevelElement_e::GROUND_TRIGGER)
        {
            assert(var->m_triggerLinkWall);
            for(QSet<QPair<int, int>>::iterator it = var->m_triggerLinkWall->begin(); it != var->m_triggerLinkWall->end(); ++it)
            {
                m_tableModel->setPreviewCase(*it);
            }
        }
    }
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
}

//======================================================================
void GridEditor::setStdTableSize()
{
    QPair<int, int> sizeTable = m_tableModel->getTableSize();
    for(int32_t i = 0; i < sizeTable.first; ++i)
    {
        ui->tableView->setColumnWidth(i, CASE_SIZE_PX);
    }
    for(int32_t i = 0; i < sizeTable.second; ++i)
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
    case LevelElement_e::TARGET_TELEPORT:
        return "Target teleport";
    case LevelElement_e::WALL:
        return "Wall";
    case LevelElement_e::TRIGGER:
        return "Trigger";
    case LevelElement_e::GROUND_TRIGGER:
        return "Ground trigger";
    case LevelElement_e::DELETE:
        return "Delete";
    case LevelElement_e::PLAYER_DEPARTURE:
        return "Player Departure";
    case LevelElement_e::SELECTION:
        return "Selection";
    case LevelElement_e::TOTAL:
        assert(false);
    }
    return "";
}

//======================================================================
Direction_e getDirEnumFromQString(const QString &str)
{
    Direction_e dir = Direction_e::EAST;
    if(str == "NORTH")
    {
        dir = Direction_e::NORTH;
    }
    else if(str == "SOUTH")
    {
        dir = Direction_e::SOUTH;
    }
    else if(str == "EAST")
    {
        dir = Direction_e::EAST;
    }
    else if(str == "WEST")
    {
        dir = Direction_e::WEST;
    }
    return dir;
}
