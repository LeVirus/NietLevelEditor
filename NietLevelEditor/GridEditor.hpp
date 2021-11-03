#pragma once

#include <QIcon>
#include <QDialog>
#include <QModelIndex>
#include <limits>
#include "LevelDataManager.hpp"

class TableModel;
class QGroupBox;
class QItemSelection;
class EventFilter;

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

enum class WallDrawMode_e
{
    LINE_AND_RECT,
    DIAGONAL_LINE,
    DIAGONAL_RECT
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
    void connectSlots();
    void setStdTableSize();
    void initSelectableWidgets();
    void loadIconPictures(const QString &installDir);
    void loadWallsPictures(const QString &installDir);
    void loadDoorsPictures(const QString &installDir);
    void loadTriggersPictures(const QString &installDir);
    void loadTeleportsPictures(const QString &installDir);
    void loadEnemiesPictures(const QString &installDir);
    void loadObjectsPictures(const QString &installDir);
    void loadStaticCeilingElementPictures(const QString &installDir);
    void loadStaticGroundElementPictures(const QString &installDir);
    void loadBarrelsPictures(const QString &installDir);
    void loadExitsPictures(const QString &installDir);
    void setWallShape(bool preview = false);
    void setWallLineRectShape(const QPair<int, int> &topLeftIndex,
                              const QPair<int, int> &bottomRightIndex,
                              bool preview = false);
    void setWallDiagLineShape(const QPair<int, int> &topLeftIndex,
                              const QPair<int, int> &bottomRightIndex,
                              bool preview = false);
    void setWallDiagRectShape(const QPair<int, int> &topLeftIndex,
                              const QPair<int, int> &bottomRightIndex,
                              bool preview = false);
    QPixmap getSprite(const ArrayFloat_t &spriteData, const QString &installDir);
    void setCaseIcon(int x, int y, bool deleteMode = false);
    inline QIcon getCurrentSelectedIcon()const
    {
        return m_drawData[static_cast<uint32_t>(m_currentElementType)][m_currentSelection];
    }
private slots:
    void setElementSelected(LevelElement_e num, int currentSelect);
    void stdElementCaseSelectedChanged(const QModelIndex &current, const QModelIndex &previous);
    void wallSelection(const QModelIndex &index);
    void wallMouseReleaseSelection();
    void setWallDrawModeSelected(int wallDrawMode);
private:
    Ui::GridEditor *ui;
    LevelDataManager m_levelDataManager;
    TableModel *m_tableModel = nullptr;
    int m_currentSelection;
    LevelElement_e m_currentElementType;
    WallDrawMode_e m_wallDrawMode;
    std::array<QVector<QIcon>, static_cast<uint32_t>(LevelElement_e::TOTAL)> m_drawData;
    const int32_t CASE_SIZE_PX = 40, CASE_SPRITE_SIZE = (CASE_SIZE_PX * 4) / 5;
    bool m_elementSelected, m_displayPreview = false;
    EventFilter *m_eventFilter;
    QModelIndex m_wallFirstCaseSelection, m_wallSecondCaseSelection;
};

QString getStringFromLevelElementEnum(LevelElement_e num);
