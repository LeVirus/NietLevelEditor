#include "EventFilter.hpp"
#include <QEvent>
#include <iostream>

//======================================================================
EventFilter::EventFilter(QObject *parent) : QObject(parent)
{

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
