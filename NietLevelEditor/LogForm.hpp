#pragma once

#include <QDialog>

namespace Ui {
class LogForm;
}

class LogForm : public QDialog
{
    Q_OBJECT

public:
    explicit LogForm(QWidget *parent = nullptr);
    void reinit();
    inline bool validate()const
    {
        return m_valid;
    }
    inline QString getMessage()const
    {
        return m_messageText;
    }
    void setMessage(const QString &message);
    ~LogForm();
private slots:
    void valid();
    void cancel();
private:
    QString m_messageText;
    bool m_valid;
    Ui::LogForm *ui;
};
