#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include <QMessageBox>

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
    QMessageBox::information(nullptr, "Install dir", "Please select game install directory.");
    m_confNewLevelForm.onBrowseIniFileClicked();
    m_confNewLevelForm.exec();
}

//======================================================================
MainWindow::~MainWindow()
{
    delete ui;
}

