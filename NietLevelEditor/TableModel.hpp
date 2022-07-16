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
    QString m_triggerINISectionName;
    void clear()
    {
        m_memMoveWallCases.clear();
        m_memMoveWallData.clear();
        m_triggerPos = {};
    }
    MoveWallData& operator=(const MoveWallData& other)
    {
        if (this == &other)
        {
            return *this;
        }
        m_memMoveWallCases = other.m_memMoveWallCases;
        m_memMoveWallData = other.m_memMoveWallData;
        m_velocity = other.m_velocity;
        m_triggerType = other.m_triggerType;
        m_triggerBehaviour = other.m_triggerBehaviour;
        m_triggerType = other.m_triggerType;
        m_triggerINISectionName = other.m_triggerINISectionName;
        if(other.m_triggerPos)
        {
            m_triggerPos = *other.m_triggerPos;
        }
        return *this;
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
    std::optional<bool> m_endLevelEnemy;
};

struct WallShapeData
{
    //!!!diag case top left respresent ORIGIN POINT!!!
    QPair<int, int> m_gridCoordTopLeft, m_gridCoordBottomRight;
    uint32_t m_baseWallCount = 0, m_currentWallCount = 0;
    QVector<QPair<int, int>> m_deletedWall;
    QString m_iniId;
    std::optional<MoveWallData> m_memMoveData;
    bool m_diagCaseUp;
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
    bool setIdData(const QModelIndex &index, const CaseData &value, bool endLevelEnemyCase = false);
    void removeData(const QModelIndex &index, bool dontMemRemovedWall = false);
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
    void setTableWallDiagCaseConf(QPair<int, int> originPoint, bool directionUp);
    void setTableWallDiagRectCaseConf(QPair<int, int> originPoint);
    inline QPair<int, int> getTableSize()const
    {
        return m_tableSize;
    }
    inline int getLastWallCount()const
    {
        if(m_memWallShape.empty())
        {
            return -1;
        }
        return m_memWallShape.back().second.m_baseWallCount;
    }
    bool checkLevelData()const;
    inline const WallDataContainer_t &getWallData()const
    {
        return m_memWallShape;
    }
    inline const std::optional<QPair<int, int>> &getEndLevelEnemyPos()const
    {
        return m_levelEndEnemy;
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
    inline const QVector<QPair<QPair<int, int>, Direction_e>> &getCheckpointsData()const
    {
        return m_vectCheckpoints;
    }
    inline const QVector<LogData> &getLogData()const
    {
        return m_memLog;
    }
    inline const QVector<QPair<int, int>> &getSecretsData()const
    {
        return m_vectSecrets;
    }
    inline Direction_e getPlayerDirectionDeparture()const
    {
        return m_playerDirectionDeparture;
    }
    inline void setPlayerDirectionDeparture(Direction_e dir)
    {
        m_playerDirectionDeparture = dir;
    }
    std::optional<LogData> getLogDataPos(const QPair<int, int> &pos)const;
    void addCheckpoint(const QPair<int, int> &pos, const QPair<uint32_t, Direction_e> &checkpointData);
    void addLog(const QPair<int, int> &pos, const QString &message, const QString &displayID);
    inline void addSecret(const QPair<int, int> &pos)
    {
        m_vectSecrets.push_back(pos);
    }
    inline uint32_t getNumberOfCheckpoints()
    {
        return m_vectCheckpoints.size();
    }
    std::optional<QPair<uint32_t, Direction_e>> getCheckpointData(const QPair<int, int> &pos) const;
    void updateTriggerPos(const QPair<int, int> &pos);
    void removeTrigger(CaseData &triggerCase, const QPair<int, int> &triggercoord);
private:
    void rmStdElement(const QPair<int, int> &pos, LevelElement_e elementType);
    void removeCheckpoint(const QPair<int, int> &pos);
    void removeSecret(const QPair<int, int> &pos);
private:
    WallDataContainer_t m_memWallShape;
    std::multimap<QString, QPair<int, int>> m_memEnemy, m_memBarrel, m_memDoor, m_memExit, m_memObject, m_memStaticCeiling,
    m_memStaticGround;
    QVector<LogData> m_memLog;
    std::multimap<QString, TeleportData> m_memTeleport;
    QPair<int, int> m_tableSize;
    QVector<QVector<QPair<QPixmap, std::optional<CaseData>>>> m_vectPic;
    QVector<QBitArray> m_vectPreview;
    std::optional<QPair<int, int>> m_departurePlayer, m_exitPos, m_levelEndEnemy;
    QVector<QPair<QPair<int, int>, Direction_e>> m_vectCheckpoints;
    QVector<QPair<int, int>> m_vectSecrets;
    Direction_e m_playerDirectionDeparture = Direction_e::NORTH;
signals:
    void editCompleted(const QString &str);
};
