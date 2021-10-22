#pragma once

#include <QMap>
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
    bool loadPictureDataINI();
    bool loadStandardDataINI();
private:
    QSettings *m_standardSettingINI = nullptr, *m_pictureDataINI = nullptr;
    QString m_installDirectory;
    QStringList m_texturesPath;
    QMap<QString, ArrayFloat_t> m_memPictureElement;
};

