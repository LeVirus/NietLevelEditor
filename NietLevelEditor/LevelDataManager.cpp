#include "LevelDataManager.hpp"
#include <QSettings>
#include <QFile>

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
    m_standardSettingINI = new QSettings(standardDataINI, QSettings::NativeFormat);
    if(m_standardSettingINI->status() != QSettings::Status::NoError)
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
           m_memPictureElement.insert(keysList.at(i), array);
       }
    }
    m_texturesPath = m_pictureDataINI->value("PathToTexture/textures", -1.0f).toStringList();
    if(m_texturesPath.isEmpty())
    {
        return false;
    }
    return true;
}

//======================================================================
bool LevelDataManager::loadStandardDataINI()
{
    const QStringList keysList = m_standardSettingINI->childGroups();
    for(int32_t i = 0; i < keysList.size(); ++i)
    {
       if(keysList.at(i).contains("Wall"))
       {

       }
       else if(keysList.at(i).contains("Door"))
       {

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
    }
    return true;
}

//======================================================================
void LevelDataManager::clear()
{
    if(m_standardSettingINI)
    {
        delete m_standardSettingINI;
        m_standardSettingINI = nullptr;
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
