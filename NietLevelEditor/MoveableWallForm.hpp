﻿#pragma once

#include <QDialog>

class QVBoxLayout;
class LevelDataManager;
class LineWallMove;

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
    inline bool isDistantTriggerMode()
    {
        return m_distantTriggerMode;
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
    void setTriggerIcons(const QVector<QIcon> &vectIcon);
    void init();
    const QObjectList &getWallMove()const;
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
    bool m_distantTriggerMode;
    TriggerType_e m_triggerType;
    TriggerBehaviourType_e m_triggerBehaviour;
    int m_velocity = 1;
};

void swapContent(LineWallMove *lineWallA, LineWallMove *lineWallB);
