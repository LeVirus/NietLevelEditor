#include "BackgroundForm.hpp"
#include "ui_BackgroundForm.h"
#include <QRadioButton>
#include <iostream>

//======================================================================
BackgroundForm::BackgroundForm(const IconArray_t &pictureData, QWidget *parent) :
    QDialog(parent),
    m_arrayIcons(pictureData),
    ui(new Ui::BackgroundForm)
{
    ui->setupUi(this);
    confWidgets();
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
    BackgroundData &currentBackground = (m_ceilingMode) ? m_ceilingBackground : m_groundBackground;
    currentBackground.m_displayMode = m_displayMode;
    if(ui->colorContainer->isEnabled())
    {
        currentBackground.m_colorData = std::array<std::array<float, 4>, 4>();
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
    close();
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
    (*currentBackground.m_colorData)[firstIndex][secondIndex] = qobject_cast<const QDoubleSpinBox*>(widget)->value();
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
void BackgroundForm::confWidgets()
{
    for(uint32_t i = 0; i < m_arrayIcons.size(); ++i)
    {
        for(int32_t j = 0; j < m_arrayIcons[i].size(); ++j)
        {
            if(!m_arrayIcons[i][j].first.isEmpty())
            {
                ui->spriteSimpleTextureComboBox->addItem(m_arrayIcons[i][j].second, m_arrayIcons[i][j].first);
                ui->spriteTiledTextureComboBox->addItem(m_arrayIcons[i][j].second, m_arrayIcons[i][j].first);
            }
        }
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

