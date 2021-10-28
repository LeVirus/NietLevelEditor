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
}

//======================================================================
ConfNewLevelForm::~ConfNewLevelForm()
{
    delete ui;
}

//======================================================================
void ConfNewLevelForm::linkButtons()
{
    QPushButton *okButton = findChild<QPushButton*>("okButton"), *cancelButton = findChild<QPushButton*>("cancelButton"),
            *browseButton = findChild<QPushButton*>("browseButton");
    assert(okButton);
    assert(cancelButton);
    assert(browseButton);
    QObject::connect(browseButton, SIGNAL(clicked()), this, SLOT(onBrowseIniFileClicked()));
    QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(okButton, SIGNAL(clicked()), this, SLOT(onOkButtonClicked()));
}

//======================================================================
void ConfNewLevelForm::onBrowseIniFileClicked()
{
    QFileDialog dialog;
    m_installDirectory = dialog.getExistingDirectory();
}

//======================================================================
void ConfNewLevelForm::onOkButtonClicked()
{
    QSpinBox *widthLevelspinBox = findChild<QSpinBox*>("widthLevelspinBox"), *heightLevelspinBox = findChild<QSpinBox*>("heightLevelspinBox");
    assert(widthLevelspinBox);
    assert(heightLevelspinBox);
    if(m_installDirectory.isEmpty() || !m_gridEditorForm.initGrid(m_installDirectory, widthLevelspinBox->value(), heightLevelspinBox->value()))
    {
        QMessageBox::warning(this, "Error", "Please select install directory.");
        return;
    }
    m_gridEditorForm.exec();
    close();
}
