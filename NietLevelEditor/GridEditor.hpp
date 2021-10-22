#pragma once

#include <QDialog>
#include <limits>
#include "LevelDataManager.hpp"

inline const float EPSILON_FLOAT = std::numeric_limits<float>::epsilon();

namespace Ui {
class GridEditor;
}

class GridEditor : public QDialog
{
    Q_OBJECT
public:
    explicit GridEditor(QWidget *parent = nullptr);
    bool initGrid(const QString &installDir);
    ~GridEditor();
private:
    Ui::GridEditor *ui;
    LevelDataManager m_levelDataManager;
};
