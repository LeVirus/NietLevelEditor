#include "BackgroundForm.hpp"
#include "ui_BackgroundForm.h"
#include <QRadioButton>
#include <QMessageBox>
#include <iostream>

//======================================================================
BackgroundForm::BackgroundForm(const std::map<QString, ArrayFloat_t> &iconData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BackgroundForm)
{
    ui->setupUi(this);
    confWidgets(iconData);
}

//======================================================================
BackgroundForm::~BackgroundForm()
{
    delete ui;
}

//======================================================================
void BackgroundForm::modifBackgroundDisplayMode(BackgroundDisplayMode_e mode)
{
    m_displayMode = mode;
    ui->colorContainer->setEnabled(mode == BackgroundDisplayMode_e::COLOR || mode == BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE);
    ui->spriteSimpleTextureComboBox->setEnabled(mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE ||
                                          mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE);
    ui->spriteTiledTextureComboBox->setEnabled(mode == BackgroundDisplayMode_e::TILED_TEXTURE ||
                                         mode == BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE ||
                                         mode == BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE);
}

//======================================================================
void BackgroundForm::modifDisplayModeColor(bool toggled)
{
    if(toggled)
    {
        modifBackgroundDisplayMode(BackgroundDisplayMode_e::COLOR);
    }
}

//======================================================================
void BackgroundForm::modifDisplayModeTiledTexture(bool toggled)
{
    if(toggled)
    {
        modifBackgroundDisplayMode(BackgroundDisplayMode_e::TILED_TEXTURE);
    }
}

//======================================================================
void BackgroundForm::modifDisplayModeSimpleTexture(bool toggled)
{
    if(toggled)
    {
        modifBackgroundDisplayMode(BackgroundDisplayMode_e::SIMPLE_TEXTURE);
    }
}

//======================================================================
void BackgroundForm::modifDisplayModeColorAndTiledTexture(bool toggled)
{
    if(toggled)
    {
        modifBackgroundDisplayMode(BackgroundDisplayMode_e::COLOR_AND_TILED_TEXTURE);
    }
}

//======================================================================
void BackgroundForm::modifDisplayModeSimpleTextureAndTiledTexture(bool toggled)
{
    if(toggled)
    {
        modifBackgroundDisplayMode(BackgroundDisplayMode_e::SIMPLE_TEXTURE_AND_TILED_TEXTURE);
    }
}

//======================================================================
void BackgroundForm::confirmForm()
{
    if(!ui->colorContainer->isEnabled() && !ui->spriteSimpleTextureComboBox->isEnabled() && !ui->spriteTiledTextureComboBox->isEnabled())
    {
        QMessageBox::warning(nullptr, "Error", "No background mode are selected.");
        return;
    }
    BackgroundData &currentBackground = (m_ceilingMode) ? m_ceilingBackground : m_groundBackground;
    currentBackground.m_displayMode = m_displayMode;
    if(ui->colorContainer->isEnabled())
    {
        for(int i = 0; i < ui->colorContainer->children().size(); ++i)
        {
            if(ui->colorContainer->children()[i]->objectName() == "layoutWidget")
            {
                for(int j = 0; j < ui->colorContainer->children()[i]->children().size(); ++j)
                {
                    memColorCase(ui->colorContainer->children()[i]->children()[j]);
                }
                break;
            }
        }
    }
    if(ui->spriteSimpleTextureComboBox->isEnabled())
    {
        currentBackground.m_simpleTexture = ui->spriteSimpleTextureComboBox->currentText();
    }
    if(ui->spriteTiledTextureComboBox->isEnabled())
    {
        currentBackground.m_tiledTexture = ui->spriteTiledTextureComboBox->currentText();
    }
    if(m_ceilingMode)
    {
        m_ceilingSet = true;
    }
    else
    {
        m_groundSet = true;
    }
    close();
}

//======================================================================
void BackgroundForm::setBackgroundData(const BackgroundData &background, bool ground)
{
    BackgroundData &currentBackground = (ground) ? m_groundBackground : m_ceilingBackground;
    currentBackground = background;
    if(ground)
    {
        m_groundSet = true;
    }
    else
    {
        m_ceilingSet = true;
    }
}

//======================================================================
void BackgroundForm::memColorCase(const QObject *widget)
{
    int firstIndex, secondIndex;
    if(widget->objectName().contains("TopLeft"))
    {
        firstIndex = 0;
    }
    else if(widget->objectName().contains("TopRight"))
    {
        firstIndex = 1;
    }
    else if(widget->objectName().contains("BottomRight"))
    {
        firstIndex = 2;
    }
    else if(widget->objectName().contains("BottomLeft"))
    {
        firstIndex = 3;
    }
    else
    {
        return;
    }
    if(widget->objectName().contains("TLRSpinBox"))
    {
        secondIndex = 0;
    }
    else if(widget->objectName().contains("TLGSpinBox"))
    {
        secondIndex = 1;
    }
    else if(widget->objectName().contains("TLBSpinBox"))
    {
        secondIndex = 2;
    }
    else if(widget->objectName().contains("TLASpinBox"))
    {
        secondIndex = 3;
    }
    else
    {
        return;
    }
    BackgroundData &currentBackground = (m_ceilingMode) ? m_ceilingBackground : m_groundBackground;
    currentBackground.m_colorData[firstIndex][secondIndex] = qobject_cast<const QDoubleSpinBox*>(widget)->value();
}

//======================================================================
void BackgroundForm::unckeckAll()
{
    m_displayMode = BackgroundDisplayMode_e::NONE;
    ui->coloredRadioButton->setAutoExclusive(false);
    ui->coloredRadioButton->setChecked(false);
    ui->coloredRadioButton->setAutoExclusive(true);
    ui->simpleTextRadioButton->setAutoExclusive(false);
    ui->simpleTextRadioButton->setChecked(false);
    ui->simpleTextRadioButton->setAutoExclusive(true);
    ui->tiledTextRadioButton->setAutoExclusive(false);
    ui->tiledTextRadioButton->setChecked(false);
    ui->tiledTextRadioButton->setAutoExclusive(true);
    ui->simpleTextureAndTiledTextRadioButton->setAutoExclusive(false);
    ui->simpleTextureAndTiledTextRadioButton->setChecked(false);
    ui->simpleTextureAndTiledTextRadioButton->setAutoExclusive(true);
    ui->colorAndTiledTextRadioButton->setAutoExclusive(false);
    ui->colorAndTiledTextRadioButton->setChecked(false);
    ui->colorAndTiledTextRadioButton->setAutoExclusive(true);
    ui->colorContainer->setEnabled(false);
    ui->spriteSimpleTextureComboBox->setEnabled(false);
    ui->spriteTiledTextureComboBox->setEnabled(false);
}

//======================================================================
bool BackgroundForm::backgroundSetted()
{
    return m_groundSet && m_ceilingSet;
}

//======================================================================
void BackgroundForm::confWidgets(const std::map<QString, ArrayFloat_t> &iconData)
{
    for(std::map<QString, ArrayFloat_t>::const_iterator it = iconData.begin(); it != iconData.end(); ++it)
    {
        ui->spriteSimpleTextureComboBox->addItem(it->first);
        ui->spriteTiledTextureComboBox->addItem(it->first);
    }
    QObject::connect(ui->coloredRadioButton, &QRadioButton::toggled, this, &BackgroundForm::modifDisplayModeColor);
    QObject::connect(ui->simpleTextRadioButton, &QRadioButton::toggled, this, &BackgroundForm::modifDisplayModeSimpleTexture);
    QObject::connect(ui->tiledTextRadioButton, &QRadioButton::toggled, this, &BackgroundForm::modifDisplayModeTiledTexture);
    QObject::connect(ui->simpleTextureAndTiledTextRadioButton, &QRadioButton::toggled,
                     this, &BackgroundForm::modifDisplayModeSimpleTextureAndTiledTexture);
    QObject::connect(ui->colorAndTiledTextRadioButton, &QRadioButton::toggled, this, &BackgroundForm::modifDisplayModeColorAndTiledTexture);
    QObject::connect(ui->OkButton, &QRadioButton::clicked, this, &BackgroundForm::confirmForm);
    QObject::connect(ui->CancelButton, &QRadioButton::clicked, this, &BackgroundForm::close);
    unckeckAll();
}

