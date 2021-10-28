#pragma once

#include <QDialog>
#include <limits>
#include "LevelDataManager.hpp"

class TableModel;
inline const float EPSILON_FLOAT = std::numeric_limits<float>::epsilon();

namespace Ui {
class GridEditor;
}

class GridEditor : public QDialog
{
    Q_OBJECT
public:
    explicit GridEditor(QWidget *parent = nullptr);
    bool initGrid(const QString &installDir, int levelWidth, int levelHeight);
    ~GridEditor();
private:
    void adjustTableSize();
    void setStdTableSize();
private:
    Ui::GridEditor *ui;
    LevelDataManager m_levelDataManager;
    TableModel *m_tableModel;
    const int32_t CASE_SIZE_PX = 50;
};
