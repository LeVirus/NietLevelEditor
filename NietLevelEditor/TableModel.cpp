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
    QPixmap pix;
    m_vectPic[index.column()][index.row()].first.swap(pix);
    if(m_vectPic[index.column()][index.row()].second->m_type == LevelElement_e::PLAYER_DEPARTURE)
    {
        m_departurePlayer = {};
    }
    m_vectPic[index.column()][index.row()].second = {};
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
