#pragma once

class QLabel;
class QPushButton;

#include <QHBoxLayout>
#include "MoveableWallForm.hpp"

class LineWallMove : public QHBoxLayout
{
    Q_OBJECT
public:
    LineWallMove();
    void setProperties(Direction_e dir, int moveNumber);
private:
    void confBaseWidgets();
private:
    QLabel *m_label;
    QPushButton *m_pushButtonDelete, *m_pushButtonMoveUp, *m_pushButtonMoveDown;
    Direction_e m_direction;
    int m_moveNumber;
};

QString getStringFromDir(Direction_e dir);
