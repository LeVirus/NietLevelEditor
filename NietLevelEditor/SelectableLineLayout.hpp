#pragma once

#include <QHBoxLayout>

class QRadioButton;
class QComboBox;

class SelectableLineLayout : public QHBoxLayout
{
public:
    SelectableLineLayout(const QString &radioBoxTxt, QWidget *parent);
private:
    QRadioButton *m_radio;
    QComboBox *m_comboBox ;
};
