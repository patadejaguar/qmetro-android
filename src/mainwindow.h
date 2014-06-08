#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mapview/mapview.h"

#ifdef QMETRO_HAVE_UPDATE
#include "update/update.h"
#endif

namespace Ui {
    class MainWindow;
}

enum IniGroup{igMapView,igMain,igOther};
// Add enum for assigned IniKeyStr
enum IniKey{ikSkin,ikStationCustomFont,ikStationFont,ikStationBack,ikStationShadow,ikSBackColor,ikSShadowColor,
            ikSUpperCase,ikSWordWrap,ikSTransparency,
            ikMShowScrollbar,ikWFullScreen,ikMShowTime,ikMShowBackground,ikMAntialiasing,ikMFastScrolling,
            ikMBackColor,ikMSimpleTransfers,ikMCleanCache,ikMLastFile,ikMCaching,ikWUiScale,ikWUiButtonScale,ikMSwipeRoutes};

typedef QMap<IniKey,QString> IniKeyMap;
typedef QMap<IniGroup,IniKeyMap> IniSettingMap;

void applySettings(const IniSettingMap &settings);
void saveSettings(const IniSettingMap &settings);
void loadSettings();

bool removeDir(const QString &DirName);
qint64 sizeOfDir(const QString &DirName);

class TElement;
class TCompleter;
class Settings;
class TMobileUi;
class TMobileUi;

class MainWindow : public QMainWindow
{Q_OBJECT
friend class TMobileUi;
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //void showEvent(QShowEvent * event);

    QTextCodec *codePage;
    TMapView *map;
    Settings *SettingsWindow;

    struct TConfig
    {
     QString locale;
     int dockWidth;
     qreal mapZoomFactor;
     QPoint mapPosition;
     bool mapZoomFactorCheck;
     bool mapPositionCheck;
     bool mapKineticScrollCheck;
     int mapDefaultStationID;

     QFont appFont;
     qreal appUiScale;
     qreal appUiButtonScale;
     int appIconSize;
     bool appFullscreen;
     bool appFirstStart;
     QString appSkinName;

     bool appAlarmEnable;
     QString appAlarmName;
     int appAlarmAlertInMinute;

     QSize mainWindow;
     QSize settingWindow;
    };
    TConfig Config;

    char MobileVersion;
    Ui::MainWindow *ui;
    TMobileUi *mUi;

    QList<QIcon> actionIcons;

#ifdef QMETRO_HAVE_UPDATE
 TUpdate *Updater;
#endif

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void closeEvent(QCloseEvent * e);

    TCompleter *Completer;
    Qt::GestureType m_gestureId;

public slots:
    void setScaleForUi(qreal UiScale=0.0,qreal UiButtonScale=0.0);
    void setScaleForUi(int UiScale) { setScaleForUi((qreal)(UiScale/100.0)); }
    void setScaleForUiButtons(int UiScale) { setScaleForUi(Config.appUiScale,(qreal)(UiScale/100.0)); }
    void scaleFont(QFont &Font, const qreal &Factor);
    void toggleLang(QString locale="");
    void setNodeDefault(bool checked,TMapNode *Node=NULL);
    void loadNodeDefault();
    void checkNodeDefault();
    void toFullScreen();
    void showPageLoading();
    void showPageMap();

    void updateMapsFromZips();
    void updateMapsAndReload();
    void reloadMap();

    void on_mOpen_triggered();
    void on_mSave_triggered();
    void on_mPreference_triggered();
    void on_mAddMap_triggered();
    void on_mAbout_triggered();

    QString getMessageAddMap();
    QString getMessageInstalledMaps();
    QString getMapDefaultDir();
    void openMap(QString fileName="");
    void scaleMap(qreal scale)
    {
    #ifdef QML_EDITION
     QList<TMapView *> m =  findChildren<TMapView *>();
     if(m.count()) m.at(m.count()-1)->scaleMap(scale);
    #else
     map->scaleMap(scale);
    #endif
    }
    void loadSkin(QString skinName="");

private slots:

    void on_mFullScreen_toggled(bool checked);

    void on_bClear_clicked();
    void on_mFind_toggled(bool checked);
    void on_eSearch_returnPressed();
    void on_mTest_triggered();
    void on_mExit_triggered();
    void setDelayNames(QStringList list);
    void setTrackNames(QStringList list);
    void nodeInformation();
};

#endif // MAINWINDOW_H
