#include "TableModel.hpp"
#include <QFont>
#include <QBrush>
#include <iostream>

//======================================================================
TableModel::TableModel(QObject *parent) : QAbstractTableModel(parent)
{

}

//======================================================================
int TableModel::rowCount(const QModelIndex &parent)const
{
    return m_tableSize.second;
}

//======================================================================
int TableModel::columnCount(const QModelIndex &parent)const
{
    return m_tableSize.first;
}

//======================================================================
QVariant TableModel::data(const QModelIndex &index, int role)const
{
    switch (role)
    {
    case Qt::DecorationRole:
    {
        return m_vectPic[index.column()][index.row()].first;
    }
    case Qt::BackgroundRole:
    {
        if(m_vectPreview[index.column()][index.row()])
        {
            return QBrush(Qt::green);
        }
        break;
    }
    }
    return QVariant();
}

//======================================================================
bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role == Qt::EditRole)
    {
        if(!checkIndex(index))
        {
            return false;
        }
        assert(index.column() < m_vectPic.size());
        assert(index.row() < m_vectPic[index.column()].size());
        m_vectPic[index.column()][index.row()].first = value.value<QPixmap>();
        return true;
    }
    return false;
}

//======================================================================
bool TableModel::setIdData(const QModelIndex &index, const CaseData &value, bool endLevelEnemyCase)
{
    if(!checkIndex(index))
    {
        return false;
    }
    assert(index.column() < m_vectPic.size());
    assert(index.row() < m_vectPic[index.column()].size());
    m_vectPic[index.column()][index.row()].second = value;
    if(value.m_type == LevelElement_e::PLAYER_DEPARTURE)
    {
        if(m_departurePlayer)
        {
            removeData(this->index(m_departurePlayer->second, m_departurePlayer->first, QModelIndex()));
        }
        m_departurePlayer = {index.column(), index.row()};
    }
    else if(value.m_type == LevelElement_e::EXIT)
    {
        if(m_exitPos)
        {
            removeData(this->index(m_exitPos->second, m_exitPos->first, QModelIndex()));
        }
        if(m_levelEndEnemy)
        {
            removeData(this->index(m_levelEndEnemy->second, m_levelEndEnemy->first, QModelIndex()));
        }
        m_exitPos = {index.column(), index.row()};
    }
    else if(value.m_type == LevelElement_e::ENEMY && endLevelEnemyCase)
    {
        if(m_levelEndEnemy)
        {
            removeData(this->index(m_levelEndEnemy->second, m_levelEndEnemy->first, QModelIndex()));
        }
        if(m_exitPos)
        {
            removeData(this->index(m_exitPos->second, m_exitPos->first, QModelIndex()));
        }
        m_levelEndEnemy = {index.column(), index.row()};
    }
    return true;
}

//======================================================================
void TableModel::removeData(const QModelIndex &index, bool dontMemRemovedWall)
{
    if(!checkIndex(index) /*|| index.column() < 0 || index.row() < 0*/)
    {
        return;
    }
    std::optional<CaseData> &caseData = m_vectPic[index.column()][index.row()].second;
    if(!caseData)
    {
        return;
    }
    QPixmap pix;
    //remove picture or color
    m_vectPic[index.column()][index.row()].first.swap(pix);
    if(caseData->m_type == LevelElement_e::PLAYER_DEPARTURE)
    {
        m_departurePlayer.reset();
    }
    else if(caseData->m_type == LevelElement_e::EXIT)
    {
        m_exitPos.reset();
    }
    else if(caseData->m_type == LevelElement_e::ENEMY && m_levelEndEnemy)
    {
        if(*m_levelEndEnemy == QPair<int, int>{index.column(), index.row()})
        {
            rmStdElement({index.column(), index.row()}, caseData->m_type);
            m_levelEndEnemy.reset();
        }
    }
    else if(caseData->m_type == LevelElement_e::WALL)
    {
        assert(caseData->m_wallShapeNum);
        --m_memWallShape[*caseData->m_wallShapeNum].second.m_currentWallCount;
        m_memWallShape[*caseData->m_wallShapeNum].second.m_deletedWall.push_back({index.column(), index.row()});
        if(caseData->m_moveWallData->m_triggerPos)
        {
            if(dontMemRemovedWall)
            {
                m_memWallShape[*caseData->m_wallShapeNum].second.m_deletedWall.push_back({index.column(), index.row()});
            }
            QModelIndex triggerIndex = this->index(caseData->m_moveWallData->m_triggerPos->second,
                                                   caseData->m_moveWallData->m_triggerPos->first);
            if(triggerIndex.isValid())
            {
                std::optional<CaseData> &triggerData = getDataElementCase(triggerIndex);
                if(triggerData)
                {
                    QSet<QPair<int, int>>::iterator it = triggerData->m_triggerLinkWall->find({index.column(), index.row()});
                    if(it != triggerData->m_triggerLinkWall->end())
                    {
                        triggerData->m_triggerLinkWall->erase(it);
                    }
                    caseData->m_moveWallData->m_triggerPos.reset();
                    caseData->m_moveWallData.reset();
                }
            }
        }
    }
    else if(caseData->m_type == LevelElement_e::CHECKPOINT)
    {
        removeCheckpoint({index.column(), index.row()});
    }
    else if(caseData->m_type == LevelElement_e::SECRET)
    {
        removeSecret({index.column(), index.row()});
    }
    else
    {
        rmStdElement({index.column(), index.row()}, caseData->m_type);
    }
    caseData.reset();
    return;
}

//======================================================================
void TableModel::removeCheckpoint(const QPair<int, int> &pos)
{
    for(int32_t i = 0; i < m_vectCheckpoints.size(); ++i)
    {
        if(m_vectCheckpoints[i].first == pos)
        {
            m_vectCheckpoints.erase(m_vectCheckpoints.begin() + i);
            return;
        }
    }
    assert(false);
}

//======================================================================
void TableModel::removeSecret(const QPair<int, int> &pos)
{
    for(int32_t i = 0; i < m_vectSecrets.size(); ++i)
    {
        if(m_vectSecrets[i] == pos)
        {
            m_vectSecrets.erase(m_vectSecrets.begin() + i);
            return;
        }
    }
    assert(false);
}

//======================================================================
void TableModel::clearPreview()
{
    for(int i = 0; i < m_vectPreview.size(); ++i)
    {
        m_vectPreview[i].fill(false);
    }
}

//======================================================================
void TableModel::clearModel()
{
    m_exitPos.reset();
    m_memWallShape.clear();
    m_vectPic.clear();
    m_departurePlayer = {};
    m_vectPreview.clear();
    m_memBarrel.clear();
    m_memDoor.clear();
    m_memEnemy.clear();
    m_memExit.clear();
    m_memObject.clear();
    m_memStaticCeiling.clear();
    m_memStaticGround.clear();
    m_memTeleport.clear();
    m_memLog.clear();
    m_vectCheckpoints.clear();
    m_vectSecrets.clear();
}

//======================================================================
void TableModel::setPreviewCase(int x, int y)
{
    if(x < 0 || y < 0)
    {
        return;
    }
    m_vectPreview[x][y] = true;
}

//======================================================================
void TableModel::setPreviewCase(const QPair<int, int> &pos)
{
    setPreviewCase(pos.first, pos.second);
}

//======================================================================
void TableModel::setTargetTeleport(const QPair<int, int> &teleporterPosition,
                                   const QModelIndex &targetPos)
{
    std::optional<CaseData> &caseData =
            m_vectPic[teleporterPosition.first][teleporterPosition.second].second;
    if(!caseData)
    {
        caseData = CaseData();
        caseData->m_type = LevelElement_e::TELEPORT;
    }
    caseData->m_targetTeleport = {targetPos.column(), targetPos.row()};
}

//======================================================================
int TableModel::memWallShape(WallDrawShape_e wallShape, const QPair<int, int> &topLeftIndex,
                             const QPair<int, int> &bottomRightIndex, const QString &iniId, const MoveWallData *memMoveData)
{
    if(memMoveData)
    {
        m_memWallShape.push_back({wallShape, {topLeftIndex, bottomRightIndex, 0, 0, {}, iniId, *memMoveData, false}});
    }
    else
    {
        m_memWallShape.push_back({wallShape, {topLeftIndex, bottomRightIndex, 0, 0, {}, iniId, {}, false}});
    }
    return m_memWallShape.size() - 1;
}

//======================================================================
void TableModel::memStdElement(const QPair<int, int> &pos, LevelElement_e elementType,
                               const QString &iniId)
{
    if(elementType == LevelElement_e::BARREL)
    {
        m_memBarrel.insert({iniId, pos});
    }
    else if(elementType == LevelElement_e::DOOR)
    {
        m_memDoor.insert({iniId, pos});
    }
    else if(elementType == LevelElement_e::ENEMY)
    {
        m_memEnemy.insert({iniId, pos});
    }
    else if(elementType == LevelElement_e::EXIT)
    {
        m_memExit.clear();
        m_memExit.insert({iniId, pos});
    }
    else if(elementType == LevelElement_e::OBJECT)
    {
        m_memObject.insert({iniId, pos});
    }
    else if(elementType == LevelElement_e::STATIC_CEILING)
    {
        m_memStaticCeiling.insert({iniId, pos});
    }
    else if(elementType == LevelElement_e::STATIC_GROUND)
    {
        m_memStaticGround.insert({iniId, pos});
    }
}

//======================================================================
void TableModel::rmStdElement(const QPair<int, int> &pos, LevelElement_e elementType)
{
    std::multimap<QString, QPair<int, int>>::iterator it;
    if(elementType == LevelElement_e::BARREL)
    {
        for(it = m_memBarrel.begin(); it != m_memBarrel.end(); ++it)
        {
            if(it->second == pos)
            {
                m_memBarrel.erase(it);
                return;
            }
        }
    }
    else if(elementType == LevelElement_e::DOOR)
    {
        for(it = m_memDoor.begin(); it != m_memDoor.end(); ++it)
        {
            if(it->second == pos)
            {
                m_memDoor.erase(it);
                return;
            }
        }
    }
    else if(elementType == LevelElement_e::ENEMY)
    {
        for(it = m_memEnemy.begin(); it != m_memEnemy.end(); ++it)
        {
            if(it->second == pos)
            {
                m_memEnemy.erase(it);
                return;
            }
        }
    }
    else if(elementType == LevelElement_e::EXIT)
    {
        for(it = m_memExit.begin(); it != m_memExit.end(); ++it)
        {
            if(it->second == pos)
            {
                m_memExit.erase(it);
                return;
            }
        }
    }
    else if(elementType == LevelElement_e::OBJECT)
    {
        for(it = m_memObject.begin(); it != m_memObject.end(); ++it)
        {
            if(it->second == pos)
            {
                m_memObject.erase(it);
                return;
            }
        }
    }
    else if(elementType == LevelElement_e::STATIC_CEILING)
    {
        for(it = m_memStaticCeiling.begin(); it != m_memStaticCeiling.end(); ++it)
        {
            if(it->second == pos)
            {
                m_memStaticCeiling.erase(it);
                return;
            }
        }
    }
    else if(elementType == LevelElement_e::STATIC_GROUND)
    {
        for(it = m_memStaticGround.begin(); it != m_memStaticGround.end(); ++it)
        {
            if(it->second == pos)
            {
                m_memStaticGround.erase(it);
                return;
            }
        }
    }
    else if(elementType == LevelElement_e::TELEPORT)
    {
        for(std::multimap<QString, TeleportData>::iterator it = m_memTeleport.begin(); it != m_memTeleport.end(); ++it)
        {
            if(it->second.m_teleporterPos == pos)
            {
                m_memTeleport.erase(it);
                return;
            }
        }
    }
    else if(elementType == LevelElement_e::LOG)
    {
        for(int32_t i = 0; i < m_memLog.size(); ++i)
        {
            if(m_memLog[i].m_position == pos)
            {
                m_memLog.erase(m_memLog.begin() + i);
                return;
            }
        }
    }
    else
    {
        return;
    }
    std::cout << "ERROR " << (int)elementType << " " << pos.first << "  " << pos.second << "REMOVE FAILED \n";
    assert(false);
}

//======================================================================
void TableModel::memTeleportElement(const QPair<int, int> &teleporterPos, const QPair<int, int> &targetPos, const QString &iniId)
{
    m_memTeleport.insert({iniId, {teleporterPos, targetPos}});
}

//======================================================================
void TableModel::updateWallNumber(uint32_t num)
{
    assert(!m_memWallShape.empty());
    m_memWallShape.back().second.m_baseWallCount = num;
    m_memWallShape.back().second.m_currentWallCount = num;
}

//======================================================================
void TableModel::updateTriggerPos(const QPair<int, int> &pos)
{
    assert(!m_memWallShape.empty());
    m_memWallShape.back().second.m_memMoveData->m_triggerPos = pos;
}

//======================================================================
void TableModel::removeTrigger(CaseData &triggerCase, const QPair<int, int> &triggercoord)
{
    removeData(this->index(triggercoord.second, triggercoord.first, QModelIndex()));
    if(!triggerCase.m_triggerLinkWall || triggerCase.m_triggerLinkWall->empty())
    {
        return;
    }
    QModelIndex index;
    for(QSet<QPair<int, int>>::const_iterator it = triggerCase.m_triggerLinkWall->begin(); it != triggerCase.m_triggerLinkWall->end(); ++it)
    {
        index = this->index(it->second, it->first, QModelIndex());
        if(!index.isValid())
        {
            continue;
        }
        CaseData &wallCase = *m_vectPic[index.column()][index.row()].second;
        wallCase.m_moveWallData->clear();
        wallCase.m_moveWallData.reset();
        assert(!wallCase.m_moveWallData);
        wallCase.m_moveWallData = std::nullopt;
    }
    for(int i = 0; i < m_memWallShape.size(); ++i)
    {
        if(m_memWallShape[i].second.m_memMoveData && m_memWallShape[i].second.m_memMoveData->m_triggerPos &&
                *m_memWallShape[i].second.m_memMoveData->m_triggerPos == triggercoord)
        {
            m_memWallShape[i].second.m_memMoveData.reset();
            return;
        }
    }
}

//======================================================================
void TableModel::setTableDeletionZone(const QPair<int, int> &originSelectPos, const QPair<int, int> &targetSelectPos, bool preview)
{
    int minX = std::min(originSelectPos.first, targetSelectPos.first),
            maxX = std::max(originSelectPos.first, targetSelectPos.first),
            minY = std::min(originSelectPos.second, targetSelectPos.second),
            maxY = std::max(originSelectPos.second, targetSelectPos.second);
    clearPreview();
    QModelIndex index;
    for(int i = minY; i < maxY + 1; ++i)
    {
        for(int j = minX; j < maxX + 1; ++j)
        {
            if(preview)
            {
                setPreviewCase(j, i);
            }
            else
            {
                index = this->index(i, j, QModelIndex());
                removeData(index);
            }
        }
    }
}

//======================================================================
void TableModel::setTableWallDiagCaseConf(QPair<int, int> originPoint, bool directionUp)
{
    m_memWallShape.back().second.m_gridCoordTopLeft = originPoint;
    m_memWallShape.back().second.m_diagCaseUp = directionUp;
}

//======================================================================
void TableModel::setTableWallDiagRectCaseConf(QPair<int, int> originPoint)
{
    m_memWallShape.back().second.m_gridCoordTopLeft = originPoint;
}

//======================================================================
bool TableModel::checkLevelData()const
{
    if((!m_exitPos && !m_levelEndEnemy) || !m_departurePlayer)
    {
        return false;
    }
    return true;
}

//======================================================================
std::optional<LogData> TableModel::getLogDataPos(const QPair<int, int> &pos)const
{
    for(int32_t i = 0; i < m_memLog.size(); ++i)
    {
        if(m_memLog[i].m_position == pos)
        {
            return m_memLog[i];
        }
    }
    return {};
}

//======================================================================
void TableModel::addCheckpoint(const QPair<int, int> &pos, const QPair<uint32_t, Direction_e> &checkpointData)
{
    for(int i = 0; i < m_vectCheckpoints.size(); ++i)
    {
        if(m_vectCheckpoints[i].first == pos)
        {
            m_vectCheckpoints.erase(m_vectCheckpoints.begin() + i);
            break;
        }
    }
    m_vectCheckpoints.insert(checkpointData.first, {pos, checkpointData.second});
}

//======================================================================
void TableModel::addLog(const QPair<int, int> &pos, const QString &message, const QString &displayID)
{
    m_memLog.push_back({pos, message, displayID});
}

//======================================================================
std::optional<QPair<uint32_t, Direction_e>> TableModel::getCheckpointData(const QPair<int, int> &pos)const
{
    for(int i = 0; i < m_vectCheckpoints.size(); ++i)
    {
        if(m_vectCheckpoints[i].first == pos)
        {
            return QPair<uint32_t, Direction_e>{static_cast<uint32_t>(i), m_vectCheckpoints[i].second};
        }
    }
    return {};
}

//======================================================================
void TableModel::setLevelSize(int tableWidth, int tableHeight)
{
    m_tableSize = {tableWidth, tableHeight};
    m_vectPic.resize(tableWidth);
    m_vectPreview.resize(tableWidth);
    for(int i = 0; i < m_vectPic.size(); ++i)
    {
        m_vectPic[i].resize(tableHeight);
        m_vectPreview[i].resize(tableHeight);
    }
}
