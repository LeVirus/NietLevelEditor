#include "LineWallMove.hpp"
#include <QPushButton>
#include <QLabel>

//======================================================================
LineWallMove::LineWallMove(int index): m_index(index)
{
    confBaseWidgets();
    connectSlots();
}

//======================================================================
LineWallMove::~LineWallMove()
{
    delete m_pushButtonMoveUp;
    delete m_pushButtonDelete;
    delete m_pushButtonMoveDown;
    delete m_label;
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
void LineWallMove::connectSlots()
{
    QObject::connect(m_pushButtonMoveUp, &QPushButton::clicked, this, &LineWallMove::moveUp);
    QObject::connect(m_pushButtonMoveDown, &QPushButton::clicked, this, &LineWallMove::moveDown);
    QObject::connect(m_pushButtonDelete, &QPushButton::clicked, this, &LineWallMove::remove);
}

//======================================================================
void LineWallMove::moveUp()
{
    emit sigMoveUp(m_index);
}

//======================================================================
void LineWallMove::moveDown()
{
    emit sigMoveDown(m_index);
}

//======================================================================
void LineWallMove::remove()
{
    emit sigRemove(m_index);
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
    return "";
}
