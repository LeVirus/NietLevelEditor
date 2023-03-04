#include "ConfNewLevelForm.hpp"
#include "ui_ConfNewLevelForm.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>

//======================================================================
ConfNewLevelForm::ConfNewLevelForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfNewLevelForm)
{
    ui->setupUi(this);
    linkButtons();
    setWindowFlags(Qt::WindowSystemMenuHint);
}

//======================================================================
void ConfNewLevelForm::clearForm()
{
    ui->browseExistingLevel->setEnabled(false);
    ui->widthLevelspinBox->setEnabled(false);
    ui->heightLevelspinBox->setEnabled(false);
    ui->radioExistingLevel->setAutoExclusive(false);
    ui->radioExistingLevel->setChecked(false);
    ui->radioExistingLevel->setAutoExclusive(true);
    ui->radioNewLevel->setAutoExclusive(false);
    ui->radioNewLevel->setChecked(false);
    ui->radioNewLevel->setAutoExclusive(true);
    m_mode = FormMode_e::UNSELECTED;
}

//======================================================================
ConfNewLevelForm::~ConfNewLevelForm()
{
    delete ui;
}

//======================================================================
void ConfNewLevelForm::linkButtons()
{
    QObject::connect(ui->browseButton, SIGNAL(clicked()), this, SLOT(onBrowseIniFileClicked()));
    QObject::connect(ui->browseExistingLevel, SIGNAL(clicked()), this, SLOT(openExistingLevel()));
    QObject::connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));
    QObject::connect(ui->radioNewLevel, SIGNAL(toggled(bool)), this, SLOT(toggleNewLevel(bool)));
    QObject::connect(ui->radioExistingLevel, SIGNAL(toggled(bool)), this, SLOT(toggleLoadLevel(bool)));
}

//======================================================================
void ConfNewLevelForm::openExistingLevel()
{
    QFileDialog dialog;
    QString selfilter = "CLVL (*.clvl)";
    m_existingLevelFile = dialog.getOpenFileName(nullptr, "Level file selection",
                                                 m_installDirectory + "/Ressources/", selfilter, &selfilter);
    QFile file(m_existingLevelFile);
    if(!file.exists())
    {
        m_existingLevelFile.clear();
    }
    else
    {
        onOkButtonClicked();
    }
}

//======================================================================
void ConfNewLevelForm::toggleNewLevel(bool selected)
{
    if(selected)
    {
        ui->widthLevelspinBox->setEnabled(selected);
        ui->heightLevelspinBox->setEnabled(selected);
        ui->browseExistingLevel->setEnabled(!selected);
        m_mode = FormMode_e::NEW_LEVEL;
    }
}

//======================================================================
void ConfNewLevelForm::toggleLoadLevel(bool selected)
{
    if(selected)
    {
        ui->widthLevelspinBox->setEnabled(!selected);
        ui->heightLevelspinBox->setEnabled(!selected);
        ui->browseExistingLevel->setEnabled(selected);
        m_mode = FormMode_e::EXISTING_LEVEL;
    }
}

//======================================================================
void ConfNewLevelForm::onBrowseIniFileClicked()
{
    QFileDialog dialog;
    QStringList files;
    do
    {
        m_installDirectory = dialog.getExistingDirectory();
        QDir dir(m_installDirectory);
        if(dir.exists())
        {
            if(dir.cd("Ressources") && dir.exists())
            {
                files = dir.entryList(QStringList() << "*.ini", QDir::Files);
                if(files.contains("pictureData.ini") && files.contains("standardData.ini"))
                {
                    //OK
                    break;
                }
            }
        }
        QMessageBox::information(nullptr, "Install directory selection", "Please select game install directory.");
    }while(true);
}

//======================================================================
void ConfNewLevelForm::onOkButtonClicked()
{
    QSpinBox *widthLevelspinBox = findChild<QSpinBox*>("widthLevelspinBox"), *heightLevelspinBox = findChild<QSpinBox*>("heightLevelspinBox");
    assert(widthLevelspinBox);
    assert(heightLevelspinBox);
    if(m_installDirectory.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Please select install directory.");
        return;
    }

    if(m_mode == FormMode_e::UNSELECTED)
    {
        QMessageBox::warning(this, "Error", "Please select editing mode (New or existing level).");
        return;
    }
    if(!m_gridEditorForm.loadMainInstallDirData(m_installDirectory))
    {
        QMessageBox::warning(this, "Error", "Error while initializing grid.");
        return;
    }
    if(m_mode == FormMode_e::NEW_LEVEL)
    {
        m_gridEditorForm.initGrid(m_installDirectory, widthLevelspinBox->value(), heightLevelspinBox->value());
    }
    else if(m_mode == FormMode_e::EXISTING_LEVEL)
    {
        if(m_existingLevelFile.isEmpty())
        {
            QMessageBox::warning(this, "Error", "Please select a correct level file.");
            return;
        }
        if(!m_gridEditorForm.loadExistingLevelINI(m_existingLevelFile))
        {
            QMessageBox::warning(this, "Error", "Error while loading level ini file.");
            return;
        }
        std::optional<QPair<int, int>> levelSize = m_gridEditorForm.getLoadedLevelSize();
        assert(levelSize);
        m_gridEditorForm.initGrid(m_installDirectory, levelSize->first, levelSize->second);
        if(!m_gridEditorForm.loadExistingLevelGrid())
        {
            QMessageBox::warning(this, "Error", "Error while loading level on grid.");
            return;
        }
    }
    m_gridEditorForm.unselectAllRadioButtons();
    m_existingLevelFile.clear();
    m_gridEditorForm.exec();
    close();
}
