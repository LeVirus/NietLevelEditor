#pragma once

#include <map>
#include <QStringList>
#include <functional>
#include <optional>
#include <memory>
#include <QFile>
#include <QVector>
#include <includeDir/iniwriter.h>
#include "GlobalLevelConfForm.hpp"

using ArrayFloat_t = std::array<float, 5>;
class QSettings;
class TableModel;
struct BackgroundData;
struct MoveWallData;
struct WallShapeData;
struct TeleportData;

enum class WallDrawShape_e;

using WallDataContainer_t = QVector<QPair<WallDrawShape_e, WallShapeData>>;
using BackgroundPairData_t = QPair<const BackgroundData*, const BackgroundData*>;
enum class Direction_e;
enum class LevelElement_e;

enum class StandardElement_e
{
    DOOR,
    STATIC_CEILING_ELEMENT,
    STATIC_GROUND_ELEMENT,
    EXIT,
    BARREL,
    ENEMY,
    OBJECT
};

struct DoorData
{
    QString m_sprite;
    bool m_vertical;
    std::optional<QString> m_cardID;
};

struct WallDataINI
{
    QString m_position, m_removePosition;
    std::unique_ptr<QVector<QPair<WallDrawShape_e, WallShapeData>>> m_vectPos, m_vectRem;
    //Moveable data
    std::optional<QString> m_iniID;
    std::unique_ptr<MoveWallData> m_moveableData;
};

struct LogData
{
    QPair<int, int> m_position;
    QString m_message, m_displayID;
};

struct LevelData
{
    uint32_t m_levelNum;
    QPair<int, int> m_levelSize;
    std::optional<QString> m_music, m_prologueText, m_epilogueText, m_epilogueMusic;
    //first ground
    std::unique_ptr<QPair<BackgroundData, BackgroundData>> m_backgroundData;
    QPair<int, int> m_playerDeparture;
    std::optional<QPair<QString, QPair<int, int>>> m_endLevelEnemyPos;
    Direction_e m_playerDirection;
    QVector<QPair<int, int>> m_secrets;
    QVector<QPair<QPair<int, int>, Direction_e>> m_checkpoints;
    std::map<QString, WallDataINI> m_wallsData;
    std::multimap<QString, TeleportData> m_teleportData;
    std::map<QString, LogData> m_logsData;
    std::multimap<QString, QPair<int, int>> m_exitData, m_barrelsData, m_groundElementsData, m_ceilingElementsData,
    m_enemiesData, m_objectsData, m_doorsData;
};

class LevelDataManager
{
public:
    LevelDataManager() = default;
    ~LevelDataManager();
    bool loadLevelData(const QString &installDir);
    bool loadExistingLevel(const QString &levelFilePath);
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
    inline const std::map<QString, QString> &getCardData()const
    {
        return m_cardElement;
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
    inline const std::map<QString, QString> &getLogData()const
    {
        return m_logElement;
    }
    inline const QStringList &getTexturePaths()const
    {
        return m_texturesPath;
    }
    inline const std::map<QString, ArrayFloat_t> &getSpriteData()const
    {
        return m_memPictureElement;
    }
    inline LevelData const *getExistingLevel()const
    {
        return m_existingLevelData.get();
    }
    std::optional<ArrayFloat_t> getPictureData(const QString &sprite)const;
    std::optional<QPair<int, int>> getLoadedLevelSize()const;
    void generateLevel(const TableModel &tableModel, const QString &musicFilename,
                       const BackgroundPairData_t &backgroundData, Direction_e playerDirection, const GlobalLevelData &globalLevelData);
private:
    std::optional<QTemporaryFile *> loadEncryptedINIFile(const QString &filePath, uint32_t encryptKey);
    bool loadBackgroundLevel(bool ground, const QSettings &ini);
    bool loadStandardElementLevel(const QSettings &ini, StandardElement_e elementType);
    bool loadWallLevel(const QSettings &ini);
    bool loadBasicSecretsElementLevel(const QSettings &ini);
    bool loadBasicCheckpointsElementLevel(const QSettings &ini);
    bool loadTeleportLevel(const QSettings &ini);
    bool loadLogElementLevel(std::string_view levelPath);
    bool generateStructPosWall(const QString &key, bool positionMode);
    void generateWallsIniLevel(const TableModel &tableModel);
    void generateTeleportsIniLevel(const TableModel &tableModel);
    void generateEnemiesIniLevel(const TableModel &tableModel);
    void generateStandardIniLevel(const std::multimap<QString, QPair<int, int>> &datas, const std::optional<QPair<int, int>> &endLevelEnemyPos = {});
    void generateCheckpointElementsIniLevel(const QVector<QPair<QPair<int, int>, Direction_e>> &datas);
    void generateLogsElementsIniLevel(const QVector<LogData> &datas);
    void generateSecretsElementsIniLevel(const QVector<QPair<int, int>> &datas);
    void writeWallData(const std::map<QString, WallDataINI> &wallData);
    QString getCurrentWallRemovedINI(int index, const WallDataContainer_t &wallData)const;
    QString getIniWallPos(int index, const WallDataContainer_t &wallData) const;
    void loadBackgroundData(const BackgroundPairData_t &backgroundData);
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
    bool loadLogData(const QString &key);
    bool loadBarrelData(const QString &key);
    bool loadExitData(const QString &key);
    bool loadCardData(const QString &key);
private:
    QSettings *m_INIFile = nullptr, *m_pictureDataINI = nullptr;
    QString m_installDirectory;
    QStringList m_texturesPath;
    std::map<QString, ArrayFloat_t> m_memPictureElement;
    std::map<QString, QStringList> m_wallElement;
    std::map<QString, DoorData> m_doorElement;
    std::map<QString, QString> m_triggerElement, m_teleportElement, m_enemyElement, m_objectElement, m_staticCeilingElement,
    m_staticGroundElement, m_barrelElement, m_exitElement, m_logElement, m_cardElement;
    std::unique_ptr<LevelData> m_existingLevelData;
    inipp::Ini<char> m_ini;
    const uint32_t ENCRYPTION_KEY_CONF_FILE = 42, ENCRYPTION_KEY_STANDARD_LEVEL = 17;
};

std::string encrypt(const std::string &str, uint32_t key);
std::string decrypt(const std::string &str, uint32_t key);
std::string treatStrEndLineSave(const std::string &str);
std::string treatStrEndLineLoad(const std::string &str);
QString getStrNumINIKey(int shapeWallNum);
QString formatToIniFile(const QString &str);
