#include "TableModel.hpp"
#include <QFont>
#include <QBrush>

//======================================================================
TableModel::TableModel(QObject *parent) : QAbstractTableModel(parent)
{

}

//======================================================================
int TableModel::rowCount(const QModelIndex &parent)const
{
    return m_tableHeight;
}

//======================================================================
int TableModel::columnCount(const QModelIndex &parent)const
{
    return m_tableWidth;
}

//======================================================================
QVariant TableModel::data(const QModelIndex &index, int role)const
{
//    int row = index.row();
//    int col = index.column();
    switch (role)
    {
    case Qt::DecorationRole:
    {
        return m_vectPic[index.column()][index.row()];
    }
//    case Qt::BackgroundRole:
//    {
//        if (row == 0 && col == 2)  //change background only for cell(1,2)
//            return QBrush(Qt::red);
//        break;
//    }
    }
    return QVariant();
}

//======================================================================
bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole)
    {
        if (!checkIndex(index))
        {
            return false;
        }
        m_vectPic[index.row()][index.column()] = value.value<QPixmap>();
        return true;
    }
    return false;
}

//======================================================================
bool TableModel::removeData(const QModelIndex &index)
{
    if (!checkIndex(index))
    {
        return false;
    }
    QPixmap pix;
    m_vectPic[index.row()][index.column()].swap(pix);
    return true;
}

//======================================================================
void TableModel::setLevelSize(int tableWidth, int tableHeight)
{
    m_tableWidth = tableWidth;
    m_tableHeight = tableHeight;
    m_vectPic.resize(tableWidth);
    for(int i = 0; i < m_vectPic.size(); ++i)
    {
        m_vectPic[i].resize(tableHeight);
    }
}
