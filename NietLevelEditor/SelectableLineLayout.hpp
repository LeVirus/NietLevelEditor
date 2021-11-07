#pragma once

#include <QHBoxLayout>
#include "GridEditor.hpp"

class QRadioButton;
class QComboBox;
class GridEditor;
class QCheckBox;

class SelectableLineLayout : public QHBoxLayout
{
    Q_OBJECT
public:
    SelectableLineLayout(const QString &radioBoxTxt, LevelElement_e typeElement, GridEditor *parent);
    void setIcons(const QVector<QIcon> &vectIcons);
    void confWallSelectWidget(GridEditor *parent);
    void uncheckMoveableWall();
    Q_SIGNAL
    void lineSelected(LevelElement_e, int);
    std::optional<int> getSelected();
    void setRadioButtonEnabled(bool enable);
private slots:
    void selected();
    void selectedIndex(int currentIndex);
private:
    QRadioButton *m_radio;
    QComboBox *m_comboBox;
    LevelElement_e m_elementType;
    QComboBox *m_wallComboBox = nullptr;
    QCheckBox *m_wallCheckBox = nullptr;
};
