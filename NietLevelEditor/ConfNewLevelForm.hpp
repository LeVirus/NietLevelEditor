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
    inline bool isDirectorySelected()const
    {
        return m_directoryOK;
    }
    ~ConfNewLevelForm();
private:
    void linkButtons();
public slots:
    void onBrowseIniFileClicked();
private slots:
    void onOkButtonClicked();
    void openExistingLevel();
    void toggleNewLevel(bool selected);
    void toggleLoadLevel(bool selected);
private:
    Ui::ConfNewLevelForm *ui;
    GridEditor *m_gridEditorForm = nullptr;
    QString m_installDirectory, m_existingLevelFile;
    bool m_directoryOK = false;
    FormMode_e m_mode;
};

