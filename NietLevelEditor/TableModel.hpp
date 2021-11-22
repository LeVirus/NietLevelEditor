#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <QVector>
#include <QBitArray>
#include <QSet>
#include "GridEditor.hpp"
#include "MoveableWallForm.hpp"

struct MoveWallData
{
    QVector<QPair<int, int>> m_memMoveWallCases;
    QVector<QPair<Direction_e, int>> m_memMoveWallData;
    int m_velocity;
    TriggerType_e m_triggerType;
    TriggerBehaviourType_e m_triggerBehaviour;
    void clear()
    {
        m_memMoveWallCases.clear();
        m_memMoveWallData.clear();
        m_triggerPos = {};
    }
    std::optional<QPair<int, int>> m_triggerPos;
};

struct CaseData
{
    LevelElement_e m_type;
    QString m_id;
    std::optional<int> m_wallShapeNum;
    std::optional<QPair<int, int>> m_targetTeleport;
    std::optional<MoveWallData> m_moveWallData;
    std::optional<QSet<QPair<int, int>>> m_triggerLinkWall;
};

struct WallShapeData
{
    QPair<int, int> m_gridCoordTopLeft, m_gridCoordBottomRight;
    uint32_t m_wallCount = 0;
    QVector<QPair<int, int>> m_deletedWall;
    QString m_iniId;
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
    inline std::optional<CaseData> &getDataElementCase(const QModelIndex &index)
    {
        return m_vectPic[index.column()][index.row()].second;
    }
    void clearPreview();
    void clearModel();
    void setPreviewCase(int x, int y);
    void setPreviewCase(const QPair<int, int> &pos);
    void setTargetTeleport(const QPair<int, int> &teleporterPosition, const QModelIndex &targetPos);
    int memWallShape(WallDrawMode_e wallShape, const QPair<int, int> &topLeftIndex, const QPair<int, int> &bottomRightIndex, const QString &iniId);
    void updateWallNumber(uint32_t num);
    inline QPair<int, int> getTableSize()const
    {
        return m_tableSize;
    }
private:
    QVector<QPair<WallDrawMode_e, WallShapeData>> m_memWallShape;
    QPair<int, int> m_tableSize;
    QVector<QVector<QPair<QPixmap, std::optional<CaseData>>>> m_vectPic;
    QVector<QBitArray> m_vectPreview;
    std::optional<QPair<int, int>> m_departurePlayer;
signals:
    void editCompleted(const QString &str);
};
