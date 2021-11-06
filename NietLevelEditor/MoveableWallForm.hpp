#pragma once

#include <QDialog>

class QVBoxLayout;
class LevelDataManager;

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
    void loadTriggerDisplay(const LevelDataManager &levelDataManager, const QString &installDir);
    inline bool confirmed()const
    {
        return m_confirmed;
    }
    inline void init()
    {
        m_confirmed = false;
    }
    ~MoveableWallForm();
private:
    void initUI();
private slots:
    void treatComboBoxTriggerBehaviour(int index);
    void treatComboBoxTrigger(int index);
    void setConfirmed();
    void addMove();
private:
    Ui::MoveableWallForm *ui;
    QVBoxLayout *m_scrollLayout;
    bool m_confirmed;
};
