#pragma once

#include <QIcon>
#include <QDialog>
#include <QModelIndex>
#include <QSet>
#include <limits>
#include "LevelDataManager.hpp"

class TableModel;
class QGroupBox;
class QItemSelection;
class EventFilter;
class TeleportForm;
class MoveableWallForm;
class SelectableLineLayout;
struct MoveWallData;

enum class Direction_e;

enum class LevelElement_e
{
    WALL,
    DOOR,
    TELEPORT,
    TARGET_TELEPORT,
    ENEMY,
    OBJECT,
    STATIC_CEILING,
    STATIC_GROUND,
    BARREL,
    EXIT,
    TRIGGER,
    DELETE,
    PLAYER_DEPARTURE,
    SELECTION,
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
    void loadTeleportsPictures(const QString &installDir);
    void loadEnemiesPictures(const QString &installDir);
    void loadObjectsPictures(const QString &installDir);
    void loadStaticCeilingElementPictures(const QString &installDir);
    void loadStaticGroundElementPictures(const QString &installDir);
    void loadBarrelsPictures(const QString &installDir);
    void loadExitsPictures(const QString &installDir);
    bool setWallShape(bool preview = false);
    void memStdWallMove();
    void setWallLineRectShape(const QPair<int, int> &topLeftIndex,
                              const QPair<int, int> &bottomRightIndex,
                              bool preview = false);
    void setWallDiagLineShape(const QPair<int, int> &topLeftIndex,
                              const QPair<int, int> &bottomRightIndex,
                              bool preview = false);
    bool setWallDiagRectShape(const QPair<int, int> &topLeftIndex,
                              const QPair<int, int> &bottomRightIndex,
                              bool preview = false);
    void setCaseIcon(int x, int y, bool deleteMode = false);
    void memWallMove(const QModelIndex &index);
    void setPlayerDeparture(int x, int y);
    QIcon getCurrentSelectedIcon()const;
    void updateGridView();
    void setLineSelectableEnabled(bool enable);
    void treatWallDrawing();
    void treatElementsDrawing();
    void confNewTriggerData(const QModelIndex &caseIndex);
private slots:
    void setElementSelected(LevelElement_e num, int currentSelect);
    void stdElementCaseSelectedChanged(const QModelIndex &current, const QModelIndex &previous);
    void wallSelection(const QModelIndex &index);
    void mouseReleaseSelection();
    void treatSelection(const QModelIndex &caseIndex);
    void setWallDrawModeSelected(int wallDrawMode);
    void setWallMoveableMode(int moveableMode);
    void loadTriggerDisplay(const QString &installDir);
private:
    Ui::GridEditor *ui;
    QPair<int, int> m_lastPositionAdded;
    LevelDataManager m_levelDataManager;
    TableModel *m_tableModel = nullptr;
    int m_currentSelection;
    LevelElement_e m_currentElementType;
    WallDrawMode_e m_wallDrawMode;
    bool m_wallMoveableMode;
    std::array<QVector<QIcon>, static_cast<uint32_t>(LevelElement_e::TOTAL)> m_drawData;
    bool m_elementSelected, m_displayPreview = false;
    EventFilter *m_eventFilter;
    QModelIndex m_wallFirstCaseSelection, m_wallSecondCaseSelection;
    TeleportForm *m_teleportForm = nullptr;
    MoveableWallForm *m_moveableWallForm = nullptr;
    SelectableLineLayout *m_memWallSelectLayout = nullptr;
    std::map<LevelElement_e, QVector<QString>> m_mapElementID;
    std::unique_ptr<MoveWallData> m_memcurrentMoveWallData;
    QSet<QPair<int, int>> m_memCurrentLinkTriggerWall;
};

Direction_e getDirEnumFromQString(const QString &str);
const int32_t CASE_SIZE_PX = 40, CASE_SPRITE_SIZE = (CASE_SIZE_PX * 4) / 5;
QPixmap getSprite(const ArrayFloat_t &spriteData, const LevelDataManager &levelDataManager,
                  const QString &installDir);
QString getStringFromLevelElementEnum(LevelElement_e num);

