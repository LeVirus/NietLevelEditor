#pragma once

#include <QMainWindow>
#include "ConfNewLevelForm.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void linkButtons();
private slots:
    void openEmptyEditorGrid();
private:
    Ui::MainWindow *ui;
    ConfNewLevelForm m_confNewLevelForm;
};
