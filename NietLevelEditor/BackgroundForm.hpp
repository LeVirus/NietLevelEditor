#pragma once

#include <QDialog>
#include "GridEditor.hpp"

namespace Ui {
class BackgroundForm;
}

enum class BackgroundDisplayMode_e
{
    COLOR,
    TILED_TEXTURE,
    SIMPLE_TEXTURE,
    COLOR_AND_TILED_TEXTURE,
    SIMPLE_TEXTURE_AND_TILED_TEXTURE,
    NONE
};

struct BackgroundData
{
    //order :: top left, top right, bottom right, bottom left
    std::array<std::array<float, 4>, 4> m_colorData;
    QString m_simpleTexture, m_tiledTexture;
    BackgroundDisplayMode_e m_displayMode;
};

class BackgroundForm : public QDialog
{
    Q_OBJECT
public:
    explicit BackgroundForm(const std::map<QString, ArrayFloat_t> &iconData, QWidget *parent = nullptr);
    inline void confCeilingOrGroundMode(bool ceiling)
    {
        m_ceilingMode = ceiling;
    }
    inline const BackgroundData &getGroundData()const
    {
        return m_groundBackground;
    }
    inline const BackgroundData &getCeilingData()const
    {
        return m_ceilingBackground;
    }
    void modifBackgroundDisplayMode(BackgroundDisplayMode_e mode);
    void unckeckAll();
    bool backgroundSetted();
    void setBackgroundData(const BackgroundData &background, bool ground);
    ~BackgroundForm();
private slots:
    void modifDisplayModeColor(bool toggled);
    void modifDisplayModeTiledTexture(bool toggled);
    void modifDisplayModeSimpleTexture(bool toggled);
    void modifDisplayModeColorAndTiledTexture(bool toggled);
    void modifDisplayModeSimpleTextureAndTiledTexture(bool toggled);
    void confirmForm();
    void memColorCase(const QObject *widget);
private:
    void confWidgets(const std::map<QString, ArrayFloat_t> &iconData);
private:
    Ui::BackgroundForm *ui;
    QString m_pictureDirectory;
    bool m_ceilingMode;
    BackgroundDisplayMode_e m_displayMode;
    BackgroundData m_groundBackground, m_ceilingBackground;
    bool m_ceilingSet = false, m_groundSet = false;
};
