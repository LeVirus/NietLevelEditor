#pragma once

#include <map>
#include <QStringList>
#include <functional>
#include <optional>

using ArrayFloat_t = std::array<float, 5>;
class QSettings;

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
private:
    void clear();
    inline bool spriteExists(const QString &sprite)
    {
        return (m_memPictureElement.find(sprite) != m_memPictureElement.end());
    }
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
    std::map<QString, QString> m_triggerElement, m_teleportElement;
};

