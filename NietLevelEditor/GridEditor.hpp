#pragma once

#include <QDialog>
#include <QMap>
#include <functional>
#include <limits>

using ArrayFloat_t = std::array<float, 5>;
inline const float EPSILON_FLOAT = std::numeric_limits<float>::epsilon();

class QSettings;

namespace Ui {
class GridEditor;
}

class GridEditor : public QDialog
{
    Q_OBJECT

public:
    explicit GridEditor(QWidget *parent = nullptr);
    bool initGrid(const QString &installDir);
    ~GridEditor();
private:
    void clear();
    bool loadPictureDataINI();
    bool loadStandardDataINI();
private:
    Ui::GridEditor *ui;
    QSettings *m_standardSettingINI = nullptr, *m_pictureDataINI = nullptr;
    QString m_installDirectory;
    QStringList m_texturesPath;
    QMap<QString, ArrayFloat_t> m_memPictureElement;
};
