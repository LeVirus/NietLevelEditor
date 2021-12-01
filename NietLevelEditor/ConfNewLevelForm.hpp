#pragma once

#include <QDialog>
#include "GridEditor.hpp"

namespace Ui
{
class ConfNewLevelForm;
}

enum class FormMode_e
{
    UNSELECTED,
    NEW_LEVEL,
    EXISTING_LEVEL
};

class ConfNewLevelForm : public QDialog
{
    Q_OBJECT
public:
    explicit ConfNewLevelForm(QWidget *parent = nullptr);
    void clearForm();
    ~ConfNewLevelForm();
private:
    void linkButtons();
private slots:
    void onBrowseIniFileClicked();
    void onOkButtonClicked();
    void openExistingLevel();
    void toggleNewLevel(bool selected);
    void toggleLoadLevel(bool selected);
private:
    Ui::ConfNewLevelForm *ui;
    GridEditor m_gridEditorForm;
    QString m_installDirectory, m_existingLevelFile;
    FormMode_e m_mode;
};

