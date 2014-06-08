#ifndef ABOUT_H
#define ABOUT_H

#include <qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif

#include <QDialog>

#include "mainwindow.h"
#include "mapview/maplayer.h"
#include "flickcharm.h"

#ifdef Q_OS_SYMBIAN
 #define _DEF( x ) QString(x).remove('"') // Symbian 474 "\"TEXT\""
#else
 #define _DEF( x ) QString(x)
#endif

#ifdef QMETRO_HAVE_ALARM
#include "alarm/audiooutput.h"
#endif

#ifdef QMETRO_HAVE_VIBRO
//#include "alarm/vibro.h"
#endif

//#ifdef Q_OS_SYMBIAN
//extern QString VERSION;
//#endif
extern MainWindow *mainWnd;
extern QString appPath,URL_MAPS,SKIN_DEFAULT,URL_APP;

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent): QDialog(parent)
    {
#ifdef QMETRO_HAVE_ALARM
     Player=NULL;
#endif
     //Logo= new QLabel();
     OK=NULL;
     Info= new QTextBrowser();
     //Info->setStyleSheet("border:1px solid grey;");
     setWindowTitle(tr("About")+QString(" %1").arg(_DEF(APPNAME)));
#ifdef Q_WS_MAEMO_5
     setAttribute(Qt::WA_Maemo5StackedWindow);
     setWindowFlags(Qt::Window);
     setAttribute(Qt::WA_Maemo5AutoOrientation);
#else
     FlickCharm *fcharm=new FlickCharm(Info);
     fcharm->activateOn(Info);
#endif

     setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
     setAttribute(Qt::WA_DeleteOnClose);
     setMinimumWidth(260);
     setMinimumHeight(220);
     resize(500,400);

     //Logo->setFixedWidth(64);
     //Logo->setContentsMargins(0,5,0,0);
     //Logo->setPixmap(QPixmap(":/qmetro.png"));

     QString AuthorOfSkin;
     if(mainWnd->Config.appSkinName!=SKIN_DEFAULT)
     {
      QSettings ini(appPath+"skin/"+mainWnd->Config.appSkinName+"/main.ini",QSettings::IniFormat);
      AuthorOfSkin=ini.value("Main/Author").toString();
      if(!AuthorOfSkin.isEmpty()) AuthorOfSkin="<br><br>"+tr("Author of skin")+":<br>"+AuthorOfSkin.remove(QRegExp("<[^>]*>"));
     }

     QString LIBS,MODULES=" (Core,Gui";

#ifdef QMETRO_HAVE_UPDATE
     MODULES+=",Network";
#endif
#ifdef QMETRO_HAVE_ALARM
     MODULES+=",Multimedia";
     LIBS+="<li>&nbsp;libmodplug";
#endif
#ifdef QMETRO_HAVE_NFC
     MODULES+=",Connectivity";
#endif

     MODULES+=")";
     QString hr=QString("<p><table height=\"2\" width=\"100%\" bgcolor=\"%1\"><tr><td></table></p>")
                .arg(palette().color(QPalette::Mid).name());
     QString text=
     QString("<table><tr><td align=\"center\" style=\"padding-right:6px;\">"
             "<img "+(0 ? QString("width=\"%1\" height=\"%1\"").arg(mainWnd->Config.appIconSize):"")+" align=\"left\" src=\":/qmetro.png\">"
             "<a href=\"https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=9EZQ47YPVAF36\">"
             "<img "+(0 ? QString("width=\"%1\" height=\"%2\"").arg(mainWnd->Config.appIconSize).arg(mainWnd->Config.appIconSize/1.6):"")+" src=\":/img/donate.png\"></a>"
             "<td><p><font size=6><b>"+_DEF(APPNAME)+"</b></font>"
//          +"<br><sup><font size=5 color=\"#a90105\">"+tr("Happy New Year!")+"</font></sup>"+
             "<br>%4"
             "<br><br>%5 %1<br>( "
            +_DEF(QMETRO_PLATFORM)+
             " )<br>&copy; 2011-2013<br><a href=\"mailto:guuleon@gmail.com\">Gusak Leonid</a><br>"
             "<a href=\""+URL_APP+"\">"+URL_APP.remove("http://")+"</a></table></p>"
            +(mainWnd->Config.locale!="ru" ? "":"<br><center><table border=1><tr><td><small>WMR133269969667<small></table></center>")+

             "<hr><br>%6:<br><a href=\"mailto:Boris@pMetro.su\">Muradov Boris</a>"
             " (<a href=\"%10\">pMetro</a>)"
             "<br><br>%11"
            + AuthorOfSkin +
             "<hr><b>%7 ("+QString("%1/%2").arg( mainWnd->map->nodeByID.count() ).arg( (mainWnd->map->layer.count() ? ((TMapLayer*)mainWnd->map->layer.at(0))->route.count() : 0) ) +"):</b>"
             "<p>%3</p>"
             "<hr><b>%8:</b><ul type=\"circle\"><li>&nbsp;Qt %2 %12</ul>"
             "<hr>%9")
     .arg(_DEF(VERSION)).arg(QString(QT_VERSION_STR)+MODULES).arg((mainWnd->map->params.mapAuthors.isEmpty() ? tr("Unknown author"):mainWnd->map->params.mapAuthors)) // 1
     .arg(tr("transit system map")).arg(tr("Version")).arg(tr("Author of map file format")) // 3
     .arg(tr("About current map file")).arg(tr("This program uses")) // 6
     .arg(tr("This program is distributed in the hope that it will be useful, "
             "but WITHOUT ANY WARRANTY; without even the implied warranty of "
             "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
             "GNU General Public License for more details.")).arg(URL_MAPS) // 9 10
     .arg(tr("Author of translation: Gusak Leonid").replace(": ",":<br>")) // 11
     .arg("<li>&nbsp;zlib + OSDaB"+LIBS); // 12

     text.replace("<hr>",hr);

//     Info->setAttribute(Qt::WA_NoSystemBackground);
     QPalette pal = Info->palette();
     pal.setBrush(QPalette::Base, palette().color(QPalette::Window));
     pal.setColor(QPalette::Text,palette().color(QPalette::Text));
     Info->setPalette(pal);

     Info->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
     Info->setOpenExternalLinks(1);
     Info->setReadOnly(1);
     Info->setFrameShape(QFrame::NoFrame);
     Info->setText(text);

     //HLayout= new QHBoxLayout;
     VLayout= new QVBoxLayout;
     VLayout->setMargin(2);
     //HLayout->addWidget(Logo);
     //HLayout->setAlignment(Logo,Qt::AlignTop);
     VLayout->addWidget(Info);
//#ifndef Q_WS_MAEMO_5
     if(mainWnd->MobileVersion)
     {
      OK = new QPushButton(tr("OK"));
      //connect(OK,SIGNAL(clicked()),mainWnd,SLOT(toFullScreen()));
      connect(OK,SIGNAL(clicked()),this,SLOT(close()));
      connect(OK,SIGNAL(clicked()),mainWnd,SLOT(showPageMap()));
      VLayout->addWidget(OK);
     }
//#endif
     setLayout(VLayout);
     if(OK) OK->setFocus();

#ifdef QMETRO_HAVE_VIBRO
// VibroFeedback vibro;
// vibro.start();
#endif

#ifdef QMETRO_HAVE_ALARM
     playerCreate();
   //Player->loadAudioFile(":/test.xmz");

 #ifdef Q_OS_SYMBIAN
//        qDebug() << "ATTENTION: Stupid rule of Nokia QA!";
        connect(qApp,SIGNAL(appFocusGained()),this,SLOT(playerCreate()));
        connect(qApp,SIGNAL(appFocusLost()),this,SLOT(playerDelete()));        
 #endif
#endif

    }
    ~About()
    {
     if(OK) delete OK;
     delete Info;
#ifdef QMETRO_HAVE_ALARM
     hide();
     playerDelete();
#endif

    }

private:
//    void closeEvent(QCloseEvent *e)
//    {Q_UNUSED(e);
//     deleteLater();
//    }

    QHBoxLayout *HLayout;
    QVBoxLayout *VLayout;
    //QDialogButtonBox *OK;
    QPushButton *OK;
    //QLabel *Logo;
    QTextBrowser *Info;

#ifdef QMETRO_HAVE_ALARM
 AudioPlay *Player;

public slots:

 void playerDelete() { Player->deleteLater(); Player=0; }
 void playerCreate()
 {
  if(!Player)
  {
   Player = new AudioPlay(":/audio/xmas.mdz",1);
   QTimer::singleShot(5000,Player,SLOT(resumePlay()));
  }
 }

#endif

protected:
    void keyPressEvent(QKeyEvent *e)
    {
     if(e->key()==Qt::Key_Escape) close();
    }

//    bool event(QEvent *e)
//    {
//     if(e->type()==QEvent::WindowDeactivate)
//     {
//      close();
//      return true;
//     }
//     return QWidget::event(e);
//    }
};

#endif // ABOUT_H
