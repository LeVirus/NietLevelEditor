#pragma once

#include <QIcon>
#include <QDialog>
#include <QModelIndex>
#include <QSet>
#include <limits>
#include "LevelDataManager.hpp"
#include <MoveableWallForm.hpp>

class TableModel;
class QGroupBox;
class QItemSelection;
class EventFilter;
class MoveableWallForm;
class SelectableLineLayout;
class BackgroundForm;
struct MoveWallData;
class QComboBox;

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
    TRIGGER,
    GROUND_TRIGGER,
    EXIT,
    PLAYER_DEPARTURE,
    SELECTION,
    DELETE,
    TOTAL
};

enum class WallDrawShape_e
{
    LINE_AND_RECT,
    DIAGONAL_LINE,
    DIAGONAL_RECT
};

struct DisplayData
{
    QString m_elementSectionName, m_spriteName;
    QIcon m_icon;
};

inline const float EPSILON_FLOAT = std::numeric_limits<float>::epsilon();

using IconArray_t = std::array<QVector<DisplayData>, static_cast<uint32_t>(LevelElement_e::TOTAL)>;

namespace Ui {
class GridEditor;
}

class GridEditor : public QDialog
{
    Q_OBJECT
public:
    explicit GridEditor(QWidget *parent = nullptr);
    bool loadMainInstallDirData(const QString &installDir);
    bool loadExistingLevelINI(const QString &levelFilePath);
    void initGrid(const QString &installDir, int levelWidth, int levelHeight);
    std::optional<QPair<int, int>> getLoadedLevelSize()const;
    bool loadExistingLevelGrid();
    void unselectAllRadioButtons();
    ~GridEditor();
private:
    bool loadStandardExistingLevelGrid(LevelElement_e elementType);
    bool loadTeleportExistingLevelGrid();
    bool loadBackgroundGeneralExistingLevelGrid();
    bool loadWallExistingLevelGrid();
    bool loadRemovedWallExistingLevelGrid(const std::pair<const QString, WallDataINI> &currentTreat);
    void connectSlots();
    void setStdTableSize();
    void initSelectableWidgets();
    void initButtons();
    void initMusicDir(const QString &installDir, bool widgetInit);
    void loadIconPictures(const QString &installDir);
    void loadSpritesForBackgroundForm();
    void loadWallsPictures(const QString &installDir);
    void loadDoorsPictures(const QString &installDir);
    void loadTeleportsPictures(const QString &installDir);
    void loadEnemiesPictures(const QString &installDir);
    void loadObjectsPictures(const QString &installDir);
    void loadStaticCeilingElementPictures(const QString &installDir);
    void loadStaticGroundElementPictures(const QString &installDir);
    void loadBarrelsPictures(const QString &installDir);
    void loadExitsPictures(const QString &installDir);
    bool setWallShape(bool preview = false, bool loadFromIni = false);
    void setWallDiagCaseConf();
    void setWallDiagRectCaseConf();
    void setDeletionZone(bool preview = false);
    void memStdWallMove();
    uint32_t setWallLineRectShape(const QPair<int, int> &topLeftIndex, const QPair<int, int> &bottomRightIndex,
                                  int shapeNum, bool preview = false, bool deleteMode = false);
    uint32_t setWallDiagLineShape(const QPair<int, int> &topLeftIndex, const QPair<int, int> &bottomRightIndex,
                                  int shapeNum, bool preview = false, bool deleteMode = false);
    bool setWallDiagRectShape(const QPair<int, int> &topLeftIndex, const QPair<int, int> &bottomRightIndex,
                              int shapeNum, uint32_t &wallNumber, bool preview = false, bool deleteMode = false);
    void setCaseIcon(int x, int y, int wallShapeNum, bool deleteMode = false, bool dontMemRemovedWall = false);
    void memWallMove(const QModelIndex &index);
    void setColorCaseData(int x, int y, LevelElement_e type);
    QIcon getCurrentSelectedIcon()const;
    void updateGridView();
    void setLineSelectableEnabled(bool enable);
    void treatWallDrawing();
    void treatElementsDrawing();
    void confNewTriggerData(const QModelIndex &caseIndex);
    void setTargetTeleport(const QModelIndex &caseIndex);
    void setPlayerDeparture(const QModelIndex &caseIndex);
    void setTeleporter(const QModelIndex &caseIndex);
private slots:
    void execConfCeilingBackground();
    void execConfGroundBackground();
    void generateLevel();
    void setElementSelected(LevelElement_e num, int currentSelect);
    void stdElementCaseSelectedChanged(const QModelIndex &current, const QModelIndex &previous);
    void wallSelection(const QModelIndex &index);
    void mouseReleaseSelection();
    void treatSelection(const QModelIndex &caseIndex);
    void setWallDrawModeSelected(int wallDrawMode);
    void setWallMoveableMode(int moveableMode);
    void loadTriggerDisplay(const QString &installDir);
    void memPlayerDirection(int direction);
private:
    Ui::GridEditor *ui;
    QPair<int, int> m_lastPositionAdded;
    LevelDataManager m_levelDataManager;
    TableModel *m_tableModel = nullptr;
    int m_currentSelection;
    LevelElement_e m_currentElementType;
    WallDrawShape_e m_wallDrawMode;
    bool m_wallMoveableMode;
    IconArray_t m_drawData;
    bool m_elementSelected, m_displayPreview = false;
    EventFilter *m_eventFilter;
    QModelIndex m_firstCaseSelection, m_secondCaseSelection;
    BackgroundForm *m_backgroundForm = nullptr;
    MoveableWallForm *m_moveableWallForm = nullptr;
    SelectableLineLayout *m_memWallSelectLayout = nullptr;
    std::map<LevelElement_e, QVector<QString>> m_mapElementID;
    std::unique_ptr<MoveWallData> m_memcurrentMoveWallData;
    QSet<QPair<int, int>> m_memCurrentLinkTriggerWall;
    QPushButton *m_generateButton;
    QComboBox *m_musicWidget;
    Direction_e m_memPlayerDirection = Direction_e::NORTH;
    bool m_widgetInit = false, m_loadingExistingLevelMode = false, m_loadingDistantTriggerMode = false;
};

Direction_e getDirEnumFromQString(const QString &str);
const int32_t CASE_SIZE_PX = 40, CASE_SPRITE_SIZE = (CASE_SIZE_PX * 4) / 5;
QPixmap getSprite(const ArrayFloat_t &spriteData, const LevelDataManager &levelDataManager, const QString &installDir);
QString getStringFromLevelElementEnum(LevelElement_e num);

