#include "SelectableLineLayout.hpp"
#include <QRadioButton>
#include <QComboBox>

//======================================================================
SelectableLineLayout::SelectableLineLayout(const QString &radioBoxTxt, LevelElement_e typeElement, GridEditor *parent) :
    m_radio(new QRadioButton(radioBoxTxt, parent)),
    m_comboBox(new QComboBox(parent)),
    m_elementType(typeElement)
{
    addWidget(m_radio);
    m_comboBox->setEnabled(false);
    addWidget(m_comboBox);
    QObject::connect(m_radio, &QRadioButton::toggled, m_comboBox, &QComboBox::setEnabled);
    QObject::connect(m_radio, &QRadioButton::pressed, this, &SelectableLineLayout::selected);
    QObject::connect(m_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedIndex(int)));}

//======================================================================
void SelectableLineLayout::setIcons(const QVector<QIcon> &vectIcons)
{
    for(int32_t i = 0; i < vectIcons.size(); ++i)
    {
        m_comboBox->addItem(vectIcons[i], "D");
    }
}

//======================================================================
void SelectableLineLayout::confWallSelectWidget(GridEditor *parent)
{
    m_wallComboBox = new QComboBox(parent);
    addWidget(m_wallComboBox);
    m_wallComboBox->setEnabled(false);
    QObject::connect(m_radio, &QRadioButton::toggled, m_wallComboBox, &QComboBox::setEnabled);
    m_wallComboBox->addItems({"Rect/Line", "Diagonal Line", "Diagonal Rect"});
    QObject::connect(m_wallComboBox, SIGNAL(currentIndexChanged(int)), parent, SLOT(setWallDrawModeSelected(int)));
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
