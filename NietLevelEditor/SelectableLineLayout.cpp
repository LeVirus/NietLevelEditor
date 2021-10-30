#include "SelectableLineLayout.hpp"
#include <QRadioButton>
#include <QComboBox>

//======================================================================
SelectableLineLayout::SelectableLineLayout(const QString &radioBoxTxt, QWidget *parent) :
    m_radio(new QRadioButton(radioBoxTxt, parent)),
    m_comboBox(new QComboBox(parent))
{
    addWidget(m_radio);
    m_comboBox->setEnabled(false);
    addWidget(m_comboBox);
    QObject::connect(m_radio, &QRadioButton::toggled, m_comboBox, &QComboBox::setEnabled);
}
