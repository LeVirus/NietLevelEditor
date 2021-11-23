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
    if (role == Qt::EditRole)
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
bool TableModel::setIdData(const QModelIndex &index, const CaseData &value)
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
    return true;
}

//======================================================================
bool TableModel::removeData(const QModelIndex &index)
{
    if (!checkIndex(index))
    {
        return false;
    }
    std::optional<CaseData> &caseData = m_vectPic[index.column()][index.row()].second;
    if(!caseData)
    {
        return true;
    }
    QPixmap pix;
    m_vectPic[index.column()][index.row()].first.swap(pix);
    if(caseData->m_type == LevelElement_e::PLAYER_DEPARTURE)
    {
        m_departurePlayer = {};
    }
    else if(caseData->m_type == LevelElement_e::WALL)
    {
        assert(caseData->m_wallShapeNum);
        --m_memWallShape[*caseData->m_wallShapeNum].second.m_wallCount;
    }
    caseData = {};
    return true;
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
        m_memWallShape.push_back({wallShape, {topLeftIndex, bottomRightIndex, 0, {}, iniId, *memMoveData}});
    }
    else
    {
        m_memWallShape.push_back({wallShape, {topLeftIndex, bottomRightIndex, 0, {}, iniId, {}}});
    }
    return m_memWallShape.size() - 1;
}

//======================================================================
void TableModel::memStdElement(const QPair<int, int> &pos, LevelElement_e elementType, const QString &iniId)
{
    switch(elementType)
    {
    case LevelElement_e::BARREL:
        m_memBarrel.insert({iniId, pos});
        break;
    case LevelElement_e::DOOR:
        m_memDoor.insert({iniId, pos});
        break;
    case LevelElement_e::ENEMY:
        m_memEnemy.insert({iniId, pos});
        break;
    case LevelElement_e::EXIT:
        m_memExit.insert({iniId, pos});
        break;
    case LevelElement_e::OBJECT:
        m_memObject.insert({iniId, pos});
        break;
    case LevelElement_e::STATIC_CEILING:
        m_memStaticCeiling.insert({iniId, pos});
        break;
    case LevelElement_e::STATIC_GROUND:
        m_memStaticGround.insert({iniId, pos});
        break;
    case LevelElement_e::TELEPORT:
    case LevelElement_e::TRIGGER:
    case LevelElement_e::SELECTION:
    case LevelElement_e::TARGET_TELEPORT:
    case LevelElement_e::PLAYER_DEPARTURE:
    case LevelElement_e::DELETE:
    case LevelElement_e::GROUND_TRIGGER:
    case LevelElement_e::WALL:
    case LevelElement_e::TOTAL:
        break;
    }
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
    m_memWallShape.back().second.m_wallCount = num;
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
