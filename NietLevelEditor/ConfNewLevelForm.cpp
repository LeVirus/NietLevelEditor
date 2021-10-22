#include "ConfNewLevelForm.hpp"
#include "ui_ConfNewLevelForm.h"
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <iostream>

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
    QString iniContent;
    dialog.setDefaultSuffix("ini");//doesn't work
    m_strINIFilePath = dialog.getOpenFileName(this, tr("Load File"), ".", tr("ini(*.ini);;All files (*)"));
    std::cerr << m_strINIFilePath.toStdString();
    QFile file(m_strINIFilePath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream >> iniContent;
        file.close();
    }
}

//======================================================================
void ConfNewLevelForm::onOkButtonClicked()
{
    if(m_strINIFilePath.isEmpty())
    {
        QMessageBox::warning(this, "Error", "INI file missing.");
        return;
    }
    m_gridEditorForm.exec();
    close();
}
