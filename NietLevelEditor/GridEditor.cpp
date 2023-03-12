#include "GridEditor.hpp"
#include "ui_GridEditor.h"
#include <QFile>
#include <QStandardItem>
#include <QPainter>
#include <QRadioButton>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QMenuBar>
#include <QPushButton>
#include <QMessageBox>
#include <iostream>
#include <QDir>
#include <algorithm>
#include "TableModel.hpp"
#include "SelectableLineLayout.hpp"
#include "EventFilter.hpp"
#include "LineWallMove.hpp"
#include "BackgroundForm.hpp"
#include "CheckpointForm.hpp"
#include "LogForm.hpp"

//======================================================================
GridEditor::GridEditor(QWidget *parent) : QDialog(parent), ui(new Ui::GridEditor), m_eventFilter(new EventFilter(this)),
    m_checkpointForm(new CheckpointForm(this))
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowSystemMenuHint);
}

//======================================================================
bool GridEditor::loadMainInstallDirData(const QString &installDir)
{
    return m_levelDataManager.loadLevelData(installDir);
}

//======================================================================
bool GridEditor::loadExistingLevelINI(const QString &levelFilePath)
{
    return m_levelDataManager.loadExistingLevel(levelFilePath);
}

//======================================================================
void GridEditor::initGrid(const QString &installDir, int levelWidth, int levelHeight)
{
    m_memDoorVertical.clear();
    m_wallMoveableMode = false;
    m_doorCardMode = false;
    m_wallDrawMode = WallDrawShape_e::LINE_AND_RECT;
    m_elementSelected = false;
    m_installDir = installDir;
    loadIconPictures(installDir);
    if(!m_moveableWallForm)
    {
        m_moveableWallForm = new MoveableWallForm(this);
        m_moveableWallForm->setTriggerIcons(m_drawData[static_cast<int>(LevelElement_e::TRIGGER)]);
    }
    if(!m_logForm)
    {
        m_logForm = new LogForm();
    }
    if(!m_backgroundForm)
    {
        loadSpritesForBackgroundForm();
    }
    if(!m_tableModel)
    {
        m_tableModel = new TableModel(ui->tableView);
    }
    else
    {
        m_tableModel->clearModel();
    }
    m_tableModel->setLevelSize(levelWidth, levelHeight);
    ui->tableView->reset();
    ui->tableView->setModel(m_tableModel);
    setStdTableSize();
    m_displayPreview = false;
    if(!m_widgetInit)
    {
        initSelectableWidgets();
        initButtons();
        connectSlots();
    }
    initMusicDir(installDir, m_widgetInit);
    m_widgetInit = true;
}

//======================================================================
std::optional<QPair<int, int>> GridEditor::getLoadedLevelSize()const
{
    return m_levelDataManager.getLoadedLevelSize();
}

//======================================================================
bool GridEditor::loadExistingLevelGrid()
{
    LevelData const *existingLevel = m_levelDataManager.getExistingLevel();
    if(!existingLevel)
    {
        return false;
    }
    QModelIndex caseIndex = m_tableModel->index(m_levelDataManager.getExistingLevel()->m_playerDeparture.second,
                                                m_levelDataManager.getExistingLevel()->m_playerDeparture.first, QModelIndex());
    m_tableModel->setPlayerDirectionDeparture(m_memPlayerDirection);
    if(m_levelDataManager.getExistingLevel()->m_music)
    {
        for(int i = 0; i < m_musicWidget->count(); ++i)
        {
            if(m_musicWidget->itemText(i) == *m_levelDataManager.getExistingLevel()->m_music)
            {
                m_musicWidget->setCurrentIndex(i);
                break;
            }
        }
    }
    m_memPlayerDirection = existingLevel->m_playerDirection;
    m_memPlayerDepartureWidget->setCurrentIndex(static_cast<uint32_t>(existingLevel->m_playerDirection));
    setColorElement(caseIndex, LevelElement_e::PLAYER_DEPARTURE);
    if(!loadWallExistingLevelGrid())
    {
        return false;
    }
    loadBackgroundGeneralExistingLevelGrid();
    loadTeleportExistingLevelGrid();
    loadSecretsExistingLevelGrid();
    loadCheckpointsExistingLevelGrid();
    loadLogsExistingLevelGrid();
    loadStandardExistingLevelGrid(LevelElement_e::BARREL);
    loadStandardExistingLevelGrid(LevelElement_e::DOOR);
    loadStandardExistingLevelGrid(LevelElement_e::ENEMY);
    loadStandardExistingLevelGrid(LevelElement_e::STATIC_CEILING);
    loadStandardExistingLevelGrid(LevelElement_e::STATIC_GROUND);
    loadStandardExistingLevelGrid(LevelElement_e::EXIT);
    loadStandardExistingLevelGrid(LevelElement_e::OBJECT);
    updateGridView();
    updateCheckpointDisplay();
    return true;
}

//======================================================================
void GridEditor::unselectAllRadioButtons()
{
    const QObjectList &children = ui->SelectableLayout->children();
    //-1 ignore music select layout
    for(int i = 0; i < children.size() - 1; ++i)
    {
        static_cast<SelectableLineLayout*>(children[i])->uncheckRadioButton();
    }
    m_currentSelection = 0;
    m_currentElementType = LevelElement_e::SELECTION;
    m_memWallSelectLayout->reinitWallComp();
}

//======================================================================
void GridEditor::connectSlots()
{
    QObject::connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged,
                     this, &GridEditor::stdElementCaseSelectedChanged);
    ui->tableView->viewport()->installEventFilter(m_eventFilter);
    QObject::connect(ui->tableView, &QAbstractItemView::pressed, this, &GridEditor::wallSelection);
    QObject::connect(m_eventFilter, &EventFilter::mouseReleased, this, &GridEditor::mouseReleaseSelection);
}

//======================================================================
void GridEditor::setCaseIcon(int x, int y, int wallShapeNum, bool deleteMode, bool dontMemRemovedWall, QPixmap *cardDoorCase)
{
    bool endLevelEnemyCase = (m_currentElementType == LevelElement_e::ENEMY &&
                              m_memFinishLevelEnemySelectLayout->isEndLevelEnemyChecked());
    QModelIndex index = m_tableModel->index(y, x, QModelIndex());
    std::optional<CaseData> &caseData = m_tableModel->getDataElementCase(index);
    LevelElement_e type = caseData ? LevelElement_e::TOTAL : caseData->m_type;
    if(type == LevelElement_e::TRIGGER || type == LevelElement_e::GROUND_TRIGGER)
    {
        if(m_currentElementType == LevelElement_e::TRIGGER || m_currentElementType == LevelElement_e::GROUND_TRIGGER)
        {
            return;
        }
        m_tableModel->removeTrigger(*caseData, {x, y});
    }
    else
    {
        m_tableModel->removeData(index, dontMemRemovedWall);
    }
    if(deleteMode)
    {
        return;
    }
    //end level enemy
    if(endLevelEnemyCase)
    {
        bool ok = m_tableModel->setData(index, QVariant(getColoredBackgroundIcon()));
        assert(ok);
    }
    else
    {
        bool ok;
        if(cardDoorCase)
        {
            assert(m_currentElementType == LevelElement_e::DOOR);
            ok = m_tableModel->setData(index, QVariant(QIcon(*cardDoorCase).
                                                       pixmap({CASE_SPRITE_SIZE, CASE_SPRITE_SIZE})));
        }
        else
        {
            ok = m_tableModel->setData(index, QVariant(getCurrentSelectedIcon().
                                                            pixmap({CASE_SPRITE_SIZE, CASE_SPRITE_SIZE})));
        }
        assert(ok);
    }
    if(!caseData || (type != LevelElement_e::TRIGGER && type != LevelElement_e::GROUND_TRIGGER))
    {
        m_tableModel->setIdData(index, CaseData{m_currentElementType,
                                                m_mapElementID[m_currentElementType][m_currentSelection], {}, {}, {}, {}}, endLevelEnemyCase);
    }
    if(endLevelEnemyCase)
    {
        m_memFinishLevelEnemySelectLayout->uncheckCheckBox();
    }
    else if(m_currentElementType == LevelElement_e::WALL)
    {
        caseData->m_wallShapeNum = wallShapeNum;
        if(m_wallMoveableMode)
        {
            if((!m_loadingExistingLevelMode && (m_moveableWallForm->getTriggerType() == TriggerType_e::DISTANT_SWITCH ||
                    m_moveableWallForm->getTriggerType() == TriggerType_e::GROUND)) ||
                    (m_loadingExistingLevelMode && m_loadingDistantTriggerMode))
            {
                m_memCurrentLinkTriggerWall.insert({x, y});
            }
            memWallMove(index);
        }
    }
    else
    {
        caseData->m_wallShapeNum = {};
    }
}

//======================================================================
QPixmap GridEditor::getColoredBackgroundIcon()
{
    QPixmap pix(CASE_SPRITE_SIZE, CASE_SPRITE_SIZE);
    QPainter paint(&pix);
    pix.fill(Qt::darkMagenta);
    paint.drawPixmap(0,0,CASE_SPRITE_SIZE,CASE_SPRITE_SIZE, getCurrentSelectedIcon().
                     pixmap({CASE_SPRITE_SIZE, CASE_SPRITE_SIZE}));
    return pix;
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
void GridEditor::setColorCaseData(int x, int y, LevelElement_e type, const QPair<uint32_t, Direction_e> &checkpointData)
{
    QModelIndex index = m_tableModel->index(y, x, QModelIndex());
    QPixmap pix(CASE_SPRITE_SIZE, CASE_SPRITE_SIZE);
    QPainter paint(&pix);
    QString text;
    paint.setPen(Qt::black);
    paint.setFont(QFont("Arial", 12));
    if(type == LevelElement_e::PLAYER_DEPARTURE)
    {
        pix.fill(Qt::cyan);
        text = "D" + getStrDir(m_memPlayerDirection);
        m_tableModel->setPlayerDirectionDeparture(m_memPlayerDirection);
    }
    else if(type == LevelElement_e::GROUND_TRIGGER)
    {
        text = "GT";
        pix.fill(Qt::magenta);
    }
    else if(type == LevelElement_e::CHECKPOINT)
    {
        pix.fill(Qt::darkGray);
        text = getStrCheckpoint(checkpointData);
        m_tableModel->setIdData(index, CaseData{type, "", {}, {}, {}, {}});
        m_tableModel->addCheckpoint({x, y}, checkpointData);
    }
    else if(type == LevelElement_e::SECRET)
    {
        text = "S";
        pix.fill(Qt::darkRed);
        m_tableModel->setIdData(index, CaseData{type, "", {}, {}, {}, {}});
        m_tableModel->addSecret({x, y});
    }
    if(type == LevelElement_e::PLAYER_DEPARTURE || !m_tableModel->getDataElementCase(index))
    {
        m_tableModel->setIdData(index, CaseData{type, "", {}, {}, {}, {}});
    }
    paint.drawText(QRect(0, 0, CASE_SPRITE_SIZE, CASE_SPRITE_SIZE), Qt::AlignCenter, text);
    m_tableModel->setData(index, QVariant(pix));
    updateGridView();
}

//======================================================================
void GridEditor::updateCheckpointDisplay()
{
    QString text;
    QPixmap pix(CASE_SPRITE_SIZE, CASE_SPRITE_SIZE);
    QPainter paint(&pix);
    paint.setPen(Qt::black);
    paint.setFont(QFont("Arial", 12));
    const QVector<QPair<QPair<int, int>, Direction_e>> checkpoint = m_tableModel->getCheckpointsData();
    QModelIndex index;
    for(int i = 0; i < checkpoint.size(); ++i)
    {
        pix.fill(Qt::darkGray);
        text = getStrCheckpoint({i, checkpoint[i].second});
        index = m_tableModel->index(checkpoint[i].first.second, checkpoint[i].first.first, QModelIndex());
        paint.drawText(QRect(0, 0, CASE_SPRITE_SIZE, CASE_SPRITE_SIZE), Qt::AlignCenter, text);
        m_tableModel->setData(index, QVariant(pix));
    }
}

//======================================================================
QIcon GridEditor::getCurrentSelectedIcon()const
{
    uint32_t index = static_cast<uint32_t>(m_currentElementType);
    assert(index < m_drawData.size());
    assert(m_currentSelection < m_drawData[index].size());
    return m_drawData[index][m_currentSelection].m_icon;
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
    //-1 ignore music select layout
    for(int i = 0; i < children.size() - 1; ++i)
    {
        static_cast<SelectableLineLayout*>(children[i])->setRadioButtonEnabled(enable);
    }
    m_generateButton->setEnabled(enable);
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
        else if(currentEnum == LevelElement_e::PLAYER_DEPARTURE)
        {
            m_memPlayerDepartureWidget = selectLayout->confPlayerDeparture(this);
        }
        else if(currentEnum == LevelElement_e::DOOR)
        {
            m_memDoorSelectLayout = selectLayout;
            selectLayout->confDoorSelectWidget(this, m_cardIcons);
        }
        else if(currentEnum == LevelElement_e::ENEMY)
        {
            m_memFinishLevelEnemySelectLayout = selectLayout;
            selectLayout->confEnemySelectWidget(this);
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
    button = new QPushButton("Generate Level");
    ui->SelectableLayout->addWidget(button);
    QObject::connect(button, &QPushButton::clicked, this, &GridEditor::generateLevel);
    m_generateButton = button;
}

//======================================================================
void GridEditor::initMusicDir(const QString &installDir, bool widgetInit)
{
    m_musicWidget = new QComboBox();
    QString musicDir = installDir + "/Ressources/Audio/Music/";
    QDir dir(musicDir);
    assert(dir.exists());
    QFileInfoList list = dir.entryInfoList();
    QFileInfo fileInfo;
    m_musicWidget->clear();
    m_musicWidget->addItem("None");
    for(int i = 0; i < list.size(); ++i)
    {
        fileInfo = list[i];
        if(fileInfo.suffix() == "flac")
        {
            m_musicWidget->addItem(fileInfo.fileName());
        }
    }
    if(!widgetInit)
    {
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(new QLabel("Music"));
        layout->addWidget(m_musicWidget);
        layout->setAlignment(Qt::AlignmentFlag::AlignTop);
        ui->SelectableLayout->addLayout(layout);
    }
}

//======================================================================
void GridEditor::loadIconPictures(const QString &installDir)
{
    m_mapElementID.clear();
    loadWallsPictures(installDir);
    loadDoorsPictures();
    loadCardPictures(installDir);
    loadStandardPictures(installDir, LevelElement_e::LOG);
    loadStandardPictures(installDir, LevelElement_e::TELEPORT);
    loadStandardPictures(installDir, LevelElement_e::ENEMY);
    loadStandardPictures(installDir, LevelElement_e::OBJECT);
    loadStandardPictures(installDir, LevelElement_e::TRIGGER);
    loadStandardPictures(installDir, LevelElement_e::STATIC_GROUND);
    loadStandardPictures(installDir, LevelElement_e::STATIC_CEILING);
    loadStandardPictures(installDir, LevelElement_e::BARREL);
    loadStandardPictures(installDir, LevelElement_e::EXIT);
}

//======================================================================
void GridEditor::loadSpritesForBackgroundForm()
{
    const std::map<QString, ArrayFloat_t> &picData = m_levelDataManager.getSpriteData();
    m_backgroundForm = new BackgroundForm(picData);
}

//======================================================================
void GridEditor::memPlayerDirection(int direction)
{
    assert(direction < 4);
    m_memPlayerDirection = static_cast<Direction_e>(direction);
}

//======================================================================
void GridEditor::setCardDoorMode(int active)
{
    m_doorCardMode = active;
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
            m_drawData[currentIndex].push_back({it->first, it->second[0], getSprite(*spriteData, m_levelDataManager, installDir)});
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
            m_drawData[currentIndex].push_back({it->first, it->second[0], final});
        }
    }
}

//======================================================================
void GridEditor::loadDoorsPictures()
{
    const std::map<QString, DoorData> &doorsMap = m_levelDataManager.getDoorData();
    uint32_t currentIndex = static_cast<uint32_t>(LevelElement_e::DOOR);
    m_drawData[currentIndex].reserve(doorsMap.size());
    std::optional<ArrayFloat_t> spriteData;
    m_mapElementID.insert({LevelElement_e::DOOR, QVector<QString>()});
    m_mapElementID[LevelElement_e::DOOR].reserve(doorsMap.size());
    for(std::map<QString, DoorData>::const_iterator it = doorsMap.begin(); it != doorsMap.end(); ++it)
    {
        m_mapElementID[LevelElement_e::DOOR].push_back(it->first);
        spriteData = m_levelDataManager.getPictureData(it->second.m_sprite);
        assert(spriteData);
        m_memDoorVertical.push_back(it->second.m_vertical);
        QPixmap final = getDoorPixmap(it->second.m_vertical, *spriteData, it->second.m_cardID);
        m_drawData[currentIndex].push_back({it->first, it->second.m_sprite, final});
    }
}

//======================================================================
QPixmap GridEditor::getDoorPixmap(bool vertical, const ArrayFloat_t &doorSpriteData, std::optional<QString> cardId)
{
    QPixmap final(CASE_SPRITE_SIZE, CASE_SPRITE_SIZE);
    final.fill(Qt::transparent);
    QPainter paint(&final);
    QPixmap baseDoorSprite = getSprite(doorSpriteData, m_levelDataManager, m_installDir);
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
    if(cardId)
    {
        std::cerr << cardId->toStdString() << "\n";
        std::optional<ArrayFloat_t> cardSpriteData = m_levelDataManager.getPictureData(*cardId);
        assert(cardSpriteData);
        QPixmap cardSprite = getSprite(*cardSpriteData, m_levelDataManager, m_installDir);
        paint.drawPixmap(CASE_SPRITE_SIZE / 5 * 4, CASE_SPRITE_SIZE / 10,
                         CASE_SPRITE_SIZE / 4, CASE_SPRITE_SIZE / 3, cardSprite);
    }
    return final;
}

//======================================================================
void GridEditor::loadCardPictures(const QString &installDir)
{
    std::optional<ArrayFloat_t> spriteData;
    const std::map<QString, QString> &cardsMap = m_levelDataManager.getCardData();
    m_cardIcons.reserve(cardsMap.size());
    for(std::map<QString, QString>::const_iterator it = cardsMap.begin(); it != cardsMap.end(); ++it)
    {
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_cardIcons.push_back({it->first, it->second[0], getSprite(*spriteData, m_levelDataManager, installDir)});
    }
}

//======================================================================
void GridEditor::loadStandardPictures(const QString &installDir, LevelElement_e elementType)
{
    const std::map<QString, QString> *currentMap;
    switch(elementType)
    {
    case LevelElement_e::TELEPORT:
        currentMap = &m_levelDataManager.getTeleportData();
        break;
    case LevelElement_e::ENEMY:
        currentMap = &m_levelDataManager.getEnemyData();
        break;
    case LevelElement_e::OBJECT:
        currentMap = &m_levelDataManager.getObjectData();
        break;
    case LevelElement_e::STATIC_CEILING:
        currentMap = &m_levelDataManager.getStaticCeilingData();
        break;
    case LevelElement_e::STATIC_GROUND:
        currentMap = &m_levelDataManager.getStaticGroundData();
        break;
    case LevelElement_e::BARREL:
        currentMap = &m_levelDataManager.getBarrelData();
        break;
    case LevelElement_e::LOG:
        currentMap = &m_levelDataManager.getLogData();
        break;
    case LevelElement_e::TRIGGER:
        currentMap = &m_levelDataManager.getTriggerData();
        break;
    case LevelElement_e::EXIT:
        currentMap = &m_levelDataManager.getExitData();
        break;
    case LevelElement_e::WALL:
    case LevelElement_e::DOOR:
    case LevelElement_e::PLAYER_DEPARTURE:
    case LevelElement_e::CHECKPOINT:
    case LevelElement_e::SECRET:
    case LevelElement_e::GROUND_TRIGGER:
    case LevelElement_e::TARGET_TELEPORT:
    case LevelElement_e::SELECTION:
    case LevelElement_e::DELETE:
    case LevelElement_e::TOTAL:
        assert(false);
        break;
    }
    uint32_t currentIndex = static_cast<uint32_t>(elementType);
    m_drawData[currentIndex].reserve(currentMap->size());
    std::optional<ArrayFloat_t> spriteData;
    m_mapElementID.insert({elementType, QVector<QString>()});
    m_mapElementID[elementType].reserve(currentMap->size());
    for(std::map<QString, QString>::const_iterator it = currentMap->begin(); it != currentMap->end(); ++it)
    {
        m_mapElementID[elementType].push_back(it->first);
        spriteData = m_levelDataManager.getPictureData(it->second);
        assert(spriteData);
        m_drawData[currentIndex].push_back({it->first, it->second, getSprite(*spriteData, m_levelDataManager, installDir)});
    }
}

//======================================================================
bool GridEditor::setWallShape(bool preview, bool loadFromIni)
{
    if(preview)
    {
        if(!m_displayPreview ||
                ui->tableView->selectionModel()->selection().indexes().empty() || m_firstCaseSelection.column() == -1)
        {
            return false;
        }
        m_secondCaseSelection = ui->tableView->selectionModel()->selection().indexes()[0];
    }
    int minX = std::min(m_firstCaseSelection.column(), m_secondCaseSelection.column()),
            maxX = std::max(m_firstCaseSelection.column(), m_secondCaseSelection.column()),
            minY = std::min(m_firstCaseSelection.row(), m_secondCaseSelection.row()),
            maxY = std::max(m_firstCaseSelection.row(), m_secondCaseSelection.row());
    if(preview)
    {
        m_tableModel->clearPreview();
    }
    if(!loadFromIni)
    {
        memStdWallMove();
    }
    int shapeNum = -1;
    QPair<int, int> topLeftPos = {minX, minY}, bottomRight = {maxX, maxY};
    if(!preview)
    {
        int index = static_cast<int>(m_currentElementType);
        const MoveWallData *memMoveData = m_wallMoveableMode ? &(*m_memcurrentMoveWallData) : nullptr;
        shapeNum = m_tableModel->memWallShape(m_wallDrawMode, topLeftPos, bottomRight,
                                              m_drawData[index][m_currentSelection].m_elementSectionName, memMoveData);
        if(topLeftPos == bottomRight)
        {
            setCaseIcon(minX, minY, shapeNum, false, true);
            m_tableModel->updateWallNumber(1);
            return true;
        }
    }
    bool ret = true;
    uint32_t wallNumber;
    switch (m_wallDrawMode)
    {
    case WallDrawShape_e::LINE_AND_RECT:
    {
        wallNumber = setWallLineRectShape(topLeftPos, bottomRight, shapeNum, preview);
    }
        break;
    case WallDrawShape_e::DIAGONAL_LINE:
    {
        wallNumber = setWallDiagLineShape(topLeftPos, bottomRight, shapeNum, preview);
    }
        break;
    case WallDrawShape_e::DIAGONAL_RECT:
    {
        ret = setWallDiagRectShape(topLeftPos, bottomRight, shapeNum, wallNumber, preview);
    }
        break;
    }
    if(!preview)
    {
        m_tableModel->updateWallNumber(wallNumber);
        if(m_wallDrawMode == WallDrawShape_e::DIAGONAL_LINE)
        {
            setWallDiagCaseConf();
        }
        else if(m_wallDrawMode == WallDrawShape_e::DIAGONAL_RECT)
        {
            setWallDiagRectCaseConf();
        }
    }
    //quick fix
    updateGridView();
    return ret;
}

//======================================================================
void GridEditor::setWallDiagCaseConf()
{
    int totalX = std::abs(m_firstCaseSelection.column() - m_secondCaseSelection.column()),
            totalY = std::abs(m_firstCaseSelection.row() - m_secondCaseSelection.row());
    int size = std::min(totalX, totalY);
    int originX, originY;
    bool directionUp;
    if(m_firstCaseSelection.column() < m_secondCaseSelection.column())
    {
        originX = m_firstCaseSelection.column();
        originY = m_firstCaseSelection.row();
        //reverse Y axe
        directionUp = m_firstCaseSelection.row() > m_secondCaseSelection.row();
    }
    else
    {
        directionUp = m_firstCaseSelection.row() < m_secondCaseSelection.row();
        if(totalX == size)
        {
            originX = m_secondCaseSelection.column();
        }
        else
        {
            originX = m_firstCaseSelection.column() - size;
        }
        if(totalY == size)
        {
            originY = m_secondCaseSelection.row();
        }
        else
        {
            if(m_firstCaseSelection.row() < m_secondCaseSelection.row())
            {
                originY = m_firstCaseSelection.row() + size;
            }
            else
            {
                originY = m_firstCaseSelection.row() - size;
            }
        }
    }
    m_tableModel->setTableWallDiagCaseConf({originX, originY}, directionUp);
}

//======================================================================
void GridEditor::setWallDiagRectCaseConf()
{
    int size = m_tableModel->getLastWallCount() / 2;
    QPair<int, int> originPoint;
    if(m_firstCaseSelection.column() < m_secondCaseSelection.column())
    {
        originPoint.first = m_firstCaseSelection.column();
    }
    else
    {
        originPoint.first = m_firstCaseSelection.column() - size;
    }
    if(m_firstCaseSelection.row() < m_secondCaseSelection.row())
    {
        originPoint.second = m_firstCaseSelection.row();
    }
    else
    {
        originPoint.second = m_firstCaseSelection.row() - size;
    }
    m_tableModel->setTableWallDiagRectCaseConf(originPoint);
}

//======================================================================
void GridEditor::setDeletionZone(bool preview)
{
    if(ui->tableView->selectionModel()->selection().indexes().isEmpty() || m_firstCaseSelection.column() == -1)
    {
        return;
    }
    m_secondCaseSelection = ui->tableView->selectionModel()->selection().indexes()[0];
    m_tableModel->setTableDeletionZone({m_firstCaseSelection.column(), m_firstCaseSelection.row()},
                                       {m_secondCaseSelection.column(), m_secondCaseSelection.row()}, preview);
    updateGridView();
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
    m_memcurrentMoveWallData->m_triggerINISectionName = m_moveableWallForm->getCurrentTriggerINISection();
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
uint32_t GridEditor::setWallLineRectShape(const QPair<int, int> &topLeftIndex,
                                          const QPair<int, int> &bottomRightIndex,
                                          int shapeNum, bool preview, bool deleteMode)
{
    uint32_t count = 0;
    for(int i = topLeftIndex.first; i < bottomRightIndex.first + 1; ++i)
    {
        if(preview)
        {
            m_tableModel->setPreviewCase(i, topLeftIndex.second);
            m_tableModel->setPreviewCase(i, bottomRightIndex.second);
        }
        else
        {
            setCaseIcon(i, topLeftIndex.second, shapeNum, deleteMode, true);
            ++count;
            if(topLeftIndex.second != bottomRightIndex.second)
            {
                setCaseIcon(i, bottomRightIndex.second, shapeNum, deleteMode, true);
                ++count;
            }
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
            setCaseIcon(topLeftIndex.first, i, shapeNum, deleteMode, true);
            ++count;
            if(topLeftIndex.first != bottomRightIndex.first)
            {
                setCaseIcon(bottomRightIndex.first, i, shapeNum, deleteMode, true);
                ++count;
            }
        }
    }
    return count;
}

//======================================================================
uint32_t GridEditor::setWallDiagLineShape(const QPair<int, int> &topLeftIndex,
                                          const QPair<int, int> &bottomRightIndex, int shapeNum, bool preview, bool deleteMode)
{
    uint32_t count = 0;
    int j = m_firstCaseSelection.row(), modY, i = m_firstCaseSelection.column();
    modY = (topLeftIndex.second == m_firstCaseSelection.row()) ? 1 : -1;
    int modX = (topLeftIndex.first == m_firstCaseSelection.column()) ? 1 : -1;
    for(;((modY == 1 && j < bottomRightIndex.second + 1) || (modY == -1 && j > topLeftIndex.second - 1)) &&
        ((modX == 1 && i < bottomRightIndex.first + 1) || (modX == -1 && i > topLeftIndex.first - 1)); i += modX, j += modY)
    {
        if(preview)
        {
            m_tableModel->setPreviewCase(i, j);
        }
        else
        {
            setCaseIcon(i, j, shapeNum, deleteMode, true);
            ++count;
        }
    }
    return count;
}

//======================================================================
bool GridEditor::setWallDiagRectShape(const QPair<int, int> &topLeftIndex,
                                      const QPair<int, int> &bottomRightIndex, int shapeNum, uint32_t &wallNumber,
                                      bool preview, bool deleteMode)
{
    wallNumber = 0;
    bool modX = (topLeftIndex.first != m_firstCaseSelection.column()),
            modY = (topLeftIndex.second != m_firstCaseSelection.row());
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
        bottomRightIndexCpy.first = m_firstCaseSelection.column();
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
            setCaseIcon(i, currentYA, shapeNum, deleteMode, true);
            setCaseIcon(i, currentYB, shapeNum, deleteMode, true);
            wallNumber += 2;
        }
    }
    --wallNumber;
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
            setCaseIcon(i, currentYA, shapeNum, deleteMode, true);
            setCaseIcon(i, currentYB, shapeNum, deleteMode, true);
            wallNumber += 2;
        }
    }
    wallNumber -= 3;
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
        m_memWallSelectLayout->uncheckCheckBox();
    }
    if(num != LevelElement_e::ENEMY)
    {
        m_memFinishLevelEnemySelectLayout->uncheckCheckBox();
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
    else if(m_currentElementType == LevelElement_e::DELETE)
    {
        setDeletionZone(true);
    }
}

//======================================================================
void GridEditor::wallSelection(const QModelIndex &index)
{
    if(m_currentElementType != LevelElement_e::WALL && m_currentElementType != LevelElement_e::DELETE)
    {
        return;
    }
    m_firstCaseSelection = index;
    m_secondCaseSelection = index;
    m_displayPreview = true;
}

//======================================================================
void GridEditor::mouseReleaseSelection()
{
    m_tableModel->clearPreview();
    if(!m_elementSelected ||
            ((m_currentElementType == LevelElement_e::WALL || m_currentElementType == LevelElement_e::DELETE) &&
             (m_firstCaseSelection.row() < 0 || m_firstCaseSelection.column() < 0)))
    {
        return;
    }
    if(m_currentElementType == LevelElement_e::WALL)
    {
        assert(ui->tableView->selectionModel()->selection().indexes().size() == 1);
        m_secondCaseSelection = ui->tableView->selectionModel()->selection().indexes()[0];
        treatWallDrawing();
    }
    else if(m_currentElementType == LevelElement_e::DOOR)
    {
        treatDoorDrawing();
    }
    else if(m_currentElementType == LevelElement_e::DELETE)
    {
        m_secondCaseSelection = ui->tableView->selectionModel()->selection().indexes()[0];
        m_tableModel->setTableDeletionZone({m_firstCaseSelection.column(), m_firstCaseSelection.row()},
                                           {m_secondCaseSelection.column(), m_secondCaseSelection.row()}, false);
    }
    else
    {
        treatElementsDrawing();
    }
    m_firstCaseSelection = m_tableModel->index(-1, -1, QModelIndex());
    m_secondCaseSelection = m_tableModel->index(-1, -1, QModelIndex());
    updateCheckpointDisplay();
    updateGridView();
}

//======================================================================
void GridEditor::treatWallDrawing()
{
    m_memCurrentLinkTriggerWall.clear();
    if(m_wallMoveableMode)
    {
        m_moveableWallForm->init();
        const std::optional<CaseData> &caseData = m_tableModel->getDataElementCase(m_firstCaseSelection);
        if(caseData && caseData->m_type == LevelElement_e::WALL && caseData->m_moveWallData)
        {
            m_moveableWallForm->setData(*caseData);
        }
        m_moveableWallForm->exec();
        if(!m_moveableWallForm->confirmed())
        {
            m_wallMoveableMode = false;
            m_memWallSelectLayout->uncheckCheckBox();
            return;
        }
    }
    bool draw = setWallShape();
    if(m_wallMoveableMode)
    {
        if(draw)
        {
            m_memWallSelectLayout->uncheckCheckBox();
            if(m_moveableWallForm->getTriggerType() == TriggerType_e::DISTANT_SWITCH &&
                    m_moveableWallForm->getTriggerBehaviour() != TriggerBehaviourType_e::AUTO)
            {
                m_currentElementType = LevelElement_e::TRIGGER;
                setLineSelectableEnabled(false);
                m_currentSelection = m_moveableWallForm->getCurrentTriggerAppearence();
                m_wallMoveableMode = false;
            }
            else if(m_moveableWallForm->getTriggerType() == TriggerType_e::GROUND &&
                    m_moveableWallForm->getTriggerBehaviour() != TriggerBehaviourType_e::AUTO)
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
void GridEditor::treatDoorDrawing()
{
    if(m_doorCardMode)
    {
        std::optional<QString> cardID;
        m_memDoorSelectLayout->uncheckCheckBoxDoor();
        const DisplayData &cardData = m_memDoorSelectLayout->getSelectedCardDoor();
        if(cardData.m_elementSectionName == "ObjectPurpleCard")
        {
            cardID = "SpriteSimetraCardPurple";
        }
        else if(cardData.m_elementSectionName == "ObjectBlueCard")
        {
            cardID = "SpriteSimetraCardBlue";
        }
        else if(cardData.m_elementSectionName == "ObjectGreenCard")
        {
            cardID = "SpriteSimetraCardGreen";
        }
        else if(cardData.m_elementSectionName == "ObjectGoldCard")
        {
            cardID = "SpriteSimetraCardGold";
        }
        else
        {
            assert(false);
        }
        QIcon iconBase = getCurrentSelectedIcon();
        QString baseSpriteName = m_drawData[static_cast<uint32_t>(LevelElement_e::DOOR)][m_currentSelection].m_spriteName;
        std::optional<ArrayFloat_t> spriteData = m_levelDataManager.getPictureData(baseSpriteName);
        assert(spriteData);
        QPixmap final = getDoorPixmap(m_memDoorVertical[m_currentSelection], *spriteData, cardID);
        QModelIndex caseIndex = ui->tableView->selectionModel()->selection().indexes()[0];
        setCaseIcon(caseIndex.column(), caseIndex.row(), -1, false, false, &final);
        m_tableModel->memStdElement({caseIndex.column(), caseIndex.row()}, m_currentElementType,
                                    m_drawData[static_cast<uint32_t>(LevelElement_e::DOOR)][m_currentSelection].m_elementSectionName);
    }
    else
    {
        treatElementsDrawing();
    }
}

//======================================================================
void GridEditor::treatElementsDrawing()
{
    bool deleteMode = m_currentElementType == LevelElement_e::DELETE;
    QModelIndex caseIndex = ui->tableView->selectionModel()->selection().indexes()[0];
    int index = static_cast<int>(m_currentElementType);
    if(m_currentElementType == LevelElement_e::PLAYER_DEPARTURE || m_currentElementType == LevelElement_e::CHECKPOINT ||
            m_currentElementType == LevelElement_e::SECRET)
    {
        setColorElement(caseIndex, m_currentElementType);
        return;
    }
    else if(m_currentElementType == LevelElement_e::GROUND_TRIGGER)
    {
        std::optional<CaseData> data = m_tableModel->getDataElementCase(caseIndex);
        if(data && data->m_type == LevelElement_e::CHECKPOINT)
        {
            m_tableModel->removeData(caseIndex);
        }
        setColorCaseData(caseIndex.column(), caseIndex.row(), m_currentElementType);
        m_currentElementType = LevelElement_e::WALL;
        setLineSelectableEnabled(true);
        confNewTriggerData(caseIndex);
        return;
    }
    else if(m_currentElementType == LevelElement_e::TARGET_TELEPORT)
    {
        setTargetTeleport(caseIndex);
        return;
    }
    else if(m_currentElementType == LevelElement_e::SELECTION)
    {
        treatSelection(caseIndex);
        return;
    }
    else if(m_currentElementType == LevelElement_e::LOG)
    {
        m_logForm->reinit();
        std::optional<CaseData> &caseData = m_tableModel->getDataElementCase(caseIndex);
        if(caseData && caseData->m_type == LevelElement_e::LOG)
        {
            std::optional<LogData> logData = m_tableModel->getLogDataPos({caseIndex.column(), caseIndex.row()});
            assert(logData);
            m_logForm->setMessage(logData->m_message);
        }
        m_logForm->exec();
        if(!m_logForm->valid())
        {
            return;
        }
        m_tableModel->addLog({caseIndex.column(), caseIndex.row()}, m_logForm->getMessage(), m_mapElementID[m_currentElementType][m_currentSelection]);
    }
    setCaseIcon(caseIndex.column(), caseIndex.row(), -1, deleteMode);
    if(!deleteMode)
    {
        m_tableModel->memStdElement({caseIndex.column(), caseIndex.row()}, m_currentElementType,
                                    m_drawData[index][m_currentSelection].m_elementSectionName);
    }
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
void GridEditor::setTargetTeleport(const QModelIndex &caseIndex)
{
    m_tableModel->setTargetTeleport(m_lastPositionAdded, caseIndex);
    m_currentElementType = LevelElement_e::TELEPORT;
    setLineSelectableEnabled(true);
    int teleportIndex = static_cast<int>(LevelElement_e::TELEPORT);
    m_tableModel->memTeleportElement(m_lastPositionAdded, {caseIndex.column(), caseIndex.row()},
                                     m_drawData[teleportIndex][m_currentSelection].m_elementSectionName);
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
    m_backgroundForm->confCeilingOrGroundMode(false);
    m_backgroundForm->unckeckAll();
    m_backgroundForm->exec();
}

//======================================================================
void GridEditor::generateLevel()
{
    if(!m_backgroundForm->backgroundSetted())
    {
        QMessageBox::warning(nullptr, "Error", "Ground OR/AND Ceiling background are not been setted.");
        return;
    }
    m_levelDataManager.generateLevel(*m_tableModel, m_musicWidget->currentText(),
                                     {&m_backgroundForm->getGroundData(), &m_backgroundForm->getCeilingData()}, m_memPlayerDirection);
}

//======================================================================
void GridEditor::confNewTriggerData(const QModelIndex &caseIndex)
{
    std::optional<CaseData> &triggerData = m_tableModel->getDataElementCase(caseIndex);
    if(!triggerData)
    {
        return;
    }
    std::optional<int> index = m_memWallSelectLayout->getSelected();
    assert(index);
    if(!triggerData->m_triggerLinkWall)
    {
        triggerData->m_triggerLinkWall = QSet<QPair<int, int>>();
    }
    setElementSelected(LevelElement_e::WALL, *index);
    setLineSelectableEnabled(true);
    QModelIndex wallIndex;
    m_tableModel->updateTriggerPos({caseIndex.column(), caseIndex.row()});
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
        else if(var->m_type == LevelElement_e::WALL && var->m_moveWallData)
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
void GridEditor::setColorElement(const QModelIndex &caseIndex, LevelElement_e elementType)
{
    QPair<uint32_t, Direction_e> checkpointData;
    std::optional<CaseData> &caseData = m_tableModel->getDataElementCase(caseIndex);
    if(elementType == LevelElement_e::CHECKPOINT)
    {
        m_checkpointForm->reinit();
        uint32_t numberOfCheckpoint = m_tableModel->getNumberOfCheckpoints();
        bool checkpointExists = (caseData && caseData->m_type == LevelElement_e::CHECKPOINT);
        if(numberOfCheckpoint > 0 && !checkpointExists)
        {
            m_checkpointForm->setSpinBoxSize(numberOfCheckpoint);
        }
        else if(checkpointExists)
        {
            m_checkpointForm->setSpinBoxSize(numberOfCheckpoint - 1);
        }
        if(checkpointExists)
        {
            std::optional<QPair<uint32_t, Direction_e>> checkpointData = m_tableModel->getCheckpointData({caseIndex.column(), caseIndex.row()});
            assert(checkpointData);
            m_checkpointForm->setMenuEntries(*checkpointData);
        }
        else
        {
            m_checkpointForm->setMenuEntries({numberOfCheckpoint, Direction_e::NORTH});
        }
        m_checkpointForm->exec();
        if(!m_checkpointForm->isValid())
        {
            return;
        }
        checkpointData = m_checkpointForm->getCheckpointData();
    }
    if(caseData->m_type != LevelElement_e::TRIGGER && caseData->m_type != LevelElement_e::GROUND_TRIGGER)
    {
        m_tableModel->removeData(caseIndex);
    }
    setColorCaseData(caseIndex.column(), caseIndex.row(), elementType, checkpointData);
}

//======================================================================
void GridEditor::setWallDrawModeSelected(int wallDrawMode)
{
    m_wallDrawMode = static_cast<WallDrawShape_e>(wallDrawMode);
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
    if(m_backgroundForm)
    {
        delete m_backgroundForm;
    }
}

//======================================================================
bool GridEditor::loadStandardExistingLevelGrid(LevelElement_e elementType)
{
    LevelData const *existingLevel = m_levelDataManager.getExistingLevel();
    m_currentElementType = elementType;
    std::multimap<QString, QPair<int, int>> const *currentContainer = nullptr;
    switch(m_currentElementType)
    {
    case LevelElement_e::BARREL:
    {
        currentContainer = &existingLevel->m_barrelsData;
        break;
    }
    case LevelElement_e::DOOR:
    {
        currentContainer = &existingLevel->m_doorsData;
        break;
    }
    case LevelElement_e::ENEMY:
    {
        currentContainer = &existingLevel->m_enemiesData;
        break;
    }
    case LevelElement_e::EXIT:
    {
        currentContainer = &existingLevel->m_exitData;
        break;
    }
    case LevelElement_e::OBJECT:
    {
        currentContainer = &existingLevel->m_objectsData;
        break;
    }
    case LevelElement_e::STATIC_CEILING:
    {
        currentContainer = &existingLevel->m_ceilingElementsData;
        break;
    }
    case LevelElement_e::STATIC_GROUND:
    {
        currentContainer = &existingLevel->m_groundElementsData;
        break;
    }
    case LevelElement_e::TELEPORT:
    case LevelElement_e::TARGET_TELEPORT:
    case LevelElement_e::WALL:
    case LevelElement_e::TRIGGER:
    case LevelElement_e::GROUND_TRIGGER:
    case LevelElement_e::PLAYER_DEPARTURE:
    case LevelElement_e::DELETE:
    case LevelElement_e::SELECTION:
    case LevelElement_e::CHECKPOINT:
    case LevelElement_e::SECRET:
    case LevelElement_e::LOG:
    case LevelElement_e::TOTAL:
        assert(false);
    }
    int elementTypeInt = static_cast<int>(m_currentElementType);
    for(std::map<QString, QPair<int, int>>::const_iterator it = currentContainer->begin(); it != currentContainer->end(); ++it)
    {
        m_currentSelection = m_mapElementID[m_currentElementType].indexOf(it->first);
        if(m_currentSelection == -1)
        {
            return false;
        }
        QModelIndex index = m_tableModel->index(it->second.second, it->second.first, QModelIndex());
        setCaseIcon(it->second.first, it->second.second, -1);
        m_tableModel->memStdElement({index.column(), index.row()}, m_currentElementType,
                                    m_drawData[elementTypeInt][m_currentSelection].m_elementSectionName);
    }
    //load end level enemy
    if(m_currentElementType == LevelElement_e::ENEMY && existingLevel->m_endLevelEnemyPos)
    {
        m_currentSelection = m_mapElementID[m_currentElementType].indexOf(existingLevel->m_endLevelEnemyPos->first);
        QModelIndex index = m_tableModel->index(existingLevel->m_endLevelEnemyPos->second.second,
                                                existingLevel->m_endLevelEnemyPos->second.first, QModelIndex());
        bool ok = m_tableModel->setData(index, QVariant(getColoredBackgroundIcon()));
        assert(ok);
        m_tableModel->setIdData(index, CaseData{m_currentElementType,
                                                m_mapElementID[m_currentElementType][m_currentSelection], {}, {}, {}, {}}, true);
        m_tableModel->memStdElement({index.column(), index.row()}, m_currentElementType,
                                    m_drawData[elementTypeInt][m_currentSelection].m_elementSectionName);
        m_tableModel->setEndLevelEnemyPos({index.column(), index.row()});
    }
    return true;
}

//======================================================================
void GridEditor::loadSecretsExistingLevelGrid()
{
    for(int i = 0; i < m_levelDataManager.getExistingLevel()->m_secrets.size(); ++i)
    {
        setColorCaseData(m_levelDataManager.getExistingLevel()->m_secrets[i].first,
                         m_levelDataManager.getExistingLevel()->m_secrets[i].second, LevelElement_e::SECRET);
    }
}

//======================================================================
void GridEditor::loadCheckpointsExistingLevelGrid()
{
    for(int i = 0; i < m_levelDataManager.getExistingLevel()->m_checkpoints.size(); ++i)
    {
        setColorCaseData(m_levelDataManager.getExistingLevel()->m_checkpoints[i].first.first,
                         m_levelDataManager.getExistingLevel()->m_checkpoints[i].first.second, LevelElement_e::CHECKPOINT,
                         {i, m_levelDataManager.getExistingLevel()->m_checkpoints[i].second});
    }
}

//======================================================================
void GridEditor::loadLogsExistingLevelGrid()
{
    m_currentElementType = LevelElement_e::LOG;
    for(std::map<QString, LogData>::const_iterator it = m_levelDataManager.getExistingLevel()->m_logsData.begin();
        it != m_levelDataManager.getExistingLevel()->m_logsData.end(); ++it)
    {
        m_currentSelection = m_mapElementID[LevelElement_e::LOG].indexOf(it->second.m_displayID);
        if(m_currentSelection == -1)
        {
            return;
        }
        setCaseIcon(it->second.m_position.first, it->second.m_position.second, -1);
        m_tableModel->addLog(it->second.m_position, it->second.m_message, it->second.m_displayID);
    }
}

//======================================================================
bool GridEditor::loadTeleportExistingLevelGrid()
{
    const std::multimap<QString, TeleportData> &teleportData = m_levelDataManager.getExistingLevel()->m_teleportData;
    for(std::multimap<QString, TeleportData>::const_iterator it = teleportData.begin(); it != teleportData.end(); ++it)
    {
        //TELEPORTER
        m_currentElementType = LevelElement_e::TELEPORT;
        QModelIndex index = m_tableModel->index(it->second.m_teleporterPos.second, it->second.m_teleporterPos.first, QModelIndex());
        m_currentSelection = m_mapElementID[m_currentElementType].indexOf(it->first);
        setCaseIcon(it->second.m_teleporterPos.first, it->second.m_teleporterPos.second, -1);
        m_tableModel->setIdData(index, CaseData{m_currentElementType,
                                                m_mapElementID[m_currentElementType][m_currentSelection], {}, {}, {}, {}});
        m_lastPositionAdded = it->second.m_teleporterPos;
        //TARGET
        m_currentElementType = LevelElement_e::TARGET_TELEPORT;
        QModelIndex indexx = m_tableModel->index(it->second.m_targetPos.second, it->second.m_targetPos.first, QModelIndex());
        setTargetTeleport(indexx);
    }
    return true;
}

//======================================================================
bool GridEditor::loadBackgroundGeneralExistingLevelGrid()
{
    QPair<BackgroundData, BackgroundData> const *backgroundData = m_levelDataManager.getExistingLevel()->m_backgroundData.get();
    if(!backgroundData)
    {
        return false;
    }
    //GROUND
    m_backgroundForm->setBackgroundData(backgroundData->first, true);
    //CEILING
    m_backgroundForm->setBackgroundData(backgroundData->second, false);
    return true;
}

//======================================================================
bool GridEditor::loadWallExistingLevelGrid()
{
    const std::map<QString, WallDataINI> &wallsData = m_levelDataManager.getExistingLevel()->m_wallsData;
    QString currentINIID;
    m_currentElementType = LevelElement_e::WALL;
    QPair<int, int> currentCoord;
    m_loadingExistingLevelMode = true;
    m_memCurrentLinkTriggerWall.clear();
    for(std::map<QString, WallDataINI>::const_iterator it = wallsData.begin(); it != wallsData.end(); ++it)
    {
        assert(it->second.m_vectPos);
        currentINIID = *it->second.m_iniID;
        m_currentSelection = m_mapElementID[m_currentElementType].indexOf(currentINIID);
        if(m_currentSelection < 0)
        {
            return false;
        }
        for(int32_t i = 0; i < it->second.m_vectPos->size(); ++i)
        {
            m_wallDrawMode = (*it->second.m_vectPos)[i].first;
            m_firstCaseSelection = m_tableModel->index((*it->second.m_vectPos)[i].second.m_gridCoordTopLeft.second,
                                                       (*it->second.m_vectPos)[i].second.m_gridCoordTopLeft.first);
            m_secondCaseSelection = m_tableModel->index((*it->second.m_vectPos)[i].second.m_gridCoordBottomRight.second,
                                                       (*it->second.m_vectPos)[i].second.m_gridCoordBottomRight.first);
            if(it->second.m_moveableData)
            {
                m_wallMoveableMode = true;
                if(!m_memcurrentMoveWallData)
                {
                    m_memcurrentMoveWallData = std::make_unique<MoveWallData>();
                }
                m_memcurrentMoveWallData->clear();
                *m_memcurrentMoveWallData = (*it->second.m_moveableData);
                m_loadingDistantTriggerMode = true;
            }
            setWallShape(false, true);
            m_wallMoveableMode = false;
            if(it->second.m_moveableData && it->second.m_moveableData->m_triggerType != TriggerType_e::WALL)
            {
                currentCoord = {it->second.m_moveableData->m_triggerPos->first, it->second.m_moveableData->m_triggerPos->second};
                if(it->second.m_moveableData->m_triggerType == TriggerType_e::DISTANT_SWITCH)
                {
                    m_currentElementType = LevelElement_e::TRIGGER;
                    m_currentSelection = m_mapElementID[m_currentElementType].indexOf(it->second.m_moveableData->m_triggerINISectionName);
                    setCaseIcon(currentCoord.first, currentCoord.second, -1);
                }
                else if(it->second.m_moveableData->m_triggerType == TriggerType_e::GROUND)
                {
                    m_currentElementType = LevelElement_e::GROUND_TRIGGER;
                    setColorCaseData(currentCoord.first, currentCoord.second, m_currentElementType);
                }
                confNewTriggerData(m_tableModel->index(currentCoord.second, currentCoord.first));
                m_memCurrentLinkTriggerWall.clear();
            }
        }
        loadRemovedWallExistingLevelGrid(*it);
    }
    m_loadingExistingLevelMode = false;
    m_loadingDistantTriggerMode = false;
    return true;
}

//======================================================================
bool GridEditor::loadRemovedWallExistingLevelGrid(const std::pair<const QString, WallDataINI> &currentTreat)
{
    if(!currentTreat.second.m_vectRem || currentTreat.second.m_vectRem->empty())
    {
        return true;
    }
    for(int32_t i = 0; i < currentTreat.second.m_vectRem->size(); ++i)
    {
        QPair<int, int> origin = (*currentTreat.second.m_vectRem)[i].second.m_gridCoordTopLeft,
                bottomRight = (*currentTreat.second.m_vectRem)[i].second.m_gridCoordTopLeft;
        //point case
        if((*currentTreat.second.m_vectRem)[i].second.m_gridCoordTopLeft ==
                (*currentTreat.second.m_vectRem)[i].second.m_gridCoordBottomRight)
        {
            m_tableModel->removeData(m_tableModel->index(origin.second, origin.first));
            continue;
        }
        switch((*currentTreat.second.m_vectRem)[i].first)
        {
        case WallDrawShape_e::LINE_AND_RECT:
        {
            setWallLineRectShape(origin, bottomRight, -1, false, true);
            break;
        }
        case WallDrawShape_e::DIAGONAL_LINE:
        {
            setWallDiagLineShape(origin, bottomRight, -1, false, true);
            break;
        }
        case WallDrawShape_e::DIAGONAL_RECT:
        {
            //method constraint
            uint32_t random;
            setWallDiagRectShape(origin, bottomRight, -1, random, false, true);
            break;
        }
        default:
            assert(false);
        }
    }
    return true;
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
    case LevelElement_e::LOG:
        return "MessageLog";
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
    case LevelElement_e::CHECKPOINT:
        return "Checkpoint";
    case LevelElement_e::SECRET:
        return "Secret";
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

//======================================================================
QString getStrDir(Direction_e direction)
{
    switch(direction)
    {
    case Direction_e::EAST:
        return "E";
        break;
    case Direction_e::WEST:
        return "W";
        break;
    case Direction_e::NORTH:
        return "N";
        break;
    case Direction_e::SOUTH:
        return "S";
        break;
    }
    return "";
}

//======================================================================
QString getStrCheckpoint(const QPair<int, Direction_e> &direction)
{
    return "C" + getStrDir(direction.second) + QString::number(direction.first) ;
}
