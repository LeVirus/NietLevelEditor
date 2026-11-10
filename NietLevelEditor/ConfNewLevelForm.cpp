#include "ConfNewLevelForm.hpp"
#include "qprogressbar.h"
#include "ui_ConfNewLevelForm.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSpinBox>
#include <QTimer>

//======================================================================
ConfNewLevelForm::ConfNewLevelForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfNewLevelForm)
{
    ui->setupUi(this);
    linkButtons();
    setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
}

//======================================================================
void ConfNewLevelForm::clearForm()
{
    m_mode = FormMode_e::EXISTING_LEVEL;
}

//======================================================================
ConfNewLevelForm::~ConfNewLevelForm()
{
    if(m_gridEditorForm)
    {
        delete m_gridEditorForm;
    }
    delete ui;
}

//======================================================================
void ConfNewLevelForm::linkButtons()
{
    QObject::connect(ui->browseExistingLevel, SIGNAL(clicked()), this, SLOT(openExistingLevel()));
    QObject::connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui->okButton, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));
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
        m_mode = FormMode_e::EXISTING_LEVEL;
        onOkButtonClicked();
    }
}

//======================================================================
void ConfNewLevelForm::onBrowseIniFileClicked()
{
    checkInstallDirectory(QFileDialog().getExistingDirectory());
}

//======================================================================
bool ConfNewLevelForm::checkInstallDirectory(const QString &directory)
{
    uint32_t i = 0;
    QFileDialog dialog;
    QStringList files;
    m_directoryOK = false;
    if(m_installDirectory.isEmpty())
    {
        m_installDirectory = directory;
        ++i;
    }
    for(;i < 2; ++i)
    {
        //if m_installDirectory not empty check initial value first
        QDir dir(m_installDirectory);
        if(dir.exists())
        {
            if(dir.cd("Ressources") && dir.exists())
            {
                files = dir.entryList(QStringList() << "*.ini", QDir::Files);
                if(files.contains("pictureData.ini") && files.contains("standardData.ini"))
                {
                    m_directoryOK = true;
                    m_mode = FormMode_e::NEW_LEVEL;
                    //OK
                    return true;
                }
            }
        }
        m_installDirectory = directory;
    }

    return false;
}

//======================================================================
void ConfNewLevelForm::showProgressBar()
{
    if(m_progressDialog)
    {
        return;
    }
    m_progressDialog = new QDialog(nullptr);
    m_progressDialog->setWindowTitle("Loading");
    m_progressDialog->setWindowModality(Qt::ApplicationModal);
    m_progressDialog->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowStaysOnTopHint);
    m_progressDialog->setFixedSize(280, 80);

    QVBoxLayout *layout = new QVBoxLayout(m_progressDialog);
    layout->addWidget(new QLabel("Loading...", m_progressDialog));
    if(!m_bar)
    {
        m_bar = new QProgressBar(m_progressDialog);
        m_bar->setRange(0, 100);
        m_bar->setTextVisible(false);
    }
    layout->addWidget(m_bar);
    layout->setContentsMargins(20, 15, 20, 15);

    m_progressDialog->show();
    m_progressDialog->raise();
    m_progressDialog->activateWindow();

    // Force une petite boucle d’événements (comme QMessageBox)
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);  // 100 ms pour laisser le temps de peindre
    loop.exec();
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
    if(m_gridEditorForm)
    {
        delete m_gridEditorForm;
    }
    m_gridEditorForm = new GridEditor();
    if(!m_gridEditorForm->loadMainInstallDirData(m_installDirectory))
    {
        QMessageBox::warning(this, "Error", "Error while initializing grid. Please select install directory.");
        onBrowseIniFileClicked();
        if(!m_directoryOK)
        {
            QMessageBox::warning(this, "Error", "Bad install directory.");
            return;
        }
    }
    showProgressBar();
    if(m_mode == FormMode_e::NEW_LEVEL)
    {
        m_gridEditorForm->initGrid(m_installDirectory, widthLevelspinBox->value(), heightLevelspinBox->value(), m_bar);
    }
    else if(m_mode == FormMode_e::EXISTING_LEVEL)
    {
        if(m_existingLevelFile.isEmpty())
        {
            QMessageBox::warning(this, "Error", "Please select a correct level file.");
            return;
        }
        if(!m_gridEditorForm->loadExistingLevelINI(m_existingLevelFile))
        {
            QMessageBox::warning(this, "Error", "Error while loading level ini file.");
            return;
        }
        std::optional<QPair<int, int>> levelSize = m_gridEditorForm->getLoadedLevelSize();
        assert(levelSize);
        m_gridEditorForm->initGrid(m_installDirectory, levelSize->first, levelSize->second, m_bar);
        if(!m_gridEditorForm->loadExistingLevelGrid())
        {
            QMessageBox::warning(this, "Error", "Error while loading level on grid.");
            return;
        }
    }
    m_gridEditorForm->unselectAllRadioButtons();
    m_existingLevelFile.clear();
    m_bar->setValue(100);
    hideProgressBar();
    m_gridEditorForm->exec();
    // close();
}

//======================================================================
void ConfNewLevelForm::hideProgressBar()
{
    if (m_progressDialog)
    {
        m_progressDialog->close();
        delete m_progressDialog;
        m_progressDialog = nullptr;
        m_bar = nullptr;
    }
}
