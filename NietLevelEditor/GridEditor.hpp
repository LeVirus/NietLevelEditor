#pragma once

#include <QDialog>

namespace Ui {
class GridEditor;
}

class GridEditor : public QDialog
{
    Q_OBJECT

public:
    explicit GridEditor(QWidget *parent = nullptr);
    ~GridEditor();

private:
    Ui::GridEditor *ui;
};
