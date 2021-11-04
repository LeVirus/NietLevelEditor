#pragma once

#include <QDialog>
#include <QPair>

namespace Ui {
class TeleportForm;
}

class TeleportForm : public QDialog
{
    Q_OBJECT
public:
    explicit TeleportForm(QWidget *parent = nullptr);
    void conf(int levelWidth, int levelHeight);
    inline void init()
    {
        m_validateForm = false;
    }
    inline bool valid()const
    {
        return m_validateForm;
    }
    QPair<int, int> getSpinValue()const;
    ~TeleportForm();
private slots:
    void onOkButtonClicked();
private:
    void connectSlots();
private:
    Ui::TeleportForm *ui;
    bool m_validateForm;
};
