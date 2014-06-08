#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
    class Settings;
}

class QCheckBox;
class QSpinBox;
class QLabel;

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();
    void loadSettingsToUi(const IniSettingMap &set);

    Ui::Settings *ui;
    IniSettingMap DefaultConfig;

private:
    void closeEvent(QCloseEvent * e);
    void updateLabelCacheSize();

    QColor SBackColor,SShadowColor,MBackColor;
    bool flagReloadMap;

#ifdef QMETRO_HAVE_UPDATE
    QCheckBox *checkUpdate;
    QSpinBox *spinDays;
    QLabel *labelLastCheckDate;
#endif

protected:
    void keyPressEvent(QKeyEvent *e);
    void showEvent(QShowEvent *e);

signals:
    void closed();
    void showed();

private slots:

    void on_bMEmptyExit_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_bSFont_clicked();
    void on_bSBackColor_clicked();
    void on_bSShadowColor_clicked();
    void on_bMBackColor_clicked();

    IniSettingMap changeSettings();

    QFont getFontFromLabel(QLabel *Label);
    void setFontToLabel(QLabel *Label,QFont Font);
    void on_bDefaultConfig_clicked();
    void setFlagReloadMap(){ flagReloadMap=1; }

};

#endif // SETTINGS_H
