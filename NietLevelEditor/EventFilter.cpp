#include "EventFilter.hpp"
#include <QEvent>
#include <iostream>

//======================================================================
EventFilter::EventFilter(QObject *parent) : QObject(parent)
{

}

bool EventFilter::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Escape)
    {
        return true;
    }
}

//======================================================================
bool EventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
        emit mouseReleased();
        return true;
    }
    return false;
}
