#pragma once

#include <QDialog>
#include "MoveableWallForm.hpp"

namespace Ui {
class CheckpointForm;
}

class CheckpointForm : public QDialog
{
    Q_OBJECT

public:
    explicit CheckpointForm(QWidget *parent = nullptr);
    ~CheckpointForm();
    void linkWidgets();
    inline void reinit()
    {
        m_valid = false;
    }
    inline bool isValid()const
    {
        return m_valid;
    }
    void setMenuEntries(const QPair<uint32_t, Direction_e> &data);
    void setSpinBoxSize(uint32_t size);
    QPair<uint32_t, Direction_e> getCheckpointData() const;
private slots:
    void confirm();
    void cancel();
private:
    bool m_valid;
    Ui::CheckpointForm *ui;
};
