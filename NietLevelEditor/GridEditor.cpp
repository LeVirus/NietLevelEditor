#include "GridEditor.hpp"
#include "ui_GridEditor.h"
#include <QTextStream>
#include <QSettings>
#include <QFile>
#include <iostream>

//======================================================================
GridEditor::GridEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GridEditor)
{
    ui->setupUi(this);
}

//======================================================================
bool GridEditor::initGrid(const QString &installDir)
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
    if(!loadStandardDataINI() || !loadPictureDataINI())
    {
        return false;
    }
    return true;
}

//======================================================================
bool GridEditor::loadPictureDataINI()
{
    const QStringList keysList = m_pictureDataINI->childGroups();
    ArrayFloat_t array;
    std::cerr << keysList.size() << "\n";
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
    return true;
}

//======================================================================
bool GridEditor::loadStandardDataINI()
{
    return true;
}

//======================================================================
void GridEditor::clear()
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
}

//======================================================================
GridEditor::~GridEditor()
{
    clear();
    delete ui;
}

