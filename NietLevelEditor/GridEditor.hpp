#pragma once

#include <QDialog>
#include <limits>
#include "LevelDataManager.hpp"

class TableModel;
class QGroupBox;

enum class LevelElement_e
{
    WALL,
    DOOR,
    TRIGGER,
    TELEPORT,
    ENEMY,
    OBJECT,
    STATIC_CEILING,
    STATIC_GROUND,
    BARREL,
    EXIT,
    DELETE,
    TOTAL
};

inline const float EPSILON_FLOAT = std::numeric_limits<float>::epsilon();

namespace Ui {
class GridEditor;
}

class GridEditor : public QDialog
{
    Q_OBJECT
public:
    explicit GridEditor(QWidget *parent = nullptr);
    bool initGrid(const QString &installDir, int levelWidth, int levelHeight);
    ~GridEditor();
private:
    void adjustTableSize();
    void setStdTableSize();
    void initSelectableWidgets();
    void loadIconPictures(const QString &installDir);
    QPixmap getSprite(const ArrayFloat_t &spriteData, const QString &installDir);
private slots:
    void setElementSelected(LevelElement_e num);
private:
    Ui::GridEditor *ui;
    LevelDataManager m_levelDataManager;
    TableModel *m_tableModel;
    const int32_t CASE_SIZE_PX = 50;
    LevelElement_e m_currentElement;
    std::array<QVector<QIcon>, static_cast<uint32_t>(LevelElement_e::TOTAL)> m_drawData;
};

QString getStringFromLevelElementEnum(LevelElement_e num);
