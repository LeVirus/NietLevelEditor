#include "LogForm.hpp"
#include "ui_LogForm.h"
#include <iostream>

//=============================================================
LogForm::LogForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogForm)
{
    ui->setupUi(this);
    QObject::connect(ui->OkButton, &QPushButton::clicked, this, &LogForm::valid);
    QObject::connect(ui->CancelButton, &QPushButton::clicked, this, &LogForm::cancel);
}

//=============================================================
void LogForm::reinit()
{
    m_valid = false;
    ui->plainTextEdit->clear();
    m_messageText.clear();
}

//=============================================================
LogForm::~LogForm()
{
    delete ui;
}

//=============================================================
void LogForm::valid()
{
    m_valid = true;
    m_messageText = ui->plainTextEdit->toPlainText().toUpper();
    close();
}

//=============================================================
void LogForm::cancel()
{
    close();
}
