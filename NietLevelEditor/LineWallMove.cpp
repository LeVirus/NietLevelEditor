#include "LineWallMove.hpp"
#include <QPushButton>
#include <QLabel>

//======================================================================
LineWallMove::LineWallMove()
{
    confBaseWidgets();
}


//======================================================================
void LineWallMove::confBaseWidgets()
{
    m_label = new QLabel();
    m_pushButtonMoveUp = new QPushButton("MOVE UP");
    m_pushButtonMoveDown = new QPushButton("MOVE DOWN");
    m_pushButtonDelete = new QPushButton("DEL");
    addWidget(m_label);
    addWidget(m_pushButtonMoveUp);
    addWidget(m_pushButtonMoveDown);
    addWidget(m_pushButtonDelete);
}

//======================================================================
void LineWallMove::setProperties(Direction_e dir, int moveNumber)
{
    m_direction = dir;
    m_moveNumber = moveNumber;
    m_label->setText(getStringFromDir(dir) + "  " + std::to_string(m_moveNumber).c_str());
}

//======================================================================
QString getStringFromDir(Direction_e dir)
{
    switch(dir)
    {
    case Direction_e::EAST:
        return "EAST";
    case Direction_e::WEST:
        return "WEST";
    case Direction_e::NORTH:
        return "NORTH";
    case Direction_e::SOUTH:
        return "SOUTH";
    }
}
