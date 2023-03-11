#include "SelectableLineLayout.hpp"
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

//======================================================================
SelectableLineLayout::SelectableLineLayout(const QString &radioBoxTxt,
                                           LevelElement_e typeElement, GridEditor *parent) :
    m_radio(new QRadioButton(radioBoxTxt, parent)),
    m_comboBox(new QComboBox(parent)),
    m_elementType(typeElement)
{
    addWidget(m_radio);
    QObject::connect(m_radio, &QRadioButton::toggled, m_comboBox, &QComboBox::setEnabled);
    QObject::connect(m_radio, &QRadioButton::clicked, this, &SelectableLineLayout::selected);
    if(m_elementType != LevelElement_e::CHECKPOINT && m_elementType != LevelElement_e::SECRET &&
            m_elementType != LevelElement_e::DELETE && m_elementType != LevelElement_e::PLAYER_DEPARTURE &&
            m_elementType != LevelElement_e::SELECTION)
    {
        m_comboBox->setEnabled(false);
        addWidget(m_comboBox);
        QObject::connect(m_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedIndex(int)));
    }
}

//======================================================================
void SelectableLineLayout::setIcons(const QVector<DisplayData> &vectIcons)
{
    for(int32_t i = 0; i < vectIcons.size(); ++i)
    {
        m_comboBox->addItem(vectIcons[i].m_icon, "");
    }
}

//======================================================================
void SelectableLineLayout::confWallSelectWidget(GridEditor *parent)
{
    m_wallComboBox = new QComboBox(parent);
    m_wallCheckBox = new QCheckBox(parent);
    addWidget(m_wallComboBox);
    m_wallComboBox->setEnabled(false);
    QObject::connect(m_radio, &QRadioButton::toggled, m_wallComboBox, &QComboBox::setEnabled);
    QObject::connect(m_radio, &QRadioButton::toggled, m_wallCheckBox, &QComboBox::setEnabled);
    m_wallComboBox->addItems({"Rect/Line", "Diagonal Line", "Diagonal Rect"});
    QObject::connect(m_wallComboBox, SIGNAL(currentIndexChanged(int)), parent, SLOT(setWallDrawModeSelected(int)));
    addWidget(new QLabel("Moveable"));
    addWidget(m_wallCheckBox);
    QObject::connect(m_wallCheckBox, SIGNAL(stateChanged(int)), parent, SLOT(setWallMoveableMode(int)));
    m_wallCheckBox->setEnabled(false);
}

//======================================================================
void SelectableLineLayout::confEnemySelectWidget(GridEditor *parent)
{
    m_finishLevelCheckBox = new QCheckBox(parent);
    m_finishLevelCheckBox->setEnabled(false);
    QObject::connect(m_radio, &QRadioButton::toggled, m_finishLevelCheckBox, &QComboBox::setEnabled);
    addWidget(new QLabel("End level"));
    addWidget(m_finishLevelCheckBox);
}

//======================================================================
void SelectableLineLayout::confDoorSelectWidget(GridEditor *parent, const QVector<DisplayData> &cardData)
{
    m_cardDoorCheckBox = new QCheckBox(parent);
    m_cardDoorCheckBox->setEnabled(false);
    addWidget(new QLabel("Card"));
    addWidget(m_cardDoorCheckBox);
    QComboBox *combo = new QComboBox(parent);
    for(int i = 0; i < cardData.size(); ++i)
    {
        combo->addItem(cardData[i].m_icon, "");
    }
    addWidget(combo);
    QObject::connect(m_radio, &QRadioButton::toggled, m_cardDoorCheckBox, &QCheckBox::setEnabled);
    QObject::connect(m_radio, &QRadioButton::toggled, combo, &QComboBox::setEnabled);
}

//======================================================================
QComboBox *SelectableLineLayout::confPlayerDeparture(GridEditor *parent)
{
    QComboBox *combo = new QComboBox(parent);
    combo->addItem("NORTH");
    combo->addItem("WEST");
    combo->addItem("SOUTH");
    combo->addItem("EAST");
    addWidget(combo);
    QObject::connect(combo, SIGNAL(currentIndexChanged(int)), parent, SLOT(memPlayerDirection(int)));
    QObject::connect(m_radio, &QRadioButton::toggled, combo, &QComboBox::setEnabled);
    combo->setEnabled(false);
    return combo;
}

//======================================================================
void SelectableLineLayout::uncheckCheckBox()
{
    if(m_wallComboBox)
    {
        m_wallCheckBox->setCheckState(Qt::CheckState::Unchecked);
    }
    else if(m_finishLevelCheckBox)
    {
        m_finishLevelCheckBox->setCheckState(Qt::CheckState::Unchecked);
    }
}

//======================================================================
void SelectableLineLayout::reinitWallComp()
{
    m_wallComboBox->setCurrentIndex(0);
}

//======================================================================
std::optional<int> SelectableLineLayout::getSelected()
{
    if(!m_comboBox)
    {
        return {};
    }
    return m_comboBox->currentIndex();
}

//======================================================================
void SelectableLineLayout::setRadioButtonEnabled(bool enable)
{
    m_radio->setEnabled(enable);
}

//======================================================================
void SelectableLineLayout::uncheckRadioButton()
{
    m_radio->setAutoExclusive(false);
    m_radio->setChecked(false);
    m_radio->setAutoExclusive(true);
}

//======================================================================
bool SelectableLineLayout::isEndLevelEnemyChecked()
{
    return m_finishLevelCheckBox && m_finishLevelCheckBox->isChecked();
}

//======================================================================
void SelectableLineLayout::selectedIndex(int currentIndex)
{
    emit lineSelected(m_elementType, currentIndex);
}

//======================================================================
void SelectableLineLayout::selected()
{
    emit lineSelected(m_elementType, m_comboBox->currentIndex());
}
