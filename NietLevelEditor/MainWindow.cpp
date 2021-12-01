#include "MainWindow.hpp"
#include "ui_MainWindow.h"

//======================================================================
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), m_confNewLevelForm()
{
    ui->setupUi(this);
    linkButtons();
}

//======================================================================
void MainWindow::linkButtons()
{
    QPushButton *newButton = findChild<QPushButton*>("NewLevelButton"),
    *loadButton = findChild<QPushButton*>("LoadExistLevelButton"), *quitButton = findChild<QPushButton*>("QuitButton");
    assert(newButton);
    assert(loadButton);
    assert(quitButton);
    QObject::connect(newButton, SIGNAL(clicked()), this, SLOT(openEmptyEditorGrid()));
    QObject::connect(loadButton, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
}

//======================================================================
void MainWindow::openEmptyEditorGrid()
{
    m_confNewLevelForm.clearForm();
    m_confNewLevelForm.exec();
}

//======================================================================
MainWindow::~MainWindow()
{
    delete ui;
}

