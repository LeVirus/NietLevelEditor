#include "LevelDataManager.hpp"
#include <QSettings>
#include <QFile>
#include <QRegularExpression>
#include <iostream>
#include <fstream>
#include <sstream>
#include "BackgroundForm.hpp"
#include <QFileDialog>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QTextStream>
#include "TableModel.hpp"

//======================================================================
bool LevelDataManager::loadLevelData(const QString &installDir)
{
    m_installDirectory = installDir;
    QString standardDataINI = installDir + "/Ressources/standardData.ini", pictureDataINI = installDir + "/Ressources/pictureData.ini";
    clear();
    std::optional<QTemporaryFile*> standardData = loadEncryptedINIFile(standardDataINI, ENCRYPTION_KEY_CONF_FILE);
    if(!standardData || !(*standardData))
    {
        return false;
    }
    m_INIFile = new QSettings((*standardData)->fileName(), QSettings::IniFormat);
    assert(m_INIFile->childGroups().size() > 0);
    if(m_INIFile->status() != QSettings::Status::NoError)
    {
        delete *standardData;
        return false;
    }
    std::optional<QTemporaryFile*> pictureData = loadEncryptedINIFile(pictureDataINI, ENCRYPTION_KEY_CONF_FILE);
    if(!pictureData || !(*pictureData))
    {
        delete *standardData;
        delete *pictureData;
        return false;
    }
    m_pictureDataINI = new QSettings((*pictureData)->fileName(), QSettings::IniFormat);
    delete *standardData;
    delete *pictureData;
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
bool LevelDataManager::loadExistingLevel(const QString &levelFilePath)
{
    if(m_installDirectory.isEmpty())
    {
        return false;
    }
    std::optional<QTemporaryFile*> levelData = loadEncryptedINIFile(levelFilePath, ENCRYPTION_KEY_STANDARD_LEVEL);
    if(!levelData || !(*levelData))
    {
        return false;
    }
    QSettings levelFile((*levelData)->fileName(), QSettings::IniFormat);
    //Level
    m_existingLevelData = std::make_unique<LevelData>();
    QVariant varA = levelFile.value("Level/weight", -1), varB = levelFile.value("Level/height", -1);
    if(varA.toInt() == -1 || varB.toInt() == -1)
    {
        return false;
    }
    m_existingLevelData->m_levelSize = {varA.toInt(), varB.toInt()};
    varA = levelFile.value("Level/music");
    if(!varA.isNull())
    {
        m_existingLevelData->m_music = varA.toString();
    }
    //Background
    if(!loadBackgroundLevel(true, levelFile))
    {
        return false;
    }
    if(!loadBackgroundLevel(false, levelFile))
    {
        return false;
    }
    //PlayerInit
    varA = levelFile.value("PlayerInit/playerDepartureX", -1), varB = levelFile.value("PlayerInit/playerDepartureY", -1);
    if(varA.toInt() == -1 || varB.toInt() == -1)
    {
        return false;
    }
    m_existingLevelData->m_playerDeparture = {varA.toInt(), varB.toInt()};
    varA = levelFile.value("PlayerInit/PlayerOrientation", -1);
    int playerOrientation = varA.toInt();
    if(playerOrientation < 0 || playerOrientation > 3)
    {
        return false;
    }
    m_existingLevelData->m_playerDirection = static_cast<Direction_e>(playerOrientation);
    //Wall
    if(!loadWallLevel(levelFile))
    {
        return false;
    }
    //Teleport
    if(!loadTeleportLevel(levelFile))
    {
        return false;
    }
    //Barrel
    if(!loadStandardElementLevel(levelFile, StandardElement_e::BARREL))
    {
        return false;
    }
    //Enemy
    if(!loadStandardElementLevel(levelFile, StandardElement_e::ENEMY))
    {
        return false;
    }
    //Door
    if(!loadStandardElementLevel(levelFile, StandardElement_e::DOOR))
    {
        return false;
    }
    //Exit
    if(!loadStandardElementLevel(levelFile, StandardElement_e::EXIT))
    {
        return false;
    }
    //Object
    if(!loadStandardElementLevel(levelFile, StandardElement_e::OBJECT))
    {
        return false;
    }
    //Ceiling
    if(!loadStandardElementLevel(levelFile, StandardElement_e::STATIC_CEILING_ELEMENT))
    {
        return false;
    }
    //Ground
    if(!loadStandardElementLevel(levelFile, StandardElement_e::STATIC_GROUND_ELEMENT))
    {
        return false;
    }
    return true;
}

//======================================================================
bool LevelDataManager::loadStandardElementLevel(const QSettings &ini, StandardElement_e elementType)
{
    std::multimap<QString, QPair<int, int>> *currentMap;
    QString str;
    switch(elementType)
    {
    case StandardElement_e::BARREL:
        str = "Barrel";
        currentMap = &m_existingLevelData->m_barrelsData;
        break;
    case StandardElement_e::DOOR:
        str = "Door";
        currentMap = &m_existingLevelData->m_doorsData;
        break;
    case StandardElement_e::ENEMY:
        str = "Enemy";
        currentMap = &m_existingLevelData->m_enemiesData;
        break;
    case StandardElement_e::EXIT:
        str = "Exit";
        currentMap = &m_existingLevelData->m_exitData;
        break;
    case StandardElement_e::OBJECT:
        str = "Object";
        currentMap = &m_existingLevelData->m_objectsData;
        break;
    case StandardElement_e::STATIC_CEILING_ELEMENT:
        str = "Ceiling";
        currentMap = &m_existingLevelData->m_ceilingElementsData;
        break;
    case StandardElement_e::STATIC_GROUND_ELEMENT:
        str = "Ground";
        currentMap = &m_existingLevelData->m_groundElementsData;
        break;
    }
    QStringList keys = ini.childGroups(), list;
    QString pos;
    for(int i = 0; i < keys.size(); ++i)
    {
        if(keys[i].contains(str))
        {
            pos = ini.value(keys[i] + "/GamePosition", "").toString();
            list = pos.split(" ");
            if(list.isEmpty())
            {
                return false;
            }
            for(int j = 0; j < list.size(); j += 2)
            {
                if(list[j].isEmpty())
                {
                    ++j;
                    if(j >= list.size() - 1)
                    {
                        break;
                    }
                }
                else if(j >= list.size() - 1)
                {
                    return false;
                }
                currentMap->insert({keys[i], {list[j].toInt(), list[j + 1].toInt()}});
            }
        }
    }
    return true;
}

//======================================================================
bool LevelDataManager::loadWallLevel(const QSettings &ini)
{
    QStringList keys = ini.childGroups();
    for(int i = 0; i < keys.size(); ++i)
    {
        if(keys[i].contains("WallShape"))
        {
            m_existingLevelData->m_wallsData.insert({keys[i], WallDataINI()});
            m_existingLevelData->m_wallsData[keys[i]].m_position = ini.value(keys[i] + "/GamePosition", "").toString();
            m_existingLevelData->m_wallsData[keys[i]].m_removePosition = ini.value(keys[i] + "/RemovePosition", "").toString();
            //init wall
            if(!generateStructPosWall(keys[i], true))
            {
                return false;
            }
            //removed wall
            generateStructPosWall(keys[i], false);
            if(m_existingLevelData->m_wallsData[keys[i]].m_position.isEmpty())
            {
                return false;
            }
            QVariant varA = ini.value(keys[i] + "/Direction", "");
            m_existingLevelData->m_wallsData[keys[i]].m_iniID = ini.value(keys[i] + "/WallDisplayID", "").toString();
            if(varA.toString().isEmpty())
            {
                continue;
            }
            m_existingLevelData->m_wallsData[keys[i]].m_moveableData = std::make_unique<MoveWallData>();
            QVariant varB = ini.value(keys[i] + "/NumberOfMove", "");
            QStringList listDir, listMoveNumber;
            QString strDir = varA.toString(), strMoveNumber = varB.toString();
            listDir = strDir.split(' ');
            listMoveNumber= strMoveNumber.split(' ');
            if(listDir.isEmpty() || listMoveNumber.isEmpty() || listDir.size() != listMoveNumber.size())
            {
                return false;
            }
            for(int j = 0; j < listDir.size(); ++j)
            {
                m_existingLevelData->m_wallsData[keys[i]].m_moveableData->m_memMoveWallData.push_back(
                {static_cast<Direction_e>(listDir[j].toInt()), listMoveNumber[j].toInt()});
            }
            m_existingLevelData->m_wallsData[keys[i]].m_moveableData->m_velocity = ini.value(keys[i] + "/Velocity", -1).toInt();

            m_existingLevelData->m_wallsData[keys[i]].m_moveableData->m_triggerType =
                    static_cast<TriggerType_e>(ini.value(keys[i] + "/TriggerType", -1).toInt());
            m_existingLevelData->m_wallsData[keys[i]].m_moveableData->m_triggerBehaviour =
                    static_cast<TriggerBehaviourType_e>(ini.value(keys[i] + "/TriggerBehaviourType", -1).toInt());

            if(m_existingLevelData->m_wallsData[keys[i]].m_moveableData->m_triggerType == TriggerType_e::DISTANT_SWITCH ||
                    m_existingLevelData->m_wallsData[keys[i]].m_moveableData->m_triggerType == TriggerType_e::GROUND)
            {
                m_existingLevelData->m_wallsData[keys[i]].m_moveableData->m_triggerINISectionName =
                        ini.value(keys[i] + "/TriggerDisplayID", "").toString();
                QString str = ini.value(keys[i] + "/TriggerGamePosition", "").toString();
                listDir = str.split(' ');
                if(listDir.size() != 2)
                {
                    return false;
                }
                m_existingLevelData->m_wallsData[keys[i]].m_moveableData->m_triggerPos = {listDir[0].toInt(), listDir[1].toInt()};
            }
        }
    }
    return true;
}

//======================================================================
bool LevelDataManager::loadTeleportLevel(const QSettings &ini)
{
    QStringList keys = ini.childGroups(), teleportPos, targetPos;
    QString teleportPosStr, targetPosStr;
    for(int i = 0; i < keys.size(); ++i)
    {
        if(keys[i].contains("Teleport"))
        {
            teleportPosStr = ini.value(keys[i] + "/PosA", "").toString();
            teleportPos = teleportPosStr.split(' ');
            targetPosStr = ini.value(keys[i] + "/PosB", "").toString();
            targetPos = targetPosStr.split(' ');
            if(teleportPos.size() != 2 || targetPos.size() != 2)
            {
                return false;
            }
            m_existingLevelData->m_teleportData.insert({keys[i], {{teleportPos[0].toInt(), teleportPos[1].toInt()},
                                                                  {targetPos[0].toInt(), targetPos[1].toInt()}}});
        }
    }
    return true;
}

//======================================================================
bool LevelDataManager::generateStructPosWall(const QString &key, bool positionMode)
{
    QString baseStr = m_existingLevelData->m_wallsData[key].m_position;
    QVector<QPair<WallDrawShape_e, WallShapeData>> *currentContainer;
    if(positionMode)
    {
        baseStr = m_existingLevelData->m_wallsData[key].m_position;
        m_existingLevelData->m_wallsData[key].m_vectPos = std::make_unique<QVector<QPair<WallDrawShape_e, WallShapeData>>>();
        currentContainer = m_existingLevelData->m_wallsData[key].m_vectPos.get();
    }
    else
    {
        baseStr = m_existingLevelData->m_wallsData[key].m_removePosition;
        m_existingLevelData->m_wallsData[key].m_vectRem = std::make_unique<QVector<QPair<WallDrawShape_e, WallShapeData>>>();
        currentContainer = m_existingLevelData->m_wallsData[key].m_vectRem.get();
    }
    if(baseStr.isEmpty())
    {
        if(positionMode)
        {
            QMessageBox::warning(nullptr, "Error", "Wall loading failed : baseStr.isEmpty. ");
        }
        return false;
    }
    QStringList listA = baseStr.split("  "), listB;
    if(listA.empty())
    {
        QMessageBox::warning(nullptr, "Error", "Wall loading failed : listA.empty.");
        return false;
    }
    currentContainer->reserve(listA.size());
    for(int i = 0; i < listA.size(); ++i)
    {
        currentContainer->push_back({});
        listB = listA[i].split(' ');
        if(listB.empty())
        {
            QMessageBox::warning(nullptr, "Error", "Wall loading failed : listB.empty.");
            return false;
        }
        QPair<WallDrawShape_e, WallShapeData> &currentPair = currentContainer->back();
        currentPair.second.m_iniId = key;
        if(listB.size() < 3)
        {
            QMessageBox::warning(nullptr, "Error", "Wall loading failed : listB.size() < 3.");
            return false;
        }
        //Origin
        currentPair.second.m_gridCoordTopLeft = {listB[1].toInt(), listB[2].toInt()};
        switch(listB[0].toUInt())
        {
        //Rect
        case 0:
        {
            currentPair.first = WallDrawShape_e::LINE_AND_RECT;
            currentPair.second.m_gridCoordBottomRight = {listB[1].toInt() + listB[3].toInt() - 1, listB[2].toInt() + listB[4].toInt() - 1};
            break;
        }
        //Vertical Line
        case 1:
        {
            currentPair.first = WallDrawShape_e::LINE_AND_RECT;
            currentPair.second.m_gridCoordBottomRight = {listB[1].toInt(), listB[2].toInt() + listB[3].toInt() - 1};
            break;
        }
        //Horizontal Line
        case 2:
        {
            currentPair.first = WallDrawShape_e::LINE_AND_RECT;
            currentPair.second.m_gridCoordBottomRight = {listB[1].toInt() + listB[3].toInt() - 1, listB[2].toInt()};
            break;
        }
        //Point
        case 3:
        {
            currentPair.first = WallDrawShape_e::LINE_AND_RECT;
            currentPair.second.m_gridCoordBottomRight = currentPair.second.m_gridCoordTopLeft;
            break;
        }
        //diag rect
        case 4:
        {
            currentPair.first = WallDrawShape_e::DIAGONAL_RECT;
            currentPair.second.m_gridCoordBottomRight = {listB[1].toInt() + listB[3].toInt() - 1, listB[2].toInt() + listB[3].toInt() - 1};
            break;
        }
        //diag origins up left
        case 5:
        {
            currentPair.second.m_diagCaseUp = true;
            currentPair.first = WallDrawShape_e::DIAGONAL_LINE;
            currentPair.second.m_gridCoordBottomRight = {listB[1].toInt() + listB[3].toInt() - 1, listB[2].toInt() + listB[3].toInt() - 1};
            break;
        }
        //diag origins down left
        case 6:
        {
            currentPair.second.m_diagCaseUp = false;
            currentPair.first = WallDrawShape_e::DIAGONAL_LINE;
            currentPair.second.m_gridCoordBottomRight = {listB[1].toInt() + listB[3].toInt() - 1, listB[2].toInt() - (listB[3].toInt() - 1)};
            break;
        }
        default:
            assert(false);
        }
        if(positionMode)
        {
            if(listB[0].toUInt() == 3)
            {
                currentPair.second.m_baseWallCount = 1;
            }
            else if(listB[0].toUInt() == 4)
            {
                currentPair.second.m_baseWallCount = listB[3].toInt() * 2 - 2;
            }
            else
            {
                currentPair.second.m_baseWallCount = listB[3].toInt();
            }
            currentPair.second.m_currentWallCount = currentPair.second.m_baseWallCount;
        }
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
std::optional<QPair<int, int>> LevelDataManager::getLoadedLevelSize()const
{
    if(!m_existingLevelData)
    {
        return {};
    }
    return m_existingLevelData->m_levelSize;
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
    QFile file(filename);
    if(file.exists())
    {
        file.remove();
    }
    m_ini.clear();
    m_ini.setValue("Level", "weight", std::to_string(tableModel.getTableSize().first));
    m_ini.setValue("Level", "height", std::to_string(tableModel.getTableSize().second));
    if(musicFilename != "None")
    {
        m_ini.setValue("Level", "music", musicFilename.toStdString());
    }
    loadBackgroundData(backgroundData);
    m_ini.setValue("PlayerInit", "playerDepartureX", std::to_string(tableModel.getPlayerDepartureData()->first));
    m_ini.setValue("PlayerInit", "playerDepartureY", std::to_string(tableModel.getPlayerDepartureData()->second));
    m_ini.setValue("PlayerInit", "PlayerOrientation", std::to_string(static_cast<int>(playerDirection)));
    m_ini.setValue("Exit", "GamePosition", QString::number(tableModel.getExitData().begin()->second.first).toStdString() + " " +
                        QString::number(tableModel.getExitData().begin()->second.second).toStdString());
    generateWallsIniLevel(tableModel);
    generateTeleportsIniLevel(tableModel);
    generateStandardIniLevel(tableModel.getDoorsData());
    generateStandardIniLevel(tableModel.getEnemiesData());
    generateStandardIniLevel(tableModel.getObjectsData());
    generateStandardIniLevel(tableModel.getStaticCeilingData());
    generateStandardIniLevel(tableModel.getStaticGroundData());
    generateStandardIniLevel(tableModel.getBarrelsData());
    std::stringstream stringStream;
    std::string str;
    std::ofstream outputStream;
    outputStream.open(filename.toStdString());
    m_ini.generate(stringStream);
    str = encrypt(stringStream.str(), ENCRYPTION_KEY_STANDARD_LEVEL);
    outputStream << str;
    outputStream.close();
}

//===================================================================
std::string encrypt(const std::string &str, uint32_t key)
{
    std::string strR = str;
    for(uint32_t i = 0; i < strR.size(); ++i)
    {
        strR[i] += key;
    }
    return strR;
}

//======================================================================
std::optional<QTemporaryFile*> LevelDataManager::loadEncryptedINIFile(const QString &filePath, uint32_t encryptKey)
{
    std::ifstream stream;
    stream.open(filePath.toStdString());
    if(stream.fail())
    {
        stream.close();
        return {};
    }
    std::ostringstream ostringStream;
    ostringStream << stream.rdbuf();
    stream.close();
    std::string dataString = ostringStream.str();
    //decrypt
    for(uint32_t i = 0; i < dataString.size(); ++i)
    {
        dataString[i] -= encryptKey;
    }
    QTemporaryFile *tmpFile = new QTemporaryFile(".fake.ini");
    if(!tmpFile->open())
    {
        return {};
    }
    tmpFile->write(dataString.c_str());
    tmpFile->close();
    return tmpFile;
}

//======================================================================
bool LevelDataManager::loadBackgroundLevel(bool ground, const QSettings &ini)
{
    QString id = ground ? "Ground" : "Ceiling";
    if(!m_existingLevelData->m_backgroundData)
    {
        m_existingLevelData->m_backgroundData = std::make_unique<QPair<BackgroundData, BackgroundData>>();
    }
    BackgroundData &currentBackground = ground ? m_existingLevelData->m_backgroundData->first : m_existingLevelData->m_backgroundData->second;
    BackgroundDisplayMode_e mode = BackgroundDisplayMode_e::NONE;
    QVariant varA;
    //SIMPLE
    varA = ini.value("SimpleTexture" + id + "Background/sprite");
    if(!varA.isNull())
    {
        currentBackground.m_simpleTexture = varA.toString();
        mode = BackgroundDisplayMode_e::SIMPLE_TEXTURE;
    }
    else
    {
        //COLOR
        varA = ini.value("Color" + id + "Background/colorR");
        if(!varA.isNull())
        {
            QVariant varB = ini.value("Color" + id + "Background/colorG"), varC = ini.value("Color" + id + "Background/colorB");
            QStringList listA = varA.toString().split(' '), listB = varB.toString().split(' '), listC = varC.toString().split(' ');
            if(listA.size() != 4 || listB.size() != 4 || listC.size() != 4)
            {
                return false;
            }
            for(int i = 0; i < 4; ++i)
            {
                currentBackground.m_colorData[i][0] = listA[i].toFloat();
                currentBackground.m_colorData[i][1] = listB[i].toFloat();
                currentBackground.m_colorData[i][2] = listC[i].toFloat();
                currentBackground.m_colorData[i][3] = 0.0f;
            }
            mode = BackgroundDisplayMode_e::COLOR;
        }
    }
    //TILED
    varA = ini.value("TiledTexture" + id + "Background/sprite");
    if(!varA.isNull())
    {
        currentBackground.m_tiledTexture = varA.toString();
        if(mode == BackgroundDisplayMode_e::NONE)
        {
            mode = BackgroundDisplayMode_e::TILED_TEXTURE;
        }
        else if(mode == BackgroundDisplayMode_e::COLOR)
        {
            mode = BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE;
        }
        else if(mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE)
        {
            mode = BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE;
        }
    }
    currentBackground.m_displayMode = mode;
    return true;
}

//======================================================================
void LevelDataManager::generateWallsIniLevel(const TableModel &tableModel)
{
    QString key, gamePos, removePos;
    std::map<QString, WallDataINI> memWallData;
    const WallDataContainer_t &wallData = tableModel.getWallData();
    uint32_t cmpt = 0;
    for(int i = 0; i < wallData.size(); ++i)
    {
        if(wallData[i].second.m_currentWallCount == 0)
        {
            continue;
        }
        key = getWallINIKey(cmpt);
        ++cmpt;
        gamePos = getIniWallPos(i, wallData);
        removePos = getCurrentWallRemovedINI(i, wallData);
        if(wallData[i].second.m_memMoveData)
        {
            memWallData.insert({key, WallDataINI{gamePos, removePos, nullptr, nullptr, wallData[i].second.m_iniId,
                                                 std::make_unique<MoveWallData>()}});
            *memWallData[key].m_moveableData = *wallData[i].second.m_memMoveData;
        }
        else
        {
            memWallData.insert({key, WallDataINI{gamePos, removePos, nullptr, nullptr, wallData[i].second.m_iniId, {}}});
        }
    }
    writeWallData(memWallData);
}

//======================================================================
QString getWallINIKey(int shapeWallNum)
{
    QString zeros;
    if(shapeWallNum < 10)
    {
        zeros = "0000";
    }
    else if(shapeWallNum < 100)
    {
        zeros = "000";
    }
    else if(shapeWallNum < 1000)
    {
        zeros = "00";

    }
    else if(shapeWallNum < 10000)
    {
        zeros = "0";
    }
    return "WallShape" + zeros + QString::number(shapeWallNum);
}

//======================================================================
void LevelDataManager::generateTeleportsIniLevel(const TableModel &tableModel)
{
    const std::multimap<QString, TeleportData> teleportData = tableModel.getTeleporterData();
    if(teleportData.empty())
    {
        return;
    }
    QString pos, target, biDirection;
    for(std::multimap<QString, TeleportData>::const_iterator it = teleportData.begin(); it != teleportData.end(); ++it)
    {
        pos += QString::number(it->second.m_teleporterPos.first) + " " + QString::number(it->second.m_teleporterPos.second) + "  ";
        target += QString::number(it->second.m_targetPos.first) + " " + QString::number(it->second.m_targetPos.second) + "  ";
        biDirection += "0 ";
    }
    m_ini.setValue(teleportData.begin()->first.toStdString(), "PosA", formatToIniFile(pos).toStdString());
    m_ini.setValue(teleportData.begin()->first.toStdString(), "PosB", formatToIniFile(target).toStdString());
    m_ini.setValue(teleportData.begin()->first.toStdString(), "BiDirection", formatToIniFile(biDirection).toStdString());
}

//======================================================================
void LevelDataManager::generateStandardIniLevel(const std::multimap<QString, QPair<int, int> > &datas)
{
    QString pos;
    std::map<QString, QString> mapINI;
    std::map<QString, QString>::iterator itt;
    for(std::multimap<QString, QPair<int, int>>::const_iterator it = datas.begin(); it != datas.end(); ++it)
    {
        pos = QString::number(it->second.first) + " " + QString::number(it->second.second) + "  ";
        itt = mapINI.find(it->first);
        if(itt == mapINI.end())
        {
            mapINI.insert({it->first, pos});
        }
        else
        {
            mapINI[it->first] += pos;
        }
    }
    for(std::map<QString, QString>::const_iterator it = mapINI.begin(); it != mapINI.end(); ++it)
    {
        m_ini.setValue(it->first.toStdString(), "GamePosition", formatToIniFile(it->second).toStdString());
    }
}

//======================================================================
QString LevelDataManager::getIniWallPos(int index, const WallDataContainer_t &wallData)const
{
    int sizeX, sizeY;
    QString gamePos;
    if(wallData[index].second.m_currentWallCount == 1 && wallData[index].second.m_gridCoordTopLeft == wallData[index].second.m_gridCoordBottomRight)
    {
        gamePos = "3 " + QString::number(wallData[index].second.m_gridCoordTopLeft.first) + " " +
                QString::number(wallData[index].second.m_gridCoordTopLeft.second) + "  ";
    }
    else
    {
        switch(wallData[index].first)
        {
        case WallDrawShape_e::DIAGONAL_LINE:
        {
            sizeY = wallData[index].second.m_baseWallCount;
            sizeX = sizeY;
            gamePos = wallData[index].second.m_diagCaseUp ? "6 ": "5 ";
            gamePos += QString::number(wallData[index].second.m_gridCoordTopLeft.first) + " " +
                    QString::number(wallData[index].second.m_gridCoordTopLeft.second) + " ";
            gamePos += QString::number(sizeX) + "  ";
            break;
        }
        case WallDrawShape_e::DIAGONAL_RECT:
        {
            sizeX = wallData[index].second.m_baseWallCount / 2 + 1;
            gamePos += "4 " + QString::number(wallData[index].second.m_gridCoordTopLeft.first) + " " +
                    QString::number(wallData[index].second.m_gridCoordTopLeft.second) +
                    " " + QString::number(sizeX) + "  ";
            break;
        }
        case WallDrawShape_e::LINE_AND_RECT:
        {
            sizeX = std::abs(wallData[index].second.m_gridCoordTopLeft.first - wallData[index].second.m_gridCoordBottomRight.first) + 1,
                    sizeY = std::abs(wallData[index].second.m_gridCoordTopLeft.second - wallData[index].second.m_gridCoordBottomRight.second) + 1;
            //VERTICAL LINE
            if(sizeX == 1)
            {
                gamePos += "1 " + QString::number(wallData[index].second.m_gridCoordTopLeft.first) + " " +
                        QString::number(wallData[index].second.m_gridCoordTopLeft.second) + " " + QString::number(sizeY) + "  ";
            }
            //LATERAL LINE
            else if(sizeY == 1)
            {
                gamePos += "2 " + QString::number(wallData[index].second.m_gridCoordTopLeft.first) + " " +
                        QString::number(wallData[index].second.m_gridCoordTopLeft.second) + " " + QString::number(sizeX) + "  ";
            }
            //RECT
            else
            {
                gamePos += "0 " + QString::number(wallData[index].second.m_gridCoordTopLeft.first) + " " +
                        QString::number(wallData[index].second.m_gridCoordTopLeft.second) +
                        " " + QString::number(sizeX) + " " + QString::number(sizeY) + "  ";
            }
            break;
        }
        }
    }
    return gamePos;
}

//======================================================================
void LevelDataManager::writeWallData(const std::map<QString, WallDataINI> &wallData)
{
    QString strDir, strMoveNumber;
    std::string section;
    for(std::map<QString, WallDataINI>::const_iterator it = wallData.begin(); it != wallData.end(); ++it)
    {
        section = it->first.toStdString();
        m_ini.setValue(section, "GamePosition", formatToIniFile(it->second.m_position).toStdString());
        if(!it->second.m_removePosition.isEmpty())
        {
            m_ini.setValue(section, "RemovePosition", formatToIniFile(it->second.m_removePosition).toStdString());
        }
        assert(it->second.m_iniID);
        m_ini.setValue(section, "WallDisplayID", (*it->second.m_iniID).toStdString());
        if(it->second.m_moveableData)
        {
            strDir = "";
            strMoveNumber = "";
            for(int i = 0; i < it->second.m_moveableData->m_memMoveWallData.size(); ++i)
            {
                strDir += QString::number(static_cast<int>(it->second.m_moveableData->m_memMoveWallData[i].first)) + " ";
                strMoveNumber += QString::number(it->second.m_moveableData->m_memMoveWallData[i].second) + " ";
            }
            m_ini.setValue(section, "Direction", formatToIniFile(strDir).toStdString());
            m_ini.setValue(section, "NumberOfMove", formatToIniFile(strMoveNumber).toStdString());
            m_ini.setValue(section, "Velocity", std::to_string(it->second.m_moveableData->m_velocity));
            m_ini.setValue(section, "TriggerBehaviourType",
                           std::to_string(static_cast<int>(it->second.m_moveableData->m_triggerBehaviour)));
            if(it->second.m_moveableData->m_triggerBehaviour != TriggerBehaviourType_e::AUTO)
            {
                m_ini.setValue(section, "TriggerType", std::to_string(static_cast<int>(it->second.m_moveableData->m_triggerType)));
                if(it->second.m_moveableData->m_triggerType != TriggerType_e::WALL)
                {
                    m_ini.setValue(section, "TriggerGamePosition", std::to_string(it->second.m_moveableData->m_triggerPos->first) +
                                        " " + std::to_string(it->second.m_moveableData->m_triggerPos->second));
                }
                if(it->second.m_moveableData->m_triggerType == TriggerType_e::DISTANT_SWITCH)
                {
                    m_ini.setValue(section, "TriggerDisplayID", it->second.m_moveableData->m_triggerINISectionName.toStdString());
                }
            }
        }
    }
}

//======================================================================
QString LevelDataManager::getCurrentWallRemovedINI(int index, const WallDataContainer_t &wallData)const
{
    QString str;
    for(int i = 0; i < wallData[index].second.m_deletedWall.size(); ++i)
    {
        str += "3 " + QString::number(wallData[index].second.m_deletedWall[i].first) + " " +
                QString::number(wallData[index].second.m_deletedWall[i].second) + "  ";
    }
    return str;
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
        QString colorR, colorG, colorB;
        for(uint32_t i = 0; i < 4; ++i)
        {
            QString::number(backgroundData.second->m_colorData[0][i]);
            colorR += QString::number(backgroundData.second->m_colorData[i][0]) + " ";
            colorG += QString::number(backgroundData.second->m_colorData[i][1]) + " ";
            colorB += QString::number(backgroundData.second->m_colorData[i][2]) + " ";
        }
        m_ini.setValue("ColorGroundBackground", "colorR", formatToIniFile(colorR).toStdString());
        m_ini.setValue("ColorGroundBackground", "colorG", formatToIniFile(colorG).toStdString());
        m_ini.setValue("ColorGroundBackground", "colorB", formatToIniFile(colorB).toStdString());
    }
    //SIMPLE TEXTURE
    if(mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE || mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE)
    {
        m_ini.setValue("SimpleTextureGroundBackground", "sprite", backgroundData.first->m_simpleTexture.toStdString());
    }
    //TILED TEXTURE
    if(mode == BackgroundDisplayMode_e::TILED_TEXTURE || mode == BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE ||
            mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE)
    {
        m_ini.setValue("TiledTextureGroundBackground", "sprite", backgroundData.first->m_tiledTexture.toStdString());
    }

    //CEILING====
    mode = backgroundData.second->m_displayMode;
    //COLOR
    if(mode == BackgroundDisplayMode_e::COLOR || mode == BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE)
    {
        QString colorR, colorG, colorB;
        for(uint32_t i = 0; i < 4; ++i)
        {
            QString::number(backgroundData.second->m_colorData[0][i]);
            colorR += QString::number(backgroundData.second->m_colorData[i][0]) + " ";
            colorG += QString::number(backgroundData.second->m_colorData[i][1]) + " ";
            colorB += QString::number(backgroundData.second->m_colorData[i][2]) + " ";
        }
        m_ini.setValue("ColorCeilingBackground", "colorR", formatToIniFile(colorR).toStdString());
        m_ini.setValue("ColorCeilingBackground", "colorG", formatToIniFile(colorG).toStdString());
        m_ini.setValue("ColorCeilingBackground", "colorB", formatToIniFile(colorB).toStdString());
    }
    //SIMPLE TEXTURE
    if(mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE || mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE)
    {
        m_ini.setValue("SimpleTextureCeilingBackground", "sprite", backgroundData.second->m_simpleTexture.toStdString());
    }
    //TILED TEXTURE
    if(mode == BackgroundDisplayMode_e::TILED_TEXTURE || mode == BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE ||
            mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE)
    {
        m_ini.setValue("TiledTextureCeilingBackground", "sprite", backgroundData.second->m_tiledTexture.toStdString());
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
    bool ok = false;
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

//======================================================================
QString formatToIniFile(const QString &str)
{
    QString copy = str;
    if(copy.isEmpty())
    {
        return copy;
    }
    while(copy.back() == ' ')
    {
        copy = copy.remove(copy.size() - 1, 1);
        if(copy.isEmpty())
        {
            return copy;
        }
    }
    while(copy[0] == ' ')
    {
        copy = copy.remove(0, 1);
        if(copy.isEmpty())
        {
            return copy;
        }
    }
    return copy;
}
