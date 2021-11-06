#pragma once

class QPushButton;

#include <QHBoxLayout>
#include <QLabel>
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
    inline int getIndex()const
    {
        return m_index;
    }
    inline void setQString(const QString &str)
    {
        m_label->setText(str);
    }
    inline QString getQString()const
    {
        return m_label->text();
    }
private:
    void confBaseWidgets();
    void connectSlots();
Q_SIGNALS:
    void sigMoveUp(int index);
    void sigMoveDown(int index);
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
