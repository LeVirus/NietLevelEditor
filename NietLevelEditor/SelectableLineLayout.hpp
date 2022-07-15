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
    SelectableLineLayout(const QString &radioBoxTxt, LevelElement_e typeElement, GridEditor *parent = nullptr);
    void setIcons(const QVector<DisplayData> &vectIcons);
    void confWallSelectWidget(GridEditor *parent);
    void confEnemySelectWidget(GridEditor *parent);
    void confPlayerDeparture(GridEditor *parent);
    void uncheckCheckBox();
    void reinitWallComp();
    void setWallWidgetsEnabled(bool enable);
    std::optional<int> getSelected();
    void setRadioButtonEnabled(bool enable);
    void uncheckRadioButton();
    Q_SIGNAL void lineSelected(LevelElement_e, int);
private slots:
    void selected();
    void selectedIndex(int currentIndex);
private:
    QRadioButton *m_radio;
    QComboBox *m_comboBox;
    LevelElement_e m_elementType;
    QComboBox *m_wallComboBox = nullptr;
    QCheckBox *m_wallCheckBox = nullptr, *m_finishLevelCheckBox = nullptr;
};
