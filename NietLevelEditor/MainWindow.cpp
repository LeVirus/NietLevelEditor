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
    QObject::connect(ui->NewLevelButton, SIGNAL(clicked()), this, SLOT(openEmptyEditorGrid()));
    QObject::connect(ui->QuitButton, SIGNAL(clicked()), this, SLOT(close()));
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

