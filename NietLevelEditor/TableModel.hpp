#pragma once

#include <QAbstractTableModel>
#include <QString>
#include <QVector>
#include <QBitArray>
#include <QSet>
#include <map>
#include "GridEditor.hpp"
#include "MoveableWallForm.hpp"

struct MoveWallData
{
    QVector<QPair<int, int>> m_memMoveWallCases;
    QVector<QPair<Direction_e, int>> m_memMoveWallData;
    int m_velocity;
    TriggerType_e m_triggerType;
    TriggerBehaviourType_e m_triggerBehaviour;
    std::optional<QPair<int, int>> m_triggerPos;
    void clear()
    {
        m_memMoveWallCases.clear();
        m_memMoveWallData.clear();
        m_triggerPos = {};
    }
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
    std::optional<MoveWallData> m_memMoveData;
};

using WallDataContainer_t = QVector<QPair<WallDrawShape_e, WallShapeData>>;

struct TeleportData
{
    QPair<int, int> m_teleporterPos, m_targetPos;
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
    void removeData(const QModelIndex &index);
    inline std::optional<CaseData> &getDataElementCase(const QModelIndex &index)
    {
        return m_vectPic[index.column()][index.row()].second;
    }
    void clearPreview();
    void clearModel();
    void setPreviewCase(int x, int y);
    void setPreviewCase(const QPair<int, int> &pos);
    void setTargetTeleport(const QPair<int, int> &teleporterPosition, const QModelIndex &targetPos);
    int memWallShape(WallDrawShape_e wallShape, const QPair<int, int> &topLeftIndex, const QPair<int, int> &bottomRightIndex,
                     const QString &iniId, const MoveWallData *memMoveData);
    void memStdElement(const QPair<int, int> &pos, LevelElement_e elementType, const QString &iniId);
    void memTeleportElement(const QPair<int, int> &teleporterPos, const QPair<int, int> &targetPos, const QString &iniId);
    void updateWallNumber(uint32_t num);
    void setTableDeletionZone(const QPair<int, int> &originSelectPos, const QPair<int, int> &targetSelectPos, bool preview);
    inline QPair<int, int> getTableSize()const
    {
        return m_tableSize;
    }
    bool checkLevelData()const;
    inline const WallDataContainer_t &getWallData()const
    {
        return m_memWallShape;
    }
    inline const std::multimap<QString, TeleportData> &getTeleporterData()const
    {
        return m_memTeleport;
    }
    inline const std::optional<QPair<int, int>> &getPlayerDepartureData()const
    {
        return m_departurePlayer;
    }
    inline const std::multimap<QString, QPair<int, int>> &getEnemiesData()const
    {
        return m_memEnemy;
    }
    inline const std::multimap<QString, QPair<int, int>> &getBarrelsData()const
    {
        return m_memBarrel;
    }
    inline const std::multimap<QString, QPair<int, int>> &getDoorsData()const
    {
        return m_memDoor;
    }
    inline const std::multimap<QString, QPair<int, int>> &getExitData()const
    {
        return m_memExit;
    }
    inline const std::multimap<QString, QPair<int, int>> &getObjectsData()const
    {
        return m_memObject;
    }
    inline const std::multimap<QString, QPair<int, int>> &getStaticCeilingData()const
    {
        return m_memStaticCeiling;
    }
    inline const std::multimap<QString, QPair<int, int>> &getStaticGroundData()const
    {
        return m_memStaticGround;
    }
private:
    void rmStdElement(const QPair<int, int> &pos, LevelElement_e elementType);
private:
    WallDataContainer_t m_memWallShape;
    std::multimap<QString, QPair<int, int>> m_memEnemy, m_memBarrel, m_memDoor, m_memExit, m_memObject, m_memStaticCeiling,
    m_memStaticGround;
    std::multimap<QString, TeleportData> m_memTeleport;
    QPair<int, int> m_tableSize;
    QVector<QVector<QPair<QPixmap, std::optional<CaseData>>>> m_vectPic;
    QVector<QBitArray> m_vectPreview;
    std::optional<QPair<int, int>> m_departurePlayer, m_exitPos;
signals:
    void editCompleted(const QString &str);
};
