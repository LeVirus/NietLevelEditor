#include "GlobalLevelConfForm.hpp"
#include "ui_GlobalLevelConfForm.h"
#include "LevelDataManager.hpp"
#include <iostream>

//======================================================================
GlobalLevelConfForm::GlobalLevelConfForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalLevelConfForm)
{
    ui->setupUi(this);
    linkButtons();
}

//======================================================================
void GlobalLevelConfForm::init(std::optional<GlobalLevelData> data)
{
    m_valid = false;
    if(data)
    {
        m_data = *data;
        ui->LevelNumSpinBox->setValue(m_data.m_levelNum);
        ui->ProloguePlainTextEdit->setPlainText(data->m_prologue);
        ui->EpiloguePlainTextEdit->setPlainText(data->m_epilogue);
    }
}

//======================================================================
void GlobalLevelConfForm::linkButtons()
{
    QObject::connect(ui->OkButton, &QPushButton::clicked, this, &GlobalLevelConfForm::validateForm);
    QObject::connect(ui->CancelButton, &QPushButton::clicked, this, &GlobalLevelConfForm::close);
}

//======================================================================
void GlobalLevelConfForm::validateForm()
{
    m_valid = true;
    m_data.m_levelNum = ui->LevelNumSpinBox->value();
    m_data.m_prologue = QString(treatStrEndLineSave(ui->ProloguePlainTextEdit->toPlainText().toStdString()).c_str());
    m_data.m_epilogue = QString(treatStrEndLineSave(ui->EpiloguePlainTextEdit->toPlainText().toStdString()).c_str());
    close();
}

//======================================================================
GlobalLevelConfForm::~GlobalLevelConfForm()
{
    delete ui;
}
