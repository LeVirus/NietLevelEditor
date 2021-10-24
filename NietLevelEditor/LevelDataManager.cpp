#include "LevelDataManager.hpp"
#include <QSettings>
#include <QFile>
#include <QRegularExpression>
#include <iostream>

//======================================================================
LevelDataManager::LevelDataManager()
{

}

//======================================================================
bool LevelDataManager::loadLevelData(const QString &installDir)
{
    QString standardDataINI = installDir + "/Ressources/standardData.ini", pictureDataINI = installDir + "/Ressources/pictureData.ini";
    if(!QFile::exists(standardDataINI) || !QFile::exists(pictureDataINI))
    {
        return false;
    }
    clear();
    m_INIFile = new QSettings(standardDataINI, QSettings::NativeFormat);
    if(m_INIFile->status() != QSettings::Status::NoError)
    {
        return false;
    }
    m_pictureDataINI = new QSettings(pictureDataINI, QSettings::NativeFormat);
    if(m_pictureDataINI->status() != QSettings::Status::NoError)
    {
        return false;
    }
    if(!loadPictureDataINI() || !loadStandardDataINI())
    {
        return false;
    }
    return true;
}

//======================================================================
bool LevelDataManager::loadPictureDataINI()
{
    const QStringList keysList = m_pictureDataINI->childGroups();
    ArrayFloat_t array;
    for(int32_t i = 0; i < keysList.size(); ++i)
    {
       if(keysList.at(i).contains("Sprite"))
       {
           array[0] = m_pictureDataINI->value(keysList.at(i) + "/texture", -1.0f).toFloat();
           array[1] = m_pictureDataINI->value(keysList.at(i) + "/texturePosX", -1.0f).toFloat();
           array[2] = m_pictureDataINI->value(keysList.at(i) + "/texturePosY", -1.0f).toFloat();
           array[3] = m_pictureDataINI->value(keysList.at(i) + "/textureWeight", -1.0f).toFloat();
           array[4] = m_pictureDataINI->value(keysList.at(i) + "/textureHeight", -1.0f).toFloat();
           for(uint32_t j = 0; j < array.size(); ++j)
           {
               if(array[j] < -0.5f)
               {
                   return false;
               }
           }
           m_memPictureElement.insert({keysList.at(i), array});
       }
    }
    QString str = m_pictureDataINI->value("PathToTexture/textures", -1.0f).toString();
    m_texturesPath = str.split(QRegularExpression("[,\\s]+"));
    if(m_texturesPath.isEmpty())
    {
        return false;
    }
    return true;
}

//======================================================================
bool LevelDataManager::loadStandardDataINI()
{
    const QStringList keysList = m_INIFile->childGroups();
    bool ok;
    for(int32_t i = 0; i < keysList.size(); ++i)
    {
        ok = true;
        if(keysList.at(i).contains("Wall"))
        {
            ok = loadWallData(keysList.at(i));
        }
        else if(keysList.at(i).contains("Door"))
        {
            ok = loadDoorData(keysList.at(i));
        }
        else if(keysList.at(i).contains("Trigger"))
        {

        }
        else if(keysList.at(i).contains("Enemy"))
        {

        }
        else if(keysList.at(i).contains("Object"))
        {

        }
        else if(keysList.at(i).contains("Ground"))
        {

        }
        else if(keysList.at(i).contains("Ceiling"))
        {

        }
        else if(keysList.at(i) == "Teleport")
        {

        }
        else if(keysList.at(i) == "Barrel")
        {

        }
        else if(keysList.at(i) == "Exit")
        {

        }
        if(!ok)
        {
            return false;
        }
    }
    return true;
}

//======================================================================
bool LevelDataManager::loadWallData(const QString &key)
{
    QString sprites = m_INIFile->value(key + "/Sprite", "").toString();
    if(sprites.isEmpty())
    {
        return false;
    }
    QStringList strList = sprites.split(QRegularExpression("[,\\s]+"));
    for(int32_t i = 0; i < strList.size(); ++i)
    {
        if(!spriteExists(strList.at(i)))
        {
            return false;
        }
    }
    m_wallElement.insert({key, strList});
    return true;
}

//======================================================================
bool LevelDataManager::loadDoorData(const QString &key)
{
    m_doorElement.insert({key, {}});
    m_doorElement[key].m_sprite = m_INIFile->value(key + "/Sprite", "").toString();
    if(m_doorElement[key].m_sprite.isEmpty())
    {
        return false;
    }
    m_doorElement[key].m_vertical = m_INIFile->value(key + "/Vertical", false).toBool();
    QString cardID = m_INIFile->value(key + "/CardID", "").toString();
    if(!cardID.isEmpty())
    {
        m_doorElement[key].m_cardID = cardID;
    }
    return true;
}

//======================================================================
void LevelDataManager::clear()
{
    if(m_INIFile)
    {
        delete m_INIFile;
        m_INIFile = nullptr;
    }
    if(m_pictureDataINI)
    {
        delete m_pictureDataINI;
        m_pictureDataINI = nullptr;
    }
    m_memPictureElement.clear();
    m_texturesPath.clear();
}

//======================================================================
LevelDataManager::~LevelDataManager()
{
    clear();
}
