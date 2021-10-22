#pragma once

#include <QDialog>
#include "GridEditor.hpp"

namespace Ui
{
class ConfNewLevelForm;
}

class ConfNewLevelForm : public QDialog
{
    Q_OBJECT

public:
    explicit ConfNewLevelForm(QWidget *parent = nullptr);
    ~ConfNewLevelForm();
private:
    void linkButtons();
private slots:
    void onBrowseIniFileClicked();
    void onOkButtonClicked();
private:
    Ui::ConfNewLevelForm *ui;
    GridEditor m_gridEditorForm;
    QString m_installDirectory;
};

