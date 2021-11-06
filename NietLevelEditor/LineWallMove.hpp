#pragma once

class QLabel;
class QPushButton;

#include <QHBoxLayout>
#include "MoveableWallForm.hpp"

class LineWallMove : public QHBoxLayout
{
    Q_OBJECT
public:
    LineWallMove(int index);
    ~LineWallMove();
    void setProperties(Direction_e dir, int moveNumber);
    inline void setIndex(int index)
    {
        m_index = index;
    }
private:
    void confBaseWidgets();
    void connectSlots();
Q_SIGNALS:
    void sigMoveUp(int index);
    void sigDown(int index);
    void sigRemove(int index);
private slots:
    void moveUp();
    void moveDown();
    void remove();
private:
    QLabel *m_label;
    QPushButton *m_pushButtonDelete, *m_pushButtonMoveUp, *m_pushButtonMoveDown;
    Direction_e m_direction;
    int m_moveNumber;
    int m_index;
};

QString getStringFromDir(Direction_e dir);
