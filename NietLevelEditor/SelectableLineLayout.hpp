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
    void setIcons(const QVector<DisplayData> &vectIcons, bool doorCase = false);
    void confWallSelectWidget(GridEditor *parent);
    void confEnemySelectWidget(GridEditor *parent);
    void confDoorSelectWidget(GridEditor *parent, const QVector<DisplayData> &cardData);
    const DisplayData &getSelectedCardDoor()const;
    QComboBox *confPlayerDeparture(GridEditor *parent);
    void uncheckCheckBox();
    void uncheckCheckBoxDoor();
    void reinitWallComp();
    std::optional<int> getSelected();
    void setRadioButtonEnabled(bool enable);
    void uncheckRadioButton();
    bool isEndLevelEnemyChecked();
    Q_SIGNAL void lineSelected(LevelElement_e, int);
private slots:
    void selected();
    void selectedIndex(int currentIndex);
private:
    const QVector<DisplayData> *m_memCardData;
    QRadioButton *m_radio;
    QComboBox *m_comboBox;
    LevelElement_e m_elementType;
    QComboBox *m_wallComboBox = nullptr, *m_comboDoorCard = nullptr;
    QCheckBox *m_wallCheckBox = nullptr, *m_finishLevelCheckBox = nullptr,
    *m_cardDoorCheckBox = nullptr;
};
