#pragma once

#include <map>
#include <QStringList>
#include <functional>

using ArrayFloat_t = std::array<float, 5>;
class QSettings;

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
private:
    QSettings *m_standardSettingINI = nullptr, *m_pictureDataINI = nullptr;
    QString m_installDirectory;
    QStringList m_texturesPath;
    std::map<QString, ArrayFloat_t> m_memPictureElement;
    std::map<QString, QStringList> m_wallElement;
};

