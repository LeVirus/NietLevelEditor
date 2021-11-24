#pragma once

#include <map>
#include <QStringList>
#include <functional>
#include <optional>

using ArrayFloat_t = std::array<float, 5>;
class QSettings;
class TableModel;

struct DoorData
{
    QString m_sprite;
    bool m_vertical;
    std::optional<QString> m_cardID;
};

class LevelDataManager
{
public:
    LevelDataManager();
    ~LevelDataManager();
    bool loadLevelData(const QString &installDir);
    inline const std::map<QString, QStringList> &getWallData()const
    {
        return m_wallElement;
    }
    inline const std::map<QString, DoorData> &getDoorData()const
    {
        return m_doorElement;
    }
    inline const std::map<QString, QString> &getTriggerData()const
    {
        return m_triggerElement;
    }
    inline const std::map<QString, QString> &getTeleportData()const
    {
        return m_teleportElement;
    }
    inline const std::map<QString, QString> &getEnemyData()const
    {
        return m_enemyElement;
    }
    inline const std::map<QString, QString> &getObjectData()const
    {
        return m_objectElement;
    }
    inline const std::map<QString, QString> &getStaticCeilingData()const
    {
        return m_staticCeilingElement;
    }
    inline const std::map<QString, QString> &getStaticGroundData()const
    {
        return m_staticGroundElement;
    }
    inline const std::map<QString, QString> &getBarrelData()const
    {
        return m_barrelElement;
    }
    inline const std::map<QString, QString> &getExitData()const
    {
        return m_exitElement;
    }
    inline const QStringList &getTexturePaths()const
    {
        return m_texturesPath;
    }
    std::optional<ArrayFloat_t> getPictureData(const QString &sprite)const;
    void generateLevel(const TableModel &tableModel);
private:
    void clear();
    inline bool spriteExists(const QString &sprite)const
    {
        return (m_memPictureElement.find(sprite) != m_memPictureElement.end());
    }
    bool checkListSpriteExist(const QStringList &strList)const;
    bool loadPictureDataINI();
    bool loadStandardDataINI();
    bool loadWallData(const QString &key);
    bool loadDoorData(const QString &key);
    bool loadTriggerData(const QString &key);
    bool loadEnemyData(const QString &key);
    bool loadObjectData(const QString &key);
    bool loadStaticElementGroundData(const QString &key);
    bool loadStaticElementCeilingData(const QString &key);
    bool loadTeleportData(const QString &key);
    bool loadBarrelData(const QString &key);
    bool loadExitData(const QString &key);
private:
    QSettings *m_INIFile = nullptr, *m_pictureDataINI = nullptr;
    QString m_installDirectory;
    QStringList m_texturesPath;
    std::map<QString, ArrayFloat_t> m_memPictureElement;
    std::map<QString, QStringList> m_wallElement;
    std::map<QString, DoorData> m_doorElement;
    std::map<QString, QString> m_triggerElement, m_teleportElement, m_enemyElement, m_objectElement, m_staticCeilingElement,
    m_staticGroundElement, m_barrelElement, m_exitElement;
};

