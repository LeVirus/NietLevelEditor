#include "BackgroundForm.hpp"
#include "ui_BackgroundForm.h"
#include <QRadioButton>

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
void BackgroundForm::unckeckAll()
{
    m_displayMode = BackgroundDisplayMode_e::NONE;
    ui->coloredRadioButton->setChecked(false);
    ui->simpleTextRadioButton->setChecked(false);
    ui->tiledTextRadioButton->setChecked(false);
    ui->simpleTextureAndTiledTextRadioButton->setChecked(false);
    ui->colorAndTiledTextRadioButton->setChecked(false);
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
                ui->spriteSimpleTextureComboBox->addItem(m_arrayIcons[i][j].second, "");
                ui->spriteTiledTextureComboBox->addItem(m_arrayIcons[i][j].second, "");
            }
        }
    }
    QObject::connect(ui->coloredRadioButton, &QRadioButton::toggled, this, &BackgroundForm::modifDisplayModeColor);
    QObject::connect(ui->simpleTextRadioButton, &QRadioButton::toggled, this, &BackgroundForm::modifDisplayModeSimpleTexture);
    QObject::connect(ui->tiledTextRadioButton, &QRadioButton::toggled, this, &BackgroundForm::modifDisplayModeTiledTexture);
    QObject::connect(ui->simpleTextureAndTiledTextRadioButton, &QRadioButton::toggled,
                     this, &BackgroundForm::modifDisplayModeSimpleTextureAndTiledTexture);
    QObject::connect(ui->colorAndTiledTextRadioButton, &QRadioButton::toggled, this, &BackgroundForm::modifDisplayModeColorAndTiledTexture);
    unckeckAll();
}

