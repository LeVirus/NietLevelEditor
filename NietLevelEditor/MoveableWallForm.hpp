#pragma once

#include <QDialog>
#include <optional>

class QVBoxLayout;
class LevelDataManager;
class LineWallMove;
struct CaseData;
struct DisplayData;

namespace Ui {
class MoveableWallForm;
}

enum class Direction_e
{
    NORTH,
    WEST,
    SOUTH,
    EAST
};

enum class TriggerBehaviourType_e
{
    ONCE,
    MULTI_REVERSE_DIRECTION,
    MULTI_SAME_DIRECTION,
    AUTO
};

enum class TriggerType_e
{
    WALL,
    DISTANT_SWITCH,
    GROUND
};

class MoveableWallForm : public QDialog
{
    Q_OBJECT
public:
    explicit MoveableWallForm(QWidget *parent = nullptr);
    inline bool confirmed()const
    {
        return m_confirmed;
    }
    inline int getVelocity()
    {
        return m_velocity;
    }
    inline TriggerType_e getTriggerType()
    {
        return m_triggerType;
    }
    inline TriggerBehaviourType_e getTriggerBehaviour()
    {
        return m_triggerBehaviour;
    }
    int getCurrentTriggerAppearence();
    void setData(const CaseData &data);
    void setTriggerIcons(const QVector<DisplayData> &vectIcon);
    void init();
    const QObjectList &getWallMove()const;
    QString getCurrentTriggerINISection()const;
    ~MoveableWallForm();
private:
    void initUI();
    void updateMoveLine();
    void clear();
private slots:
    void removeItemAt(int index);
    void moveItemUp(int index);
    void moveItemDown(int index);
    void treatComboBoxTriggerBehaviour(int index);
    void treatComboBoxTrigger(int index);
    void modifyVelocity(int value);
    void setConfirmed();
    void addMove();
private:
    Ui::MoveableWallForm *ui;
    QVBoxLayout *m_scrollLayout;
    bool m_confirmed;
    TriggerType_e m_triggerType;
    TriggerBehaviourType_e m_triggerBehaviour;
    int m_velocity = 1;
    std::optional<QPair<Direction_e, int>> m_memMove;
};

void swapContent(LineWallMove *lineWallA, LineWallMove *lineWallB);
