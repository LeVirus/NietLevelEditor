#include "GridEditor.hpp"
#include "ui_GridEditor.h"

GridEditor::GridEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GridEditor)
{
    ui->setupUi(this);
}

GridEditor::~GridEditor()
{
    delete ui;
}
