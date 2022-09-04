#include "GlobalLevelConfForm.hpp"
#include "ui_GlobalLevelConfForm.h"
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
void GlobalLevelConfForm::init(const QComboBox *musicWidget, std::optional<GlobalLevelData> data)
{
    m_valid = false;
    ui->MusicWidget->clear();
    for(int32_t i = 0; i < musicWidget->count(); ++i)
    {
        ui->MusicWidget->addItem(musicWidget->itemText(i));
    }
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
    m_data.m_prologue = ui->ProloguePlainTextEdit->toPlainText();
    m_data.m_epilogue = ui->EpiloguePlainTextEdit->toPlainText();
    m_data.m_epilogueMusic = ui->MusicWidget->currentText();
    close();
}

//======================================================================
GlobalLevelConfForm::~GlobalLevelConfForm()
{
    delete ui;
}
