#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <QVector>

class TableModel : public QAbstractTableModel
{
public:
    explicit TableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex())const override;
    int columnCount(const QModelIndex &parent = QModelIndex())const override;
    QVariant data(const QModelIndex &index, int role = Qt::DecorationRole)const override;
    void setLevelSize(int tableWidth, int tableHeight);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)override;
    bool removeData(const QModelIndex &index);
    inline int getTableWidth()const
    {
        return m_tableWidth;
    }
    inline int getTableHeight()const
    {
        return m_tableHeight;
    }
private:
    int m_tableWidth, m_tableHeight;
    QVector<QVector<QPixmap>> m_vectPic;
signals:
    void editCompleted(const QString &str);
};
