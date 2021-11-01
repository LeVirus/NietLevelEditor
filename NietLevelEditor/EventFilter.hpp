#pragma once

#include <QObject>
#include <QMouseEvent>

class EventFilter : public QObject
{
    Q_OBJECT
public:
    explicit EventFilter(QObject *parent = nullptr);
signals:
    void mouseReleased();
protected:
    bool eventFilter(QObject *obj, QEvent *event)override;
};

