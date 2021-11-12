#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <QVector>
#include <QBitArray>
#include "GridEditor.hpp"

struct CaseData
{
    LevelElement_e m_type;
    QString m_id;
    std::optional<QPair<int, int>> m_targetTeleport;
    std::optional<QVector<QPair<int, int>>> m_moveWallData;
};

class TableModel : public QAbstractTableModel
{
public:
    explicit TableModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex())const override;
    int columnCount(const QModelIndex &parent = QModelIndex())const override;
    QVariant data(const QModelIndex &index, int role = Qt::DecorationRole)const override;
    void setLevelSize(int tableWidth, int tableHeight);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)override;
    bool setIdData(const QModelIndex &index, const CaseData &value);
    bool removeData(const QModelIndex &index);
    inline std::optional<CaseData> getDataElementCase(const QModelIndex &index)const
    {
        return m_vectPic[index.column()][index.row()].second;
    }
    void clearPreview();
    void setPreviewCase(int x, int y);
    void setPreviewCase(const QPair<int, int> &pos);
    void setTargetTeleport(const QPair<int, int> &teleporterPosition,
                           const QModelIndex &targetPos);
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
    QVector<QVector<QPair<QPixmap, std::optional<CaseData>>>> m_vectPic;
    QVector<QBitArray> m_vectPreview;
    std::optional<QPair<int, int>> m_departurePlayer;
signals:
    void editCompleted(const QString &str);
};
