#pragma once

#include <QDialog>
#include <optional>

namespace Ui {
class GlobalLevelConfForm;
}

class QComboBox;

struct GlobalLevelData
{
    uint32_t m_levelNum;
    QString m_prologue, m_epilogue, m_epilogueMusic;
};

class GlobalLevelConfForm : public QDialog
{
    Q_OBJECT
public:
    explicit GlobalLevelConfForm(QWidget *parent = nullptr);
    void init(const QComboBox *musicWidget, std::optional<GlobalLevelData> data = {});
    inline bool valid()const
    {
        return m_valid;
    }
    inline GlobalLevelData getData()const
    {
        return m_data;
    }
    ~GlobalLevelConfForm();
private:
    void linkButtons();
private slots:
    void validateForm();
private:
    Ui::GlobalLevelConfForm *ui;
    bool m_valid;
    GlobalLevelData m_data;
};

