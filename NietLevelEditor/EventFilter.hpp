#pragma once

#include <QObject>
#include <QMouseEvent>

class EventFilter : public QObject
{
    Q_OBJECT
public:
    explicit EventFilter(QObject *parent = nullptr);
public slots:
    bool keyPressEvent(QKeyEvent *e);
signals:
    void mouseReleased();
    void keyPressed();
protected:
    bool eventFilter(QObject *obj, QEvent *event)override;
};

