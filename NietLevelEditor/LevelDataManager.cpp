#include "LevelDataManager.hpp"
#include <QSettings>
#include <QFile>
#include <QRegularExpression>
#include <iostream>
#include "BackgroundForm.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include "TableModel.hpp"

//======================================================================
LevelDataManager::LevelDataManager()
{

}

//======================================================================
bool LevelDataManager::loadLevelData(const QString &installDir)
{
    m_installDirectory = installDir;
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
std::optional<ArrayFloat_t> LevelDataManager::getPictureData(const QString &sprite)const
{
    std::map<QString, ArrayFloat_t>::const_iterator it = m_memPictureElement.find(sprite);
    if(it != m_memPictureElement.end())
    {
        return it->second;
    }
    return {};
}

//======================================================================
void LevelDataManager::generateLevel(const TableModel &tableModel, const QString &musicFilename,
                                     const QPair<BackgroundData const*, BackgroundData const*> &backgroundData, Direction_e playerDirection)
{
    if(!tableModel.checkLevelData())
    {
        QMessageBox::warning(nullptr, "Error", "Player departure and Exit have to be defined");
        return;
    }
    QFileDialog dialog;
    QString selfilter = "INI (*.ini)", filename;
    filename = dialog.getSaveFileName(nullptr, "Level file selection", m_installDirectory + "/Ressources/", "INI (*.ini)", &selfilter);
    if(m_INIFile)
    {
        delete m_INIFile;
    }
    m_INIFile = new QSettings(filename, QSettings::NativeFormat);
    //Level
    m_INIFile->setValue("Level/weight", tableModel.getTableSize().first);
    m_INIFile->setValue("Level/height", tableModel.getTableSize().second);
    if(musicFilename != "None")
    {
        m_INIFile->setValue("Level/music", musicFilename);
    }
    //Background
    loadBackgroundData(backgroundData);
    //PlayerInit
    m_INIFile->setValue("PlayerInit/playerDepartureX", tableModel.getPlayerDepartureData()->first);
    m_INIFile->setValue("PlayerInit/playerDepartureY", tableModel.getPlayerDepartureData()->second);
    m_INIFile->setValue("PlayerInit/PlayerOrientation", static_cast<int>(playerDirection));
    //Wall
    generateWallIniLevel(tableModel);
}

//======================================================================
void LevelDataManager::generateWallIniLevel(const TableModel &tableModel)
{
    QString key, gamePos, removePos;
    std::map<QString, WallDataINI> memWallData;
    const WallDataContainer_t &wallData = tableModel.getWallData();
    uint32_t cmpt = 0;
    for(int i = 0; i < wallData.size(); ++i)
    {
        removePos = "";
        gamePos = "";
        if(wallData[i].second.m_memMoveData)
        {
            key = "MoveableWall" + QString::number(cmpt);
            ++cmpt;
        }
        else
        {
            key = wallData[i].second.m_iniId;
        }
        int sizeX, sizeY;
        if(wallData[i].second.m_wallCount == 0)
        {
            continue;
        }
        if(wallData[i].second.m_wallCount == 1 && wallData[i].second.m_gridCoordTopLeft == wallData[i].second.m_gridCoordBottomRight)
        {
            gamePos += "3 " + QString::number(wallData[i].second.m_gridCoordTopLeft.first) + " " +
                    QString::number(wallData[i].second.m_gridCoordTopLeft.second) + "  ";
            continue;
        }
        switch(wallData[i].first)
        {
        case WallDrawShape_e::DIAGONAL_LINE:
        {
            sizeY = wallData[i].second.m_wallCount;
            sizeX = sizeY;
            gamePos = wallData[i].second.m_diagCaseUp ? "6 ": "5 ";
            gamePos += QString::number(wallData[i].second.m_gridCoordTopLeft.first) + " " +
                    QString::number(wallData[i].second.m_gridCoordTopLeft.second) + " ";
            gamePos += QString::number(sizeX) + "  ";
            break;
        }
        case WallDrawShape_e::DIAGONAL_RECT:
        {
            sizeY = wallData[i].second.m_wallCount / 2 + 1;
            sizeX = sizeY;
            gamePos += "4 " + QString::number(wallData[i].second.m_gridCoordTopLeft.first) + " " +
                    QString::number(wallData[i].second.m_gridCoordTopLeft.second) +
                    " " + QString::number(sizeX) + " " + QString::number(sizeY) + "  ";
            break;
        }
        case WallDrawShape_e::LINE_AND_RECT:
        {
            sizeX = std::abs(wallData[i].second.m_gridCoordTopLeft.first - wallData[i].second.m_gridCoordBottomRight.first) + 1,
            sizeY = std::abs(wallData[i].second.m_gridCoordTopLeft.second - wallData[i].second.m_gridCoordBottomRight.second) + 1;
            //VERTICAL LINE
            if(sizeX == 1)
            {
                gamePos += "1 " + QString::number(wallData[i].second.m_gridCoordTopLeft.first) + " " +
                        QString::number(wallData[i].second.m_gridCoordTopLeft.second) + " " + QString::number(sizeY) + "  ";
            }
            //LATERAL LINE
            else if(sizeY == 1)
            {
                gamePos += "2 " + QString::number(wallData[i].second.m_gridCoordTopLeft.first) + " " +
                        QString::number(wallData[i].second.m_gridCoordTopLeft.second) + " " + QString::number(sizeX) + "  ";
            }
            //RECT
            else
            {
                gamePos += "0 " + QString::number(wallData[i].second.m_gridCoordTopLeft.first) + " " +
                        QString::number(wallData[i].second.m_gridCoordTopLeft.second) +
                        " " + QString::number(sizeX) + " " + QString::number(sizeY) + "  ";
            }
            break;
        }
        }
        for(int32_t j = 0; j < wallData[i].second.m_deletedWall.size(); ++j)
        {
            removePos += QString::number(wallData[i].second.m_deletedWall[j].first) + " " +
                    QString::number(wallData[i].second.m_deletedWall[j].second) + " " + QString::number(3) + "  ";
        }
        std::map<QString, WallDataINI>::iterator it = memWallData.find(key);
        if(it == memWallData.end())
        {
            if(wallData[i].second.m_memMoveData)
            {
                memWallData.insert({key, WallDataINI{gamePos, removePos, wallData[i].second.m_iniId,
                                          std::make_unique<MoveWallData>()}});
                *memWallData[key].m_moveableData = *wallData[i].second.m_memMoveData;
            }
            else
            {
                memWallData.insert({key, WallDataINI{gamePos, removePos, {}, {}}});
            }
        }
        else
        {
            memWallData[key].m_position += gamePos;
            memWallData[key].m_removePosition += removePos;
        }
    }
}

//======================================================================
void LevelDataManager::loadBackgroundData(const BackgroundPairData_t &backgroundData)
{
    assert(m_INIFile);
    //GROUND====
    BackgroundDisplayMode_e mode = backgroundData.first->m_displayMode;
    //COLOR
    if(mode == BackgroundDisplayMode_e::COLOR || mode == BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE)
    {
        assert(backgroundData.first->m_colorData);
        QString colorR, colorG, colorB;
        for(uint32_t i = 0; i < 4; ++i)
        {
            QString::number((*backgroundData.first->m_colorData)[0][i]);
            colorR += QString::number((*backgroundData.first->m_colorData)[i][0]) + " ";
            colorG += QString::number((*backgroundData.first->m_colorData)[i][1]) + " ";
            colorB += QString::number((*backgroundData.first->m_colorData)[i][2]) + " ";
        }
        m_INIFile->setValue("ColorGroundBackground/colorR", colorR);
        m_INIFile->setValue("ColorGroundBackground/colorG", colorG);
        m_INIFile->setValue("ColorGroundBackground/colorB", colorB);
    }
    //SIMPLE TEXTURE
    else if(mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE || mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE)
    {
        m_INIFile->setValue("SimpleTextureGroundBackground/sprite", backgroundData.first->m_simpleTexture);
    }
    //TILED TEXTURE
    else if(mode == BackgroundDisplayMode_e::TILED_TEXTURE || mode == BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE ||
            mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE)
    {
        m_INIFile->setValue("TiledTextureGroundBackground/sprite", backgroundData.first->m_simpleTexture);
    }

    //CEILING====
    mode = backgroundData.second->m_displayMode;
    //COLOR
    if(mode == BackgroundDisplayMode_e::COLOR || mode == BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE)
    {
        assert(backgroundData.second->m_colorData);
        QString colorR, colorG, colorB;
        for(uint32_t i = 0; i < 4; ++i)
        {
            QString::number((*backgroundData.second->m_colorData)[0][i]);
            colorR += QString::number((*backgroundData.second->m_colorData)[i][0]) + " ";
            colorG += QString::number((*backgroundData.second->m_colorData)[i][1]) + " ";
            colorB += QString::number((*backgroundData.second->m_colorData)[i][2]) + " ";
        }
        m_INIFile->setValue("ColorCeilingBackground/colorR", colorR);
        m_INIFile->setValue("ColorCeilingBackground/colorG", colorG);
        m_INIFile->setValue("ColorCeilingBackground/colorB", colorB);
    }
    //SIMPLE TEXTURE
    else if(mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE || mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE)
    {
        m_INIFile->setValue("SimpleTextureCeilingBackground/sprite", backgroundData.second->m_simpleTexture);
    }
    //TILED TEXTURE
    else if(mode == BackgroundDisplayMode_e::TILED_TEXTURE || mode == BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE ||
            mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE)
    {
        m_INIFile->setValue("TiledTextureCeilingBackground/sprite", backgroundData.second->m_simpleTexture);
    }
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
           array[0] = m_pictureDataINI->value(keysList.at(i) + "/texture", 1).toInt();
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
    QString str = m_pictureDataINI->value("PathToTexture/textures", "").toString();
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
            ok = loadTriggerData(keysList.at(i));
        }
        else if(keysList.at(i).contains("Enemy"))
        {
            ok = loadEnemyData(keysList.at(i));
        }
        else if(keysList.at(i).contains("Object"))
        {
            ok = loadObjectData(keysList.at(i));
        }
        else if(keysList.at(i).contains("Ground"))
        {
            ok = loadStaticElementGroundData(keysList.at(i));
        }
        else if(keysList.at(i).contains("Ceiling"))
        {
            ok = loadStaticElementCeilingData(keysList.at(i));
        }
        else if(keysList.at(i) == "Teleport")
        {
            ok = loadTeleportData(keysList.at(i));
        }
        else if(keysList.at(i) == "Barrel")
        {
            ok = loadBarrelData(keysList.at(i));
        }
        else if(keysList.at(i) == "Exit")
        {
            ok = loadExitData(keysList.at(i));
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
    if(!checkListSpriteExist(strList))
    {
        return false;
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
        m_doorElement[key].m_cardID = m_INIFile->value(cardID + "/Sprite", "").toString();
    }
    return true;
}

//======================================================================
bool LevelDataManager::loadTriggerData(const QString &key)
{
    QString sprite = m_INIFile->value(key + "/Sprite", "").toString();
    if(sprite.isEmpty())
    {
        return false;
    }
    m_triggerElement.insert({key, sprite});
    return true;
}

//======================================================================
bool LevelDataManager::loadEnemyData(const QString &key)
{
    QString sprites = m_INIFile->value(key + "/StaticSpriteFront", "").toString();
    if(sprites.isEmpty())
    {
        return false;
    }
    QStringList strList = sprites.split(QRegularExpression("[,\\s]+"));
    if(!checkListSpriteExist(strList))
    {
        return false;
    }
    m_enemyElement.insert({key, strList.at(0)});
    return true;
}

//======================================================================
bool LevelDataManager::loadObjectData(const QString &key)
{
    QString sprite = m_INIFile->value(key + "/Sprite", "").toString();
    if(sprite.isEmpty())
    {
        return false;
    }
    m_objectElement.insert({key, sprite});
    return true;
}

//======================================================================
bool LevelDataManager::loadStaticElementGroundData(const QString &key)
{
    QString sprite = m_INIFile->value(key + "/Sprite", "").toString();
    if(sprite.isEmpty())
    {
        return false;
    }
    m_staticGroundElement.insert({key, sprite});
    return true;
}

//======================================================================
bool LevelDataManager::loadStaticElementCeilingData(const QString &key)
{
    QString sprite = m_INIFile->value(key + "/Sprite", "").toString();
    if(sprite.isEmpty())
    {
        return false;
    }
    m_staticCeilingElement.insert({key, sprite});
    return true;
}

//======================================================================
bool LevelDataManager::loadTeleportData(const QString &key)
{
    QString sprites = m_INIFile->value(key + "/Sprite", "").toString();
    if(sprites.isEmpty())
    {
        return false;
    }
    m_teleportElement.insert({key, sprites});
    return true;
}

//======================================================================
bool LevelDataManager::loadBarrelData(const QString &key)
{
    QString sprites = m_INIFile->value(key + "/StaticSprite", "").toString();
    if(sprites.isEmpty())
    {
        return false;
    }
    QStringList strList = sprites.split(QRegularExpression("[,\\s]+"));
    if(!checkListSpriteExist(strList))
    {
        return false;
    }
    m_barrelElement.insert({key, strList.at(0)});
    return true;
}

//======================================================================
bool LevelDataManager::loadExitData(const QString &key)
{
    QString sprites = m_INIFile->value(key + "/Sprite", "").toString();
    if(sprites.isEmpty())
    {
        return false;
    }
    m_exitElement.insert({key, sprites});
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
bool LevelDataManager::checkListSpriteExist(const QStringList &strList)const
{
    for(int32_t i = 0; i < strList.size(); ++i)
    {
        if(!spriteExists(strList.at(i)))
        {
            return false;
        }
    }
    return true;
}

//======================================================================
LevelDataManager::~LevelDataManager()
{
    clear();
}
