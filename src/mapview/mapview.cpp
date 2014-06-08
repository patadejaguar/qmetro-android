#if OPENGL_ENABLE
#include <QtOpenGL>
#endif

#include <QDebug>
#include <QApplication>
#include <QSettings>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QTextBrowser>
#include <QToolBox>
#include <QFileDialog>
#include <QTextStream>
#include <QImageWriter>
#include <QImageReader>

#include <QWheelEvent>
#include <math.h>
#include <QMessageBox>
#include <QTimer>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QTextCodec>
#include <QQueue>
#include <QPair>
#include <QScrollBar>
#include <QTouchEvent>

#include "../zip/unzip.h"
#include "../ini/inisettings.h"
#include "../flickcharm.h"

#include "mapview.h"
#include "mapgraphics.h"
#include "maproute.h"
#include "mapnode.h"
#include "mapedge.h"
#include "maplayer.h"
#include "mapelement.h"

//#if defined(Q_WS_MAEMO_5)
//#include <QtGui/QX11Info>
//#include <X11/Xlib.h>
//#include <X11/Xatom.h>
//#endif

QTime testTimer;

qreal INFINIT=1000000000;
const QPainter::RenderHints Antialiasing=QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform;
const QPainter::RenderHints NonAntialiasing=QPainter::NonCosmeticDefaultPen;

QString ru="ru";

TMapView::TMapView(QString AppPath, QString CachePath) :
  CachedMap(NULL),nodeBeginID(-1),nodeEndID(-1),
  nodeBegin(NULL),nodeEnd(NULL),nodeClick(NULL),
  mapFade(NULL),nodePopup(NULL),mapWidth(0),mapHeight(0)
{
 hide();
 setStyleSheet("");
 setAttribute(Qt::WA_OpaquePaintEvent);
 setAttribute(Qt::WA_AcceptTouchEvents);
 viewport()->grabGesture(Qt::PinchGesture);
 viewport()->grabGesture(Qt::SwipeGesture);
 viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
 //setAttribute(Qt::WA_NoSystemBackground);
 //setAttribute(Qt::WA_PaintOnScreen);
#if OPENGL_ENABLE //defined(Q_WS_MAEMO_5) //defined(Q_OS_WIN32) //!(defined Q_OS_WINCE || defined Q_OS_SYMBIAN)
 if(QGLFormat::hasOpenGL())
 {
  QGLFormat fmt = QGLFormat::defaultFormat();
  fmt.setDirectRendering(1);
  fmt.setDoubleBuffer(1);
  QGLWidget *gl=new QGLWidget(fmt);
  setViewport(gl);
 }
#endif
 
#if defined(Q_OS_WINCE) || defined(Q_OS_SYMBIAN) || defined(Q_WS_MAEMO_5)
 config.MobileVersion=1;
#else
 config.MobileVersion=0;
#endif

#ifdef Q_WS_MAEMO_5
 setAttribute(Qt::WA_Maemo5NonComposited);
#endif

 config.locale=QLocale::system().name().section("_",0,0);
 initTranslit();

 scene = new QGraphicsScene(this);
//qDebug() << 1;
// QGraphicsWidget *widget = new QGraphicsWidget();
// //widget.setLayout(scene->);
// widget->setPos(0,0);
//qDebug() << 2;
// PannableView *pan=new PannableView(Qt::Vertical | Qt::Horizontal,800,480);
/////QObject::connect(this, SIGNAL(), pannableView, SLOT(moveToItem(QGraphicsItem*)));
//qDebug() << 3;
// pan->setWidget(widget);
// //scene->setActiveWindow(&widget);
// scene->addItem( pan );
// scene->setActiveWindow(widget);
// scene->setActivePanel(widget);
// //setParent((QWidget*)widget);
// qDebug() << 4;
//// setViewport((QWidget*)pan);

 //scene->setItemIndexMethod - setup later at load
 setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);
 setOptimizationFlag(QGraphicsView::DontSavePainterState);
 scene->setSceneRect(0, 0, width(), height());
 setScene(scene);
 setFrameShape(NoFrame);
 setAlignment(Qt::AlignCenter);
 //setCacheMode(QGraphicsView::CacheBackground);
 //setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
 //setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
 //setViewportUpdateMode(BoundingRectViewportUpdate);
 //setViewportUpdateMode(FullViewportUpdate);
 //if(config.MobileVersion) setInteractive(0);
 setDragMode(QGraphicsView::ScrollHandDrag);
#ifndef QML_EDITION
 setTransformationAnchor(AnchorUnderMouse);
#endif
 setResizeAnchor(AnchorViewCenter);

 state.mapCached=0;
 state.mapDragging=0;
 params.delayIndex=params.delayToMetro=0;
 params.appPath=AppPath;
 params.appCachePath=CachePath;
 codePage = QTextCodec::codecForName("Windows-1251");
 
 TimerPopup.setInterval(1000);
 TimerPopup.setSingleShot(1);
 connect(&TimerPopup, SIGNAL(timeout()), this, SLOT(popupByTimer()));
 TimerDragging.setInterval(1000);
 TimerDragging.setSingleShot(1);
 connect(&TimerDragging, SIGNAL(timeout()), this, SLOT(mapStopDragging()));
 
 QPalette palette;
 palette.setColor(backgroundRole(), Qt::white);
 setPalette(palette);

 connect(horizontalScrollBar(),SIGNAL(sliderMoved(int)),this,SLOT(mapStartDragging()));
 connect(verticalScrollBar(),SIGNAL(sliderMoved(int)),this,SLOT(mapStartDragging()));
}

void TMapView::mapStartDragging()
{
 if(!state.mapDragging )
 {
  if(config.enableLowqDrag && config.enableAntialiasing)
  {
   //params.nodeFont.setStyleStrategy(QFont::NoAntialias);
   //params.nodeFont.setStyleStrategy(QFont::PreferDefault);
   setRenderHints(NonAntialiasing);
  }
  state.mapDragging=1;
 }
// TimerPopup.stop();
// fcharm->deactivateFrom(this);
 TimerDragging.start();
}

void TMapView::mapStopDragging()
{
 if(state.mapDragging )
 {
  if(config.enableLowqDrag)
  {
   if(config.enableAntialiasing)
   {
    setRenderHints(Antialiasing);
    viewport()->update();
    //foreach(TMapNode *node,nodeByID) node->update(); // dont need in 4.7.2 ???
   }
//   if(config.enableKineticScroll) fcharm->activateOn(this);
  }
  state.mapDragging=0;
 }
}

TMapView::~TMapView()
{
 clearMap();
}

void TMapView::keyPressEvent(QKeyEvent *event)
{
//    qDebug() << event->key();
    switch (event->key())
    {
    case Qt::Key_Equal:
    case Qt::Key_Plus:
    case Qt::Key_ZoomIn:
    case Qt::Key_VolumeDown:
//    case Qt::Key_F7: // maemo volUp 16777270 // dont work
        if(!config.enableZoomKeys) break;
        if(mapTrack.count()>1)
             showNextTrack();
        else zoomIn();
        break;

    case Qt::Key_Minus:
    case Qt::Key_ZoomOut:
    case Qt::Key_VolumeUp:
//    case Qt::Key_F8: // maemo volDown 16777271 // dont work
        if(!config.enableZoomKeys) break;
        if(mapTrack.count()>1)
             showPreviousTrack();
        else zoomOut();
        break;

    case Qt::Key_Right:
     if(mapTrack.count()>1)
          showNextTrack();
     else QGraphicsView::keyPressEvent(event);
     break;
    case Qt::Key_Left:
     if(mapTrack.count()>1)
          showPreviousTrack();
     else QGraphicsView::keyPressEvent(event);
     break;
    case '*':
        (config.MobileVersion ? zoomIn():fitMap());
        break;
    case '#':
        zoomOut();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
    }
}

void TMapView::wheelEvent(QWheelEvent *event)
{
 if(event->buttons()==Qt::MidButton)
 {
  int offset=event->delta();
  mapStartDragging();
  if (event->orientation() == Qt::Horizontal)
   horizontalScrollBar()->setValue(horizontalScrollBar()->value()-offset);
  else
   verticalScrollBar()->setValue(verticalScrollBar()->value()-offset);
 }
 else
  scaleMap(pow(qreal(2), event->delta()/qreal(240)));

}


//void drawBackgroundImage(QPainter* p,const QRectF& clip,QImage *bgimage)
//{
// p->eraseRect(clip);

// QMatrix wm(p->worldMatrix());

// if(wm.m11()!=1 && clip.intersects(bgimage->rect()))
// {
//  QImage img;
//  QRectF c( clip );
//  if( wm.m11() > 1 )
//   c.adjust( -1, -1, 1, 1 );

//  qreal cx = c.x();
//  qreal cy = c.y();
//  qreal cw = qMax( 16., c.width() );
//  qreal ch = qMax( 16., c.height() );

//  if( cx < 0 )
//  {
//   cw += cx;
//   cx = 0;
//  }
//  if( cy < 0 )
//  {
//   ch += cy;
//   cy = 0;
//  }
//  cw = qMin( cw, bgimage->width() - cx );
//  ch = qMin( ch, bgimage->height() - cy );
//  img = bgimage->copy( QRectF( cx, cy, cw, ch ).toRect() );
//  cw = qRound( cw * wm.m11() );
//  ch = qRound( ch * wm.m11() );
//  img = img.scaled( (int) cw, (int) ch, Qt::IgnoreAspectRatio,
//                    wm.m11() > .8 ? Qt::SmoothTransformation : Qt::FastTransformation );
//  p->setWorldMatrix( QMatrix( 1, 0, 0, 1, wm.dx(), wm.dy() ) );
//  p->drawImage( QPointF( cx * wm.m11(), cy * wm.m11() ), img );
//  p->setWorldMatrix( wm );
// }	else
// {
//  p->drawImage(
//     QPointF( clip.x(), clip.y() ),
//     *bgimage,
//     QRectF( clip.x(), clip.y(),
//             qMin( clip.width(), qreal( bgimage->width() ) ),
//             qMin( clip.height(), qreal( bgimage->height() ) ) ) );
// }
//}


void TMapView::drawBackground(QPainter *painter, const QRectF &rect)
{
// Q_UNUSED(rect);
// testTimer.start();

  if(state.mapLoaded)
  {
   if(config.enableCaching)
   {

    if(!mapWidth || !mapHeight) drawPaintCmd(painter,paintBkg);
    if(!mapWidth || !mapHeight)
    {
     mapWidth=sceneRect().width();
     mapHeight=sceneRect().height();
    }
    if(mapRect.isNull()) mapRect=QRect(-100,-100,mapWidth+200,mapHeight+200);

    if(state.mapCached)
    {
     painter->fillRect(mapRect,params.mapColorBack);
//     drawBackgroundImage(painter,rect,CachedMap);
     painter->drawPixmap(mapRect,*CachedMap);
// painter->drawImage(QPointF(rect.x(),rect.y()),*CachedMap,QRectF(rect.x(),rect.y(),
//                    qMin(rect.width(),qreal(CachedMap->width())),qMin(rect.height(),qreal(CachedMap->height()))));
    }
    else
    {
     if(CachedMap) delete CachedMap;
     CachedMap = new QPixmap(mapWidth+200,mapHeight+200);
     //CachedMap = new QImage(mapWidth,mapHeight,QImage::Format_RGB555);
     //CachedMap->fill(params.mapColorBack);
     QString cacheImgPath = params.mapCachePath+".cache";
     QFileInfo cacheInfo(cacheImgPath);
     if(QFile::exists(cacheImgPath) && nodeByID.count())
     {
      if(cacheInfo.size()>1024)
      {
       QImage img(mapWidth,mapHeight,QImage::Format_Indexed8);
       QImageReader imgReader(cacheImgPath,"PNG");
//       if(imgReader.canRead())
//            QPixmap::fromImageReader(&imgReader);

       //CachedMap->load(cacheImgPath,"PNG");
       if(imgReader.read(&img))
       {
        state.mapCached=1;
        painter->fillRect(mapRect,params.mapColorBack);
//        drawBackgroundImage(painter,rect,CachedMap);
        CachedMap->convertFromImage(img);
        //painter->drawImage(QRect(0,0,mapWidth,mapHeight),img);
        painter->drawPixmap(mapRect,*CachedMap);
        return;
       }
      }else
       QFile::remove(cacheImgPath);
     }
     //QImage img(mapWidth,mapHeight,QImage::Format_RGB16);
     QPainter p(CachedMap);
     //p.scale(params.mapZoomFactor*1.2,params.mapZoomFactor*1.2);
     p.translate(100,100);
     p.setRenderHints(Antialiasing);
     p.fillRect(mapRect,QBrush(params.mapColorBack));
     if(config.enableDrawBackground) drawPaintCmd(&p,paintBkg);
     if(nodeByID.count())
     {
      bool sv=0;
      int tmpBegin=nodeBeginID, tmpEnd=nodeEndID;
      drawPaintCmd(&p,paintMap);
      if(mapFade) sv=mapFade->isVisible();
      if(sv) mapFade->setVisible(0);
      nodeBeginID=nodeEndID=-1;
      //foreach(TMapNode *node,nodeByID) node->update();
      scene->render(&p,mapRect,mapRect);
      p.end();
      nodeBeginID=tmpBegin; nodeEndID=tmpEnd;
      if(sv) mapFade->setVisible(1);
     }
     state.mapCached=1;
     painter->fillRect(mapRect,params.mapColorBack);
//     drawBackgroundImage(painter,rect,CachedMap);
     painter->drawPixmap(mapRect,*CachedMap);
    }
   }else
   {
    if(!mapWidth || !mapHeight) drawPaintCmd(painter,paintBkg);
    painter->fillRect(rect,params.mapColorBack);
    if(config.enableDrawBackground) drawPaintCmd(painter,paintBkg);
    drawPaintCmd(painter,paintMap);
   }

  //qDebug() << testTimer.elapsed();
 }
// else
// {
//  int size=viewport()->rect().width()/2;
//  QString text;//=" "+params.mapCityDisplay;
//  QFont font("Tahoma");
//  font.setPixelSize(size);
//  QFontMetrics fm(font);
//  int length=fm.width(text);
//  QPointF point=viewport()->rect().center()-QPoint(length/2-size,-size/2);
//  painter->setFont(font);
//  painter->drawPixmap(point-QPoint(size,size),window()->windowIcon().pixmap(size,size));
//  painter->drawText(point,text);
// }
}
//void TMapView::drawForeground(QPainter *painter, const QRectF &rect)
//{
//}

void TMapView::scaleMap(qreal scaleFactor)
{
 if(!state.mapLoaded) return;
 mapStartDragging();
 params.mapZoomFactor = matrix().scale(scaleFactor, scaleFactor).m11(); //mapRect(QRectF(0, 0, 1, 1)).width();
 if (params.mapZoomFactor < qreal(0.3) || params.mapZoomFactor > qreal(15)) return;
 scale(scaleFactor, scaleFactor);
}

void TMapView::zoomIn()
{
 setTransformationAnchor(AnchorViewCenter);
 scaleMap(qreal(1.2));
 setTransformationAnchor(AnchorUnderMouse);
}
void TMapView::zoomOut()
{
 setTransformationAnchor(AnchorViewCenter);
 scaleMap(qreal(0.8));
 setTransformationAnchor(AnchorUnderMouse);
}

void TMapView::fitMap()
{
 if(!state.mapLoaded) return;
 QRectF RectView=QRectF(0,0,mapWidth,mapHeight); // scene->sceneRect() without frame
 mapStartDragging();
 if(config.MobileVersion)
 {
  fitInView(RectView,Qt::KeepAspectRatioByExpanding);
  verticalScrollBar()->setValue(0);
 }
 else
  fitInView(RectView,Qt::KeepAspectRatio);

 params.mapZoomFactor=matrix().m11();
}

void TMapView::loadMap(QString fileName)
{
 if(fileName.right(4).toLower()!=".pmz") return;
 QString imageFile,fname = fileName.section('/', -1 );

 if(!QFile::exists(fileName))
 {
  QString tempFileName=params.appPath+"map/"+fname;
  if(QFile::exists(tempFileName))
       fileName=tempFileName;
  else return;
 }

 // clear
 clearMap();
 //


 fname=fname.left(fname.indexOf("."));
 params.mapFile=fileName;
 params.mapCachePath=params.appCachePath+fname+"/";
 if(!extractFiles(fileName,fname+".cty;Metro.map;*.txt",params.mapCachePath)) return;


 INISettings cty(params.mapCachePath+fname+".cty",codePage);
 //cty.setIniCodec(codePage);
 cty.beginGroup("Options");
 params.mapName=cty.value("Name");
 params.mapCityName=cty.value("CityName");
 params.mapRusName=cty.value("RusName");
 QString Letter=params.mapCityName.mid(0,1);//codePage->fromUnicode(params.mapCityName.mid(0,1));
 if(BGN[ Letter[0] ]!="")
  config.enableTranslit=(config.locale!=ru);


 // Set window title
 QString CityName=params.mapCityName;
 if(config.locale==ru && !params.mapRusName.isEmpty()) CityName=params.mapRusName;
 else
 {
  Letter=CityName.mid(0,1);//codePage->fromUnicode(CityName.mid(0,1));
  if(BGN[Letter[0]]!="") CityName=params.mapName;
  Letter=CityName.mid(0,1);//codePage->fromUnicode(CityName.mid(0,1));
  if(BGN[Letter[0]]!="") convertToTranslit(CityName);
 }
 //(params.countryName.isEmpty() ? "":" - ")+params.countryName+
 if(WindowTitle.isEmpty()) WindowTitle=window()->windowTitle();
 window()->setWindowTitle(CityName+(CityName.isEmpty() ? "":" - ")+WindowTitle);
 qApp->processEvents();
 params.mapCityDisplay=CityName;

 params.mapCountryName=cty.value("Country");
 params.mapNeedVersion=cty.value("NeedVersion");
 params.mapAuthors=cty.value("MapAuthors");
 params.mapAuthors=convertToTextWithTags(params.mapAuthors);
 params.mapDelayNames=cty.value("DelayNames");

 QString day=tr("Day"),night=tr("Night"),rush=tr("Rush Hour"),none=tr("Ignore");
 if(params.mapDelayNames.isEmpty())
  params.mapDelayNames=QString("%1,%2,%3").arg(day).arg(night).arg(none);
 else params.mapDelayNames+=","+none;

 //qDebug() << params.mapDelayNames << params.mapDelayNames.split(",").count();
 QStringList delayList;
 if(config.enableTranslit)
 {
  convertToTranslit(params.mapDelayNames);
  delayList=params.mapDelayNames.split(",");
  for(int i=0;i<delayList.count();i++)
  {
   QString str=QString(delayList.at(i)).toLower();;
   convertToTranslit(str);
   if(str=="den'") delayList.replace(i,day);
   else if(str=="noch'") delayList.replace(i,night);
   else if(str=="chas-pik") delayList.replace(i,rush);
   else if(str=="ne uchityvat'") delayList.replace(i,none);
  }
 }
 else delayList=params.mapDelayNames.split(",");
 //qDebug() << delayList;
 emit delayNames(delayList);

 INISettings ini(params.mapCachePath+"Metro.map",codePage);

 ini.beginGroup("Options");
 imageFile=ini.value("ImageFileName");
 params.nodeWidth=ini.value("StationDiameter","16").toInt()/2;
 params.nodeConstructWidth=params.nodeWidth*qreal(0.7);
 params.routeWidth=ini.value("LinesWidth","9").toInt();
 if(params.routeWidth/params.nodeWidth>qreal(1.8)) // some freak map :)
  params.routeWidth=params.nodeWidth;
 params.nodeUpperCase=ini.value("UpperCase","1",vtBoolean).toInt();
 params.nodeWordWrap=ini.value("WordWrap","1",vtBoolean).toInt();
 params.mapIsVector=ini.value("IsVector","1",vtBoolean).toInt();
 params.Transports=ini.value("Transports","Metro.trp").split(',').join(";");
 params.CheckedTransports=ini.value("CheckedTransports","Metro.trp");
 if(imageFile.isEmpty() || !extractFiles(fileName,imageFile,params.mapCachePath)) return;

 if(!config.enableCustomFont)
   params.nodeFontStyle=(params.nodeWordWrap ? Qt::TextWordWrap:Qt::TextSingleLine)+Qt::TextDontClip;
 if(params.nodeUpperCase) params.nodeFont.setCapitalization(QFont::AllUppercase);
 params.nodeFont.setStyleHint(QFont::Serif);
 params.nodeFont.setWeight(QFont::Bold);
 params.nodeFont.setFamily("Arial");
 params.nodeFont.setPixelSize(11); // 8pt
 params.nodeFontDefault=params.nodeFont;

 // load data

 scene->setItemIndexMethod(QGraphicsScene::NoIndex);
 loadMapImageToPaintCmd(imageFile,paintBkg);
 //qDebug() << params.Transports << params.CheckedTransports;
 extractFiles(fileName,params.Transports,params.mapCachePath);
 loadMapLayers(params.CheckedTransports);
 //qDebug() << ((TMapLayer*)layer.at(0))->route.count();
 addLayerToPaintCmd(layer.at(0),paintMap);
 scene->setItemIndexMethod(QGraphicsScene::BspTreeIndex);

 initNodeInfo();

 // fade screen
 QRectF rFade=sceneRect();
 rFade.setWidth(rFade.width()+200);
 rFade.setHeight(rFade.height()+200);
 mapFade = new TMapElement(elFade,rFade);
 mapFade->FadeOpacity=config.valueTransparency/qreal(100);
 mapFade->setPos(QPoint(-100,-100));
 mapFade->setVisible(0);
 scene->addItem(mapFade);

 config.mapLastFile="";
 state.mapLoaded=1;
}

void TMapView::exportMap(QString fileName,QString ext)
{
 if(!mapWidth || !mapHeight)
 {
  mapWidth=sceneRect().width()-100;
  mapHeight=sceneRect().height()-100;
 }
 QRectF rect(0,0,mapWidth, mapHeight);
 QImage image(mapWidth, mapHeight, QImage::Format_RGB16);
 image.fill(0);
 QPainter painter(&image);
 painter.setRenderHints(Antialiasing);
 drawBackground(&painter,scene->sceneRect());
 scene->render(&painter,rect,rect);

 writeImage(fileName,ext,image);
}


bool TMapView::extractFiles(QString zipFile, QString extractFiles, QString extractDir)
{
 UnZip::ErrorCode ec;
 UnZip uz;
 //if (!pwd.isEmpty()) uz.setPassword(pwd);
 ec = uz.openArchive(zipFile);
 if (ec != UnZip::Ok)
 {
  QMessageBox::warning(0,QString(APPNAME),tr("Cannot open file mapping:\n%1").arg(zipFile));
  return 0;
 }
 //extractDir=QDir::convertSeparators(extractDir);
 QDir dir;
 dir.mkpath(extractDir);

 //all
 if(extractFiles.isEmpty()) uz.extractAll(extractDir); else
 {
  //separated mask: file.*;dir/file??.chk
  QStringList eList = extractFiles.split(";",QString::SkipEmptyParts);

  QList<UnZip::ZipEntry> zList = uz.entryList();
  foreach(const UnZip::ZipEntry& entry,zList)
   foreach(const QString &file, eList)
    if(entry.filename.contains(QRegExp(file,Qt::CaseInsensitive,QRegExp::Wildcard)))
    {
     if(!QFile::exists(extractDir+entry.filename))
      uz.extractFile(entry.filename,extractDir);
     break;
    }
 }

 uz.closeArchive();
 return 1;
}

void TMapView::loadMapImageToPaintCmd(QString imageFile, QList<TPaintCmd> &paintCmd)
{
 TPaintCmd cmd;
 QString eParam;
 DrawType eType;
 if(imageFile.right(4).toUpper()!=".VEC")
 {
  // (ext==".BMP" || ext==".GIF" || ext==".PNG")
  addPaintCmd(imageFile,dtImage,paintCmd);
  return;
 }

 addPaintCmd("-1",dtBrushColor,paintBkg);
 imageFile=params.mapCachePath+imageFile;
 QFile file(imageFile);
 if (!file.open(QFile::ReadOnly | QFile::Text))
 {
  //QMessageBox::warning(qApp, "Application",
  //                     tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
  return;
 }
 QTextStream in(&file);
 in.setCodec(codePage);
 QString line,type;
 //int i=0;
 while(!in.atEnd())
 {
  line=in.readLine();
  if(line[0]==';' || line.trimmed().isEmpty() || line.size()<4) continue;
  type  = line.mid(0,line.indexOf(" ")).toUpper();
  eParam = line.mid(line.indexOf(" ")+1);

  if(type=="SIZE" && !mapWidth && !mapHeight)
  {
   if(!mapWidth && !mapHeight)
   {
    mapWidth=eParam.mid(0,eParam.indexOf("x")).toInt();
    mapHeight=eParam.mid(eParam.indexOf("x")+1).toInt();
    scene->setSceneRect(QRect(-100,-100,mapWidth+200,mapHeight+200));
    //qDebug() << "scene: " + QString::number(mapWidth) +"x"+ QString::number(mapHeight);
   }
   addPaintCmd("-1",dtPenColor,  paintCmd);
   addPaintCmd("-1",dtBrushColor,paintCmd);
   continue;
  } else
   if(type=="PENCOLOR")    eType=dtPenColor; else
    if(type=="BRUSHCOLOR")  eType=dtBrushColor; else
     if(type=="POLYGON")     eType=dtPolygon; else
      if(type=="LINE")        eType=dtLine; else
       if(type=="SPLINE")      eType=dtSpline; else
        if(type=="TEXTOUT")     eType=dtTextOut; else
         if(type=="ANGLETEXTOUT")eType=dtAngleTextOut; else
          if(type=="ELLIPSE")     eType=dtEllipse; else
           if(type=="IMAGE")       eType=dtImage; else
            if(type=="ARROW")       eType=dtArrow; else
             if(type=="STAIRS")      eType=dtStairs; else
              if(type=="RAILWAY")     eType=dtRailway; else
               if(type=="SPOTCIRCLE")  eType=dtSpotCircle; else
                if(type=="SPOTRECT")  eType=dtSpotRect; else
                 if(type=="DASHED")  eType=dtDashed; else
                  if(type=="OPAQUE")eType=dtOpaque;
  else continue;

  addPaintCmd(eParam,eType,paintCmd);

 }
 file.close();
}

void TMapView::addPaintCmd(const QString &eParam,DrawType eType,QList<TPaintCmd> &paintCmd)
{
 TPaintCmd cmd;
 cmd.param=eParam;
 cmd.type=eType;
 cmd.image=NULL;
 switch(eType){
 case dtImage:{
   int x=0,y=0;
   QStringList param = eParam.split(",");
   QString imageFile,ext=param.at(0).right(4).toUpper();
   imageFile=params.mapCachePath+param.at(0);
   if(!QFile::exists(imageFile)) extractFiles(params.mapFile,param.at(0),params.mapCachePath);
   if(param.count()>1)
   {
    x=param.at(1).toInt();
    y=param.at(2).toInt();
    cmd.type=dtTranslate;
    cmd.nums << x << y;
    paintCmd << cmd;
   }
   if(ext==".VEC")
   {
    loadMapImageToPaintCmd(param.at(0),paintCmd);
   }else
   if(ext==".BMP" || ext==".GIF" || ext==".PNG" || ext==".JPG") // some env don't support jpg
   {
    cmd.image = new QPixmap(imageFile);
    cmd.type=dtImage;
    paintCmd << cmd;
   }
   if(param.count()>1)
   {
    cmd.type=dtTranslate;
    cmd.nums.clear();
    cmd.nums << -x << -y;
    paintCmd << cmd;
   }
  }break;
 case dtPenColor:
 case dtBrushColor:{
   QColor color;
   bool b = (eParam.count()>2);
   if(b) color.setNamedColor("#"+eParam);
   //cmd.colors << color;
   if(eType==dtPenColor)
   {
    if(b) cmd.pen=QPen(color,1,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin);
    else  cmd.pen=QPen(Qt::NoPen);
   }
   else
   {
    if(b) cmd.brush=QBrush(color,Qt::SolidPattern);
    else  cmd.brush=QBrush(Qt::NoBrush);
   }
   paintCmd << cmd;
  }break;

 case dtOpaque:{cmd.nums << eParam.toInt()/100.0;  paintCmd << cmd;} break;

 case dtDashed:
 case dtEllipse:
 case dtStairs:
 case dtArrow:
 case dtSpline:
 case dtLine:
 case dtRailway:
 case dtPolygon:{
   QList<qreal> nums;
   nums.clear();
   foreach (QString str, eParam.split(",")) nums << str.toFloat();
   if(eType==dtRailway)
   {
    cmd.nums << nums.at(0) << nums.at(1) << nums.at(2);;
    nums.removeFirst();
    nums.removeFirst();
    nums.removeFirst();
   }
   for(int i=0;i<nums.count();i=i+2)
   {
    if(i+1==nums.count()) break;
    cmd.polygon << QPointF(nums.at(i),nums.at(i+1));
   }

   // PEN
   cmd.pen.setWidth(1);
   cmd.pen.setStyle(Qt::SolidLine);
   if(nums.count()%2==1)
    cmd.pen.setWidthF(nums.at(nums.count()-1));
   if(eType==dtDashed) cmd.pen.setStyle(Qt::DashLine);

   // TYPE
   if(eType==dtSpline || eType==dtRailway)
   {
    if(eType==dtRailway)
    {
     QPointF p1=cmd.polygon.at(0),p2=cmd.polygon.at(1);
     int Offset=cmd.nums.at(0)/2;
     if(p1.y()>p2.y()) // WARNING: ANYKEY-METHOD! ;)
     {
      if(p1.x()>p2.x()) cmd.polygon.translate(-Offset,Offset);
      else cmd.polygon.translate(Offset,Offset); // 3
     }
     else
     {
      if(p1.x()>p2.x()) cmd.polygon.translate(Offset,-Offset);//2
      else cmd.polygon.translate(-Offset,-Offset); //1
     }
     qreal PenWidth=cmd.nums.at(1);
     QVector<qreal> dashPattern;
     cmd.nums.replace(0,cmd.nums.at(0)/(cmd.nums.at(0)+cmd.nums.at(1)+cmd.nums.at(2))*PenWidth*2);
     dashPattern << 1/cmd.nums.at(1) << 1/cmd.nums.at(2);
     cmd.pen.setWidthF(PenWidth);
     cmd.pen.setCapStyle(Qt::FlatCap);
     cmd.pen.setStyle(Qt::CustomDashLine);
     cmd.pen.setDashPattern(dashPattern);
    }
    cmd.path=splinePath(cmd.polygon);
    cmd.polygon.clear();
   }
   if(eType==dtArrow)
   {
    int count=cmd.polygon.count();
    if(count>2)
    {
     cmd.path.moveTo(cmd.polygon.at(0));
     for(int i=1;i<count-1;i++) cmd.path.lineTo(cmd.polygon.at(i));
    }
    cmd.path.addPath(arrowPath(cmd.polygon.at(count-2).toPoint(),cmd.polygon.at(count-1).toPoint()));
    cmd.polygon.clear();
   }
   if(eType==dtStairs)
   {
    cmd.path=stairsPath(cmd.polygon);
    cmd.polygon.clear();
   }

   nums.clear();
   paintCmd << cmd;
  }break;
 case dtAngleTextOut:
 case dtTextOut:{
   QString str,angle="";
   str=eParam;
   if(eType==dtAngleTextOut)
   {
    int pos=str.indexOf(",");
    angle=str.mid(0,pos);
    str.remove(0,pos+1);
   }
   QStringList param = str.trimmed().split(",");

   QFont font;
   qreal x,y;

   font.setStyleHint(QFont::Serif);
   font.setStyle(QFont::StyleNormal);
   font.setFamily(param.at(0));
   font.setPixelSize(param.at(1).toInt()*qreal(0.9));
   if(param.size()==6) // style
   {
    switch((int)param.at(5).toInt())
    {
    case 1: font.setBold(1); break;
    case 5:{font.setBold(1); font.setUnderline(1);}
    }
   }
   font.setFixedPitch(1);
   font.setLetterSpacing(QFont::AbsoluteSpacing,qreal(-0.2));
   font.setWordSpacing(qreal(-0.5));

   x=param.at(2).toFloat();
   y=param.at(3).toFloat();
   QString text=((QString)param.at(4)).trimmed();
   if(config.enableTranslit) convertToTranslit(text);
   cmd.path.addText(x, y+font.pixelSize()-2,font,text);

   if(!angle.isEmpty())
   {
    QMatrix matrix;
    matrix.translate(x,y);
    matrix.rotate(-angle.toFloat());
    matrix.translate(-x,-y);
    cmd.path=matrix.map(cmd.path);
   }


   paintCmd << cmd;
  }break;
 case dtSpotCircle:{ // need transfer to element
   QStringList param = eParam.trimmed().split(",");
   qreal radius=((QString)param.at(0)).toFloat();
   QPointF t,b,point=QPointF(((QString)param.at(1)).toFloat(),((QString)param.at(2)).toFloat());
   t=b=point;
   t-=QPointF(radius,radius);
   b+=QPointF(radius,radius);
   cmd.polygon << t << b;
   paintCmd << cmd;
  }break;
 case dtSpotRect:{ // need transfer to element
  QStringList param = eParam.trimmed().split(",");
  cmd.polygon << QPointF(((QString)param.at(0)).toFloat(),((QString)param.at(1)).toFloat())
              << QPointF(((QString)param.at(2)).toFloat(),((QString)param.at(3)).toFloat());
  paintCmd << cmd;
 }break;
 default: return; break;
 }
}

// DRAW
void TMapView::drawPaintCmd(QPainter *p,QList<TPaintCmd> &paintCmd)
{
 p->setPen(Qt::NoPen);
 p->setBrush(Qt::NoBrush);
 foreach(const TPaintCmd &cmd,paintCmd)
 {
  switch(cmd.type)
  {
  case dtPenColor: p->setPen(cmd.pen); break;
  case dtBrushColor: p->setBrush(cmd.brush); break;
  case dtOpaque: p->setOpacity(cmd.nums.at(0)); break;

  case dtPolygon:
  case dtSpline:
  case dtLine:
  case dtStairs:
  case dtArrow:
  case dtEllipse:
  case dtDashed:
  case dtRailway:
  {
    QPen tmp,pen;
    tmp=pen=p->pen();
    if(cmd.pen.widthF()>0)
    {
     pen.setStyle(cmd.pen.style());
     pen.setWidthF(cmd.pen.widthF());
     p->setPen(pen);
    }else p->setPen(QPen(Qt::NoPen));
    if(cmd.type==dtPolygon)
      p->drawPolygon(cmd.polygon);
    else
    if(cmd.type==dtLine || cmd.type==dtDashed)
      p->drawPolyline(cmd.polygon);
    else
    if(cmd.type==dtSpline || cmd.type==dtArrow || cmd.type==dtStairs || cmd.type==dtRailway)
    {
     QBrush brush = p->brush();
     p->setBrush(QBrush(Qt::NoBrush));
     if(cmd.type==dtRailway)
     {
      p->setPen(QPen(QBrush(tmp.color()),cmd.nums.at(0)*1.3,Qt::SolidLine,Qt::FlatCap));
      //p->drawPoint(cmd.polygon.at(0)+QPoint(2,2));
      //p->drawRect(QRect(QPointF(cmd.polygon.at(0)).toPoint(),QSize(5,5)));
      //qDebug() << cmd.nums.at(0);
      p->drawPath(cmd.path);
      p->setPen(QPen(QBrush(params.mapColorBack),cmd.nums.at(0)));
      p->drawPath(cmd.path);

      pen.setWidthF(cmd.pen.widthF());
      pen.setCapStyle(cmd.pen.capStyle());
      pen.setStyle(cmd.pen.style());
      pen.setDashPattern(cmd.pen.dashPattern());
      p->setPen(pen);
      p->drawPath(cmd.path);
      //tmp.setWidth(1);
     }else
      p->drawPath(cmd.path);
     p->setBrush(brush);
    }
    else
    if(cmd.type==dtEllipse)
      p->drawEllipse(QRectF(cmd.polygon.at(0),cmd.polygon.at(1)));

    //qDebug() << tmp.widthF() << pen.widthF();
    p->setPen(tmp);
   } break;

  case dtPath: p->drawPath(cmd.path); break;
  case dtAngleTextOut:
  case dtTextOut:{
    QPen pen=p->pen();
    QBrush brush=p->brush();
    p->setPen(Qt::NoPen);
    p->setBrush(pen.color());
    p->drawPath(cmd.path);
    p->setPen(pen);
    p->setBrush(brush);
   } break;
  case dtTranslate:
    p->translate(cmd.nums.at(0),cmd.nums.at(1));
     break;
  case dtImage:{
    if(!mapWidth && !mapHeight)
    {
     mapWidth=cmd.image->width();
     mapHeight=cmd.image->height();
     scene->setSceneRect(QRect(-100,-100,mapWidth+200,mapHeight+200));
    }
    p->drawPixmap(0,0,*cmd.image);
   } break;
   case dtSpotCircle:{ //## need transfer to element
    QPen pen=p->pen();
    p->setPen(Qt::NoPen);
    p->drawEllipse(QRectF(cmd.polygon.at(0),cmd.polygon.at(1)));
    p->setPen(pen);
   } break;
   case dtSpotRect:{ //## need transfer to element
    QBrush brush=p->brush();
    QPen pen=p->pen();
    p->setBrush(QBrush(params.mapColorBack));
    p->setPen(Qt::NoPen);
    QSizeF size( ((QPointF)cmd.polygon.at(1)).x()+5,((QPointF)cmd.polygon.at(1)).y()+2);
    p->drawRect(QRectF(cmd.polygon.at(0),size));
    p->setBrush(brush);
    p->setPen(pen);
   }break;
   default: break;
  }
 }
}

void TMapView::clearMap()
{
// qDebug() << "clear map";
 state.mapLoaded=0;
 state.mapCached=0;

 for(int i=0;i<paintBkg.count();i++)
   if(paintBkg.at(i).image!=NULL) delete paintBkg.at(i).image;
 paintBkg.clear();
 paintMap.clear();

 for(int a=0;a<edgeCache.count();a++) delete edgeCache.at(a);
 edgeCache.clear();

 if(CachedMap) delete CachedMap;
 CachedMap = new QPixmap(0,0);
 //CachedMap = new QImage(0,0,QImage::Format_Mono);

 mapDistance.clear();
 nodeBegin=NULL;
 nodeEnd=NULL;
 nodeByID.clear();
 clearTrack();

 scene->setSceneRect(QRect(0,0,0,0));
 scene->clear();
 mapWidth=0; mapHeight=0; mapRect=QRect();
 params.nodeFont=QFont();

 for(int i=0;i<mapGraph.count();i++)
  for(int j=0;j<mapGraph.count();j++)
  {

   if(mapGraph[i][j]==NULL) continue;
   if(mapGraph[i][j]==mapGraph[j][i]) mapGraph[j][i]=NULL;
   delete mapGraph[i][j];
   mapGraph[i][j]=NULL;
  }
 for(int i=0;i<mapGraph.count();i++){ mapGraph[i].clear(); }
 mapGraph.clear();

 for(int i=0;i<layer.count();i++)
 {
  for(int j=0;j<layer.at(i)->route.count();j++)
  {
   delete layer.at(i)->route.at(j);
  }
  layer.at(i)->route.clear();
  delete layer.at(i);
 }
 layer.clear();
}

void TMapView::loadMapLayers(QString LayerFilesTRP)
{
 QStringList list,tprlist=LayerFilesTRP.split(",",QString::SkipEmptyParts);
 int ind,nodeID=0;
 INISettings map(params.mapCachePath+"Metro.map",codePage);
 foreach(const QString &file, tprlist)
 {
  TMapLayer *mapLayer = new TMapLayer();
  INISettings trp(params.mapCachePath+file,codePage);
  mapLayer->typeName=trp.value("Options/Type");
  int line=0;

  // load routes
  QString LinesWidth=QString::number(params.routeWidth);
  while(1)
  {
   QString routeName;

   trp.beginGroup(QString("Line%1").arg(++line));
   routeName=trp.value("Name");
   //qDebug() << routeName;
   if(routeName.isEmpty()) { trp.endGroup(); break; }
   map.beginGroup(routeName);
   list=map.value("Coordinates").split(',');
   if(list.count()<2)
   {
    //qDebug() <<"----"<<routeName;
    trp.endGroup();
    map.endGroup();
    continue;
   }

   TMapRoute *route = new TMapRoute();
   route->Name=routeName;
   route->DisplayName=routeName;
   if(config.enableTranslit) convertToTranslit(route->DisplayName);
   //route->active=0;
   route->mapFile=trp.value("LineMap");

   QString str;
   QStringList cost;
   cost=trp.valueToStringList("Delays");
   if(!cost.count())
   {
    cost << trp.value("DelayDay","0");
    cost << trp.value("DelayNight","0");
    cost << "0";
   }else cost << "0";
   foreach(const QString &str,cost) route->delay<<strTimeToSec(str);
   //route->delay=cost;
   cost.clear();
   str=trp.value("Stations");

   // load nodes
   route->temp << str << trp.value("Driving");
   QStringList nodeAliases2=trp.valueToStringList("Aliases2");
   trp.endGroup();

   str.remove(QRegExp("\\([^\\)]*\\)"));
   QStringList nodeNames=trp.toStringList(str);
   ind=0;
   foreach(const QString &nodeName,nodeNames)
   {
    TMapNode *node = new TMapNode(this,route,nodeName,nodeID++);
    if(nodeAliases2.count()>ind) node->Alias=nodeAliases2.at(ind);
    //qDebug() << node->Name << node->Alias;
    route->node.append(node);
    nodeByID << node;
    scene->addItem(node);
    ind++;
   }
   nodeNames.clear();
   nodeAliases2.clear();

   route->ID=mapLayer->route.count()+1;
   mapLayer->route.append(route);



   // graphics data of map
   // load nodes: Coordinates, Rects, Heights

   for(int i=0;i<list.count();i=i+2) // Coordinates
   {
    ind=((i+2)/2)-1;
    route->node.at(ind)->setPos(QPoint(list.at(i).toInt(),list.at(i+1).toInt()));
   }
   list.clear();
   list=map.value("Rects").split(',');
   if(list.count()<=2)
   {
    list.clear();
    for(int i=0;i<route->node.count();i++)
     route->node.at(i)->Rect=QRect();
   }
   else
   for(int i=0;i<list.count();i=i+4)
    route->node.at(((i+4)/4)-1)->Rect=QRect(list.at(i).toInt(),list.at(i+1).toInt(),list.at(i+2).toInt(),list.at(i+3).toInt());
   list.clear();
   list=map.value("Heights").split(',');
   int maxi=route->node.count()-1;
   for(int i=0;i<list.count();i++)
   {
    if(i>maxi) break;
    route->node.at(i)->Height=list.at(i).toInt();
   }

   // load route styles
   list.clear();
   list=map.value("Rect").split(',');
   if(list.count()<4) route->rect=QRect();
   else
    route->rect=QRect(list.at(0).toInt(),list.at(1).toInt(),list.at(2).toInt(),list.at(3).toInt());

   QString color=map.value("Color");
   if(color.count()>2)
        route->color.setNamedColor("#"+color);
   else route->color=QColor(0,0,0,0);
   color=map.value("LabelsColor");
   if(color.count()>2) route->labelsColor.setNamedColor("#"+color);
   else route->labelsColor=QColor(0,0,0,0);
   color=map.value("LabelsBColor","000000");
   if(color.count()>2) route->labelsBColor.setNamedColor("#"+color);
   else route->labelsBColor=QColor(0,0,0,0);

   route->width=map.value("Width",LinesWidth).toFloat();

   // color & styles of nodes (some memory economy)
   route->ColorDark=route->color.darker();
   route->EndPen=QPen(Qt::red,params.nodeWidth/qreal(2));
   route->BeginPen=QPen(Qt::green,params.nodeWidth/qreal(2));
   route->TransferPen=QPen(Qt::white,0);
   route->TransferInfoPen=QPen(Qt::darkYellow,0);
   route->NormalPen=QPen(route->color.lighter(),1);
   route->NormalInfoPen=QPen(Qt::yellow,1);
   route->ConstructPen=QPen(route->color,2);
   route->ConstructInfoPen=QPen(Qt::darkYellow,2);



   map.endGroup();
   foreach(TMapNode *n,route->node) n->init();
   //route->active=1;
  }
  //qDebug() << "Routes loaded";

  // load transfers cache
  trp.beginGroup("Transfers");
  while(!trp.atEndKey())
  {
   QStringList param=trp.toStringList(trp.valueNext());
   AddCacheEdge *edge=new AddCacheEdge;
   edge->routeName[0]=param.at(0);
   edge->nodeName[0]=param.at(1);
   edge->routeName[1]=param.at(2);
   edge->nodeName[1]=param.at(3);
   edge->cost[0]=param.at(4);
   edge->cost[1]="-1";
   edge->edgeStyle=TMapEdge::esNormal;
   if(param.count()>5)
   {
    bool ok;
    QString(param.value(5)).left(1).toInt(&ok);
    if(ok) edge->cost[1]=param.at(5);
    else   edge->edgeStyle=TMapEdge::esInvisible; // invisible
    if(param.count()>6) edge->edgeStyle=TMapEdge::esInvisible;
   }
   edge->transfer=1;
   edge->assigned=0;
   if(edge->cost[0]=="0") edge->cost[0]="1";
   if(edge->cost[1]=="0") edge->cost[1]="1";
   edgeCache << edge;
  }
  trp.endGroup();

  // load additional nodes cache  
  map.beginGroup("AdditionalNodes");
  while(!map.atEndKey())
  {
   QStringList param=map.toStringList(map.valueNext());
   AddCacheEdge *edge=new AddCacheEdge;
   edge->routeName[0]=param.at(0);
   edge->nodeName[0]=param.at(1);
   edge->nodeName[1]=param.at(2);
   bool hasType=0;
   for(int i=3;i<param.count();i=i+2)
   {
    if(i+1==param.count()) { hasType=1; break;}
    edge->points << QPoint(param.at(i).toInt(),param.at(i+1).toInt());
   }

   edge->transfer=0;
   if(hasType /*&& param.at(last).toUpper()=="SPLINE"*/)
        edge->edgeStyle=TMapEdge::esSpline;
   else edge->edgeStyle=TMapEdge::esNormal;
   edge->assigned=0;
   edgeCache << edge;
  }

  map.endGroup();
  // load edges, additional nodes, transfers
  mapGraph.resize(nodeID);
  for(int i=0;i<nodeID;i++) mapGraph[i].resize(nodeID);

  foreach(TMapRoute *route,mapLayer->route)
  {
   //if(!route->active) continue;

   QStringList name,cost;
   name=map.toStringList(route->temp.at(0));
   cost=route->temp.at(1).split(',');
   route->temp.clear();
   //qDebug() << route->name << name.count() << cost.count();
   QString nodeName,node1,node2,eCost;
   bool sign=0;

   for(int i=0;i<=name.count()-cost.count();i++) cost.append("0");
   //qDebug() << name.count() << cost.count();
   for(int i=0;i<name.count();i++)
   {
    QString str=name.value(i);
    TMapEdge *edge;
    node1=""; node2=""; eCost="";
    if(!sign) // find "("
    {
     int pos=str.indexOf("(");
     if(pos!=-1) // abc(dfg
     {
      nodeName=str.left(pos);
      node1=nodeName; // abc
      node2=str.mid(pos+1,str.length()); // dfg
      eCost=cost.value(i);
      //eCost.remove("(");
      sign=1;
      if(node2.indexOf(")")!=-1)
      {
       node2=node2.remove(")");
       //eCost.remove(")");
       sign=0;
      }

     }
    }else // find ")"
    {
     int pos=str.indexOf(")");
     eCost=cost.value(i);
     if(pos!=-1)
     {
      if(eCost.indexOf(")")==-1) // when "()", but need "(,,)"
      {
       cost.insert(i,"");
       eCost="";
      } else eCost.remove(")");
      str.remove(")");
      sign=0;
     }
     node1=nodeName;
     node2=str;
     if(eCost.isEmpty()) eCost="0";
    }
    if(!sign && eCost.isEmpty() && i<name.count()-1 && name.value(i+1).indexOf("(")==-1)
    {
     node1=name.value(i);
     node2=name.value(i+1);
     eCost=cost.value(i);
    }
    if(node2.isEmpty()) continue;
    eCost.remove("(");
    eCost.remove(")");//

    node1=map.toStringList(node1).at(0); // quoter chop
    node2=map.toStringList(node2).at(0);
    //qDebug() << route->Name << node1+" - "+node2+" = "+eCost;
    //qDebug() << addEdge.count();

    edge = new TMapEdge(route,node1,node2,eCost);
    if(edge->node[0]==NULL || edge->node[1]==NULL)
    {
     delete edge;
     continue;
    }
    //qDebug() << node1 << node2 << eCost;
    // modif edges for addition nodes & transfers
    bool clearCache=1;
    for(int j=0;j<edgeCache.count();j++)
    {
     if(edgeCache.at(j)->assigned) continue;
     clearCache=0;
     if(edgeCache.at(j)->transfer) // transfers
     {
      QString n1,n2,c1,c2;

      TMapEdge *transfer;
      TMapRoute *r1=mapLayer->findRoute(edgeCache.at(j)->routeName[0]),
                *r2=mapLayer->findRoute(edgeCache.at(j)->routeName[1]);

      if(!r1 || !r2) {edgeCache.value(j)->assigned=1; continue;}
      n1=edgeCache.at(j)->nodeName[0];
      n2=edgeCache.at(j)->nodeName[1];
      c1=edgeCache.at(j)->cost[0];
      c2=edgeCache.at(j)->cost[1];
      TMapEdge::EdgeStyle style=(TMapEdge::EdgeStyle)edgeCache.at(j)->edgeStyle;
      //qDebug() << r1 << edgeCache.at(j)->routeName[0] << n1 << r2 << edgeCache.at(j)->routeName[1] << n2;
      for(int a=0;a<2;a++)
      {

       transfer = new TMapEdge(r1,n1,n2,c1,TMapEdge::etTransfer,style,r2);
       r1->edge << transfer;
       r2->edge << transfer;
       qSwap(r1,r2);
       qSwap(n1,n2);
       if(c2!="-1") c1=c2;
       style=TMapEdge::esInvisible;

      }
      edgeCache.value(j)->assigned=1;
     }
     else // addition nodes
     {
      if(edgeCache.at(j)->routeName[0]==route->Name && (
          (edgeCache.at(j)->nodeName[0]==node1 && edgeCache.at(j)->nodeName[1]==node2)
        ||(edgeCache.at(j)->nodeName[1]==node1 && edgeCache.at(j)->nodeName[0]==node2) ))
      {
       foreach(const QPoint &point,edgeCache.at(j)->points)  edge->addNode << point;
       edge->style=(TMapEdge::EdgeStyle)edgeCache.at(j)->edgeStyle;
       edgeCache.value(j)->assigned=1;
      }
     }

    }
    if(clearCache)
    {
     for(int a=0;a<edgeCache.count();a++) delete edgeCache.at(a);
     edgeCache.clear();
    }

    route->edge << edge;
   }

  }

  layer.append(mapLayer);
 }
}

void TMapView::addLayerToPaintCmd(TMapLayer *mapLayer,QList<TPaintCmd> &paintCmd)
{
 TPaintCmd tCmd;
 QPainterPath tPath;

 qreal tWidth=params.routeWidth;
// tCmd.brush=QBrush(Qt::SolidPattern);
// tCmd.type=dtBrushColor;
// paintCmd << tCmd;
 QVector<qreal> dashPattern;
 dashPattern << qreal(1.5) << qreal(0.5);

 foreach(TMapRoute *route,mapLayer->route)
 {
  //if(!route->active) continue;
  TPaintCmd cmd;
  QPainterPath pathRoute;
  QPen solid=QPen(route->color,route->width,Qt::SolidLine,Qt::SquareCap,Qt::RoundJoin);
  QPen dash=QPen(route->color,route->width*qreal(0.8),Qt::CustomDashLine,Qt::FlatCap,Qt::RoundJoin);
  dash.setDashPattern(dashPattern);
  cmd.pen=solid;
  cmd.type=dtPenColor;
  paintCmd << cmd;
  foreach(TMapEdge *edge,route->edge)
  {
   QPolygonF polygon;
   QPainterPath path;

   if(!edge->assigned || edge->style==TMapEdge::esInvisible) continue;
   edge->assigned=0;
   polygon << edge->node[0]->pos();
   foreach(const QPointF &point,edge->addNode) polygon << point;
   polygon << edge->node[1]->pos();
   //qDebug()<<polygon;
   if(polygon.count()>3) // because may be loop.
   {
    // LOH.KO-STIL :)
    QGraphicsLineItem *lineItem1 = new QGraphicsLineItem(QLineF(polygon.first(),polygon.at(1))),
                      *lineItem2 = new QGraphicsLineItem(QLineF(polygon.last(), polygon.at(polygon.count()-2)));
    //lineItem1->setPen(QPen(Qt::black,1));//params.LinesWidth/2));
    //lineItem2->setPen(QPen(Qt::black,1));//params.LinesWidth/2));

    QLineF line1(polygon.first(),polygon.at(1)),
           line2(polygon.first(), polygon.at(2));

    QPolygonF swap=polygon;
    QPointF p=swap.first();
    swap.replace(0,swap.last());
    swap.replace(swap.count()-1,p);

    if(lineItem1->collidesWithItem(lineItem2)) // cross
     polygon=swap;
    else
    if(line1.length()>line2.length()) // zig-zag
    {
     lineItem1->setLine(QLineF(swap.first(),swap.at(1)));
     lineItem2->setLine(QLineF(swap.last(), swap.at(swap.count()-2)));
     if(!lineItem1->collidesWithItem(lineItem2))
      polygon=swap;
    }

    swap.clear();
    delete lineItem1;
    delete lineItem2;
   }

   if(edge->style==TMapEdge::esSpline)
        path.addPath(splinePath(polygon));
   else path.addPolygon(polygon);

   if(edge->type==TMapEdge::etTransfer)
   {
    if(!config.enableSimpleTransfer)
    for(int i=0;i<2;i++)
    {
     qreal w=tWidth/qreal(1.7);//tWidth*1.1;
     tPath.addEllipse(edge->node[i]->pos(),w,w);
    }
    tPath.moveTo(polygon.first());
    tPath.lineTo(polygon.last());
    edge->path=path;
    mapGraph[edge->node[1]->ID][edge->node[0]->ID]->path=path;
    continue;
   }else  edge->path=path;

   if(edge->type==TMapEdge::etConstruct)
   {
    TPaintCmd aCmd;

    aCmd.pen=dash;
    aCmd.type=dtPenColor;
    paintCmd << aCmd;

    aCmd.path.addPath(path);
    aCmd.type=dtSpline;
    paintCmd << aCmd;
    aCmd.path=QPainterPath();

    aCmd.pen=solid;
    aCmd.type=dtPenColor;
    paintCmd << aCmd;

    continue;
   }
   pathRoute.addPath(path);
   edge->addNode.clear();
  }
  cmd.path.addPath(pathRoute);
  //cmd.path.simplified();
  cmd.type=dtSpline;
  paintCmd << cmd;
  pathRoute=QPainterPath();
 }

  tCmd.pen=QPen(Qt::black,tWidth*qreal(1.4),Qt::SolidLine,Qt::FlatCap,Qt::MiterJoin);
  tCmd.type=dtPenColor;
  paintCmd << tCmd;

//  QPainterPath tPath;
//  tPath.addPath(tPathE);
//  tPath.addPath(tPathL);

  tCmd.path.addPath(tPath);
  tCmd.type=dtPath;
  paintCmd << tCmd;
  tCmd = TPaintCmd();

  tCmd = TPaintCmd();

  tCmd.pen=QPen(params.mapColorBack.lighter(150),tWidth*qreal(1.2),Qt::SolidLine,Qt::FlatCap,Qt::MiterJoin);
  tCmd.type=dtPenColor;
  paintCmd << tCmd;

  tCmd.path.addPath(tPath);
  tCmd.type=dtPath;
  paintCmd << tCmd;
  tCmd = TPaintCmd();
}


void TMapView::mouseMoveEvent(QMouseEvent *event)
{
 QGraphicsView::mouseMoveEvent(event);
 if(event->buttons()==Qt::LeftButton)
 {
  mapStartDragging();
//  if(config.enableLowqDrag && config.enableAntialiasing)
//  {
//   //params.nodeFont.setStyleStrategy(QFont::NoAntialias);
//   //params.nodeFont.setStyleStrategy(QFont::PreferDefault);
//   setRenderHints(NonAntialiasing);
//  }
//  state.mapDragging=1;
 }
 //TimerPopup.stop();
 if(TimerPopup.isActive() && nodeClick && QLineF(nodeClickPos,event->pos()).length()>params.nodeWidth*5) TimerPopup.stop();
}

//void TMapView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
//{
// qDebug() << 1;
// //mouseDoubleClickEvent(event);
// QGraphicsItem *item=itemAt(event->pos().toPoint());
// if(item==NULL || item->type()!=elNode) return;
// Details *detailsWindow = new Details(this);
// detailsWindow->setWindowTitle(detailsWindow->windowTitle()+" "+((TMapNode*)item)->Name);
// detailsWindow->show();
//}

void TMapView::mousePressEvent (QMouseEvent *event)
{
 QGraphicsView::mousePressEvent(event);

 nodeClickPos=event->pos();
 if(event->button()==Qt::MidButton)
 {
  mapStartDragging();
  fitInView(QRectF(0,0,mapWidth,mapHeight),Qt::KeepAspectRatioByExpanding); //fitMap();
 }
 if(event->button()==Qt::LeftButton || event->button()==Qt::RightButton)
 {
  bool sv=0;
  if(mapFade) sv=mapFade->isVisible();
  if(sv) mapFade->setVisible(0);
  nodeClick=(TMapNode*)itemAt(nodeClickPos);
  if(sv) mapFade->setVisible(1);
 }
 if(event->button()==Qt::LeftButton)
 {
  if(nodeClick && nodeClick->type()==elNode) TimerPopup.start();
 }
}

void TMapView::popupByTimer()
{
 if(!nodeClick || !nodePopup) return;
 setDragMode(QGraphicsView::NoDrag);

 if(config.MobileVersion)
  nodePopup->popup(QPoint(rect().width()/2-nodePopup->width()/2,rect().height()-nodePopup->height())+mapToGlobal(rect().topLeft()));
 else
  nodePopup->popup(mapToGlobal(nodeClickPos));
 if(config.enableLowqDrag && config.enableAntialiasing)
  setRenderHints(Antialiasing);
 setDragMode(QGraphicsView::ScrollHandDrag);
}

void TMapView::mouseReleaseEvent(QMouseEvent *event)
{
 QGraphicsView::mouseReleaseEvent(event);

// if(event->button()==Qt::LeftButton)
// {

//  if(state.mapDragging)
//  {
//   if(config.enableLowqDrag)
//   {
//    if(config.enableAntialiasing)
//    {
//     setRenderHints(Antialiasing);
//     //params.nodeFont.setStyleStrategy(QFont::PreferAntialias);
//    }
////    else
////     params.nodeFont.setStyleStrategy(QFont::PreferDefault);
//   }
//   state.mapDragging=0;
//  }
// }
//qDebug() << QLineF(nodeClickPos,event->pos()).length()<<params.nodeWidth*5;
 if(QLineF(nodeClickPos,event->pos()).length()>params.nodeWidth*5)
 {
  nodeClick=NULL;
  return;
 }

 TimerPopup.stop();

 if((event->button()==Qt::LeftButton || event->button()==Qt::RightButton)
    && (!nodeClick || nodeClick->type()!=elNode))
 {
  if(!config.MobileVersion)
  {
   clearTrack();
   if(nodeClick) nodeClick->update();
  }
  return;
 }

 if(event->button()==Qt::RightButton)
 {
  if(nodePopup) nodePopup->popup(mapToGlobal(event->pos()));
 }

 if(event->button()==Qt::LeftButton)
 {
  if(nodeClick->TypeNode==TMapNode::ntConstruct) return;

 //testTimer.start();

  if(config.MobileVersion && nodeBegin && nodeEnd) popupByTimer();
  else
  findTrack(nodeClick);
 //qDebug() << testTimer.elapsed();
 }

}

void TMapView::selectedNodesToFront(bool enable)
{
 if(!selectedNode.count()) return;
 if(!enable && !mapFade->isVisible()) return;

 QRectF rect=selectedNode.at(0)->mapToScene(selectedNode.at(0)->boundingRect()).boundingRect();
 //qDebug() << rect;
 int z=(enable ? 2:-1),nPadding=params.nodeWidth*2;
 foreach(TMapNode *node, selectedNode)
 {
  if(enable)
  {
   QRectF nRect=node->mapToScene(node->boundingRect()).boundingRect();
   if(nRect.left()<rect.left())rect.setLeft(nRect.left());
   if(nRect.top()<rect.top())  rect.setTop(nRect.top());
   if(nRect.right()>rect.right())  rect.setRight(nRect.right());
   if(nRect.bottom()>rect.bottom())rect.setBottom(nRect.bottom());
  }
  node->setZValue(z);
 }

 if(!enable) selectedNode.clear();
 mapFade->setVisible(enable);
 if(enable)
 {
  int wh=qMin(rect.width(),rect.height());
  nPadding*=(wh/nPadding<params.nodeWidth/2 ? params.nodeWidth:params.nodeWidth/3);
  rect.adjust(-nPadding,-nPadding,nPadding,nPadding);
  fitInView(rect,Qt::KeepAspectRatio);
  if(config.MobileVersion && selectedNode.count()==1)
    verticalScrollBar()->setValue(verticalScrollBar()->value()+verticalScrollBar()->maximum()/12);

  params.mapZoomFactor=matrix().m11();
 }
}

QString TMapView::getMessageTrackInfo(int index,bool inLine)
{
 if(!mapTrack.count()) return "";

 QString info,br="<br>";
 if(inLine)
 {
  info="%1 %2 %3 %4 %5 %6";
 }
 else
 if(config.MobileVersion)
 {
  QString sp="&nbsp;";
  info=sp+"%1 %2 %3"+sp+br+
       sp+"%4 %5 %6"+sp;
 }
 else
 {
  info="<center><b>%1"+br+
       "%2 %3</b>"+br+
       "%4 %5 %6</center>"+br;
 }

 info=info.arg(inLine ? "":tr("Travel time")) // 1
      .arg(QString::number(int(mapTrack.at(index).countSecond/60))) // 2
      .arg(tr("min."))
      .arg((mapTrack.at(index).countTransfer ? QString::number(mapTrack.at(index).countTransfer)+" "+tr("transf."):""))
      .arg(QString::number(mapTrack.at(index).countStation))
      .arg(tr("st."));

 return info;
}

QString TMapView::getMessageTrackDescription(int index)
{
 if(!mapTrack.count()) return "";

 QString tf="HH:mm";
 QString br="<br>";
 QString line=" "+tr("line")+")";
 QString font="<table width=\"100%\" style=\"COLOR:white;BACKGROUND-COLOR:%1\"><tr><td align=\"center\"><b>&nbsp;%2</b><td width=0 align=\"right\">%3</tr></table>";
 QString smallTag="<small>%1</small>";
 QString centerTag="<center>%1</center>";

 QString text=smallTag.arg("%4")+
      font.arg("%5").arg("%6").arg(smallTag.arg("%8"))+
      centerTag.arg(smallTag.arg("%7"))+"<hr>";

 text=text.arg(centerTag.arg(tr("Begin station:"))) // 4
          .arg(nodeByID[mapTrack.at(index).station.first().id]->Route->color.name()) //5
          .arg(nodeByID[mapTrack.at(index).station.first().id]->DisplayName) //6
          .arg("("+nodeByID[mapTrack.at(index).station.first().id]->Route->DisplayName+line) //7
          .arg(mapTrack.at(index).station.first().time.toString(tf)); // 8

 for(int i=1;i<mapTrack.at(index).station.count()-1;i++)
 {
  if(mapTrack.at(index).station.at(i).isTransfer)
  {
   text+=centerTag.arg(smallTag.arg(tr("Go to station:")))+
         font.arg(nodeByID[mapTrack.at(index).station.at(i).id]->Route->color.name())
         .arg(nodeByID[mapTrack.at(index).station.at(i).id]->DisplayName)
         .arg("<b>"+smallTag.arg(mapTrack.at(index).station.at(i).time.toString(tf))+"</b>")+
         centerTag.arg(smallTag.arg(tr("Transfer to station:")))+
         font.arg(nodeByID[mapTrack.at(index).station.at(i+1).id]->Route->color.name())
         .arg(nodeByID[mapTrack.at(index).station.at(i+1).id]->DisplayName)
         .arg(smallTag.arg(mapTrack.at(index).station.at(i+1).time.toString(tf)))+"<hr>";
  }

 }

 text+=centerTag.arg(smallTag.arg(tr("Final station:")))+
       font.arg(nodeByID[mapTrack.at(index).station.last().id]->Route->color.name())
           .arg(nodeByID[mapTrack.at(index).station.last().id]->DisplayName)
           .arg("<b>"+smallTag.arg(mapTrack.at(index).station.last().time.toString(tf))+"</b>")+
           "<center>"+smallTag.arg("("+nodeByID[mapTrack.at(index).station.last().id]->Route->DisplayName+line)+"</center>";

 return text;
}

void TMapView::initWorkGraph(QVector< QVector<qreal> > &workGraph)
{
 int size=mapGraph.count();
 workGraph.resize(size);
 for(int i=0;i<size;i++)
 {
  workGraph[i].resize(size);
  for(int j=0;j<size;j++)
  {
   if(mapGraph[i][j]) workGraph[i][j]=mapGraph[i][j]->cost;
   else               workGraph[i][j]=0;

   if(!config.MobileVersion && i==nodeBeginID && workGraph[i][j])
    workGraph[i][j]+=params.delayToMetro*60+mapGraph[i][j]->node[1]->Route->delay.at(params.delayIndex);

   if(i!=nodeBeginID && workGraph[i][j] && mapGraph[i][j]->type==TMapEdge::etTransfer)
    workGraph[i][j]+=mapGraph[i][j]->node[1]->Route->delay.at(params.delayIndex);

  }
 }
}

void TMapView::findTrackInGraph(TMapNode *begin,TMapNode *end, QVector< QVector<qreal> > workGraph)
{
 // calc distances
  dijkstra(nodeBeginID,workGraph);

 // find tracks
 QList<int> id;
 int endID=nodeEndID;
 id << endID;
 while (mapParentNode[endID]!= -1)
 {
  id << mapParentNode[endID];
  endID = mapParentNode[endID];
 }

 //qDebug() << id;
 int countTransfer=0;
 TMapTrack track;
 for(int i=id.count()-1;i>=0;i--)
 {
  track.selectedNode << nodeByID[id.at(i)];
  track.idTrack+=QString::number(id.at(i))+"/";
  track.addStantion(id.at(i)
//                    ,nodeByID[id.at(i)]->DisplayName
//                    ,nodeByID[id.at(i)]->Route->color
                    ,QTime::currentTime().addSecs(mapDistance[id.at(i)]));
 }
 track.idTrack.chop(1);

 for(int i=0,j=id.count()-2;i<id.count()-1 && j>=0;i++,j--) // asc & desc
 {
  track.selectedEdge << mapGraph[id.at(i)][id.at(i+1)];
  track.station.at(j).isTransfer=(mapGraph[id.at(i)][id.at(i+1)]->type==TMapEdge::etTransfer) && workGraph[id.at(i)][id.at(i+1)];
  track.station.at(j).isAlarm=track.station.at(j).isTransfer;
  if(track.station.at(j).isTransfer)
  {
   track.listAlarmTime << track.station.at(j).time;
   countTransfer++;
  }
 }
 track.station.last().isAlarm=1;
 track.setParams(mapDistance[nodeEndID],track.selectedNode.count()-countTransfer,countTransfer);

// track.selectedNode << selectedNode;
// track.selectedEdge << selectedEdge;
 track.mapDistance << mapDistance;

 if(mapTrack.count())
 {
  if(track.countSecond<mapTrack.at(0).countSecond*2)
  {
   bool isExist=0;
   foreach(TMapTrack t,mapTrack) if(t.idTrack==track.idTrack){ isExist=1; break; }
   if(track.countStation>1 && !isExist) mapTrack << track;
  }
 }
 else mapTrack << track;

// QString testnames; foreach(TMapTrack::TStation s,track.station) testnames+=s.name+" ";
// qDebug() << ">>> "+testnames+"\n"<< track.countStation << track.countTransfer << track.countSecond;
// qDebug() << mapTrack.count() << countTransfer;

 if(mapTrack.count()<9 && countTransfer && track.countSecond<mapTrack.at(0).countSecond*qreal(1.8)) // count of tracks
 {
  for(int i=0;i<track.station.count()-1;i++)
  {
   if(!workGraph.count()) break;
//   qDebug() << track.station.at(i).name << track.station.at(i).isTransfer;
   if(track.station.at(i).isTransfer && workGraph[track.station.at(i).id][track.station.at(i+1).id])
   {
    qreal a,b;
    a=workGraph[track.station.at(i).id][track.station.at(i+1).id];
    b=workGraph[track.station.at(i+1).id][track.station.at(i).id];
    workGraph[track.station.at(i).id][track.station.at(i+1).id]=0;
    workGraph[track.station.at(i+1).id][track.station.at(i).id]=0;
    findTrackInGraph(nodeBegin,nodeEnd,workGraph);
    workGraph[track.station.at(i).id][track.station.at(i+1).id]=a;
    workGraph[track.station.at(i+1).id][track.station.at(i).id]=b;
   }
  }

  return;
 }
 else workGraph.clear();

}

void TMapView::findTrack(TMapNode *begin,TMapNode *end)
{
 emit nodeClicked(1);

 if(!begin && end)
 {
  nodeEndID=end->ID;
  if(nodeEnd) nodeEnd->update();
  nodeEnd=end;
  end->update();
  return;
 }
 if(!begin || begin->TypeNode==TMapNode::ntConstruct ||
    begin==end /*|| (!end && begin==nodeBegin)*/ || (end && end->TypeNode==TMapNode::ntConstruct)) return;

 if(end==NULL) // click node: begin or end
 {
  if(nodeBegin==NULL) nodeBegin=begin;
  else
  if(begin!=nodeBegin)nodeEnd=begin;

  emit nodeHasInfo(nodeBegin && nodeBegin->HasInfo && !nodeEnd);
 }else
 {
  nodeBegin=begin;
  nodeEnd=end;
 }

 selectedNodesToFront(0);

 nodeBeginID=nodeBegin->ID;
 if(nodeEnd) nodeEndID=nodeEnd->ID;
 else        mapDistance.clear();
 //viewport()->repaint();

 QVector< QVector<qreal> > workGraph;

 // calc distances
 if(begin!=nodeEnd || !workGraph.count())
 {
//  qDebug() << "graph: " <<workGraph.count();
  if(!workGraph.count()) // end node is changed
  {
   mapTrack.clear();
   initWorkGraph(workGraph);
  }
  dijkstra(nodeBeginID,workGraph);
  mapDistance[nodeBeginID]=params.delayToMetro*60;
 }


 if(!nodeEnd) // just show time to all stations
 {
  // caching calculations
  foreach(TMapNode *node,nodeByID)
   if(mapDistance[node->ID]<INFINIT)
        node->Distance=QString::number(int(mapDistance[node->ID]/60));
   else node->Distance="";

  viewport()->update(); //foreach(TMapNode *node,nodeByID) node->update();
  return;
 }
 nodeBegin->setZValue(2);
 nodeEnd->setZValue(2);
 nodeEnd->update();
 QApplication::processEvents();

 findTrackInGraph(nodeBegin,nodeEnd,workGraph);

 // sorting tracks
 TMapTrack tmp;
 int i,j,k;
 for(i=0;i<mapTrack.count();i++)
 {
    k=i; tmp=mapTrack.at(i);
    for(j=i+1;j<mapTrack.count();j++)
      if (mapTrack.at(j).countSecond < tmp.countSecond) { k=j; tmp=mapTrack.at(j); }

    mapTrack.swap(k,i);
  }

 // finding minimum count transfers and set to second
 int min=1;
 for(int i=1;i<mapTrack.count();i++)
  if(mapTrack.at(i).countTransfer<=mapTrack.at(min).countTransfer)
  {
   if(mapTrack.at(i).countTransfer==mapTrack.at(min).countTransfer)
   {
    if(mapTrack.at(i).countSecond<mapTrack.at(min).countSecond) min=i;
   }
   else min=i;
  }
 if(mapTrack.count()>1 && mapTrack.at(0).countTransfer!=mapTrack.at(min).countTransfer) mapTrack.move(min,1); //swap(1,min);

// qDebug() << "tracks: " << mapTrack.count();
// for(int i=0;i<mapTrack.count();i++) qDebug() << i+1 << mapTrack.at(i).countSecond;

 QStringList trackList;
 if(!config.MobileVersion)
 {
  for(int i=0;i<mapTrack.count();i++) trackList << QString::number(i+1)+") "+getMessageTrackInfo(i,1);
 }
 else
 {
  for(int i=0;i<mapTrack.count();i++) trackList << "";
  showTrack(0);
 }
 emit trackNames(trackList);
}

void TMapView::showTrack(int i)
{
// testTimer.start();
 if(i<0 || !mapTrack.count()) return;

 state.mapLoaded=0;
// QApplication::processEvents();
 mapStartDragging();
 QString info,text;
 selectedNodesToFront(0); // previous trackIndex
 selectedNode.clear();
 selectedNode << mapTrack.at(i).selectedNode;
// selectedNode.clear();
// selectedEdge.clear();
// mapDistance.clear();
// selectedNode << track.selectedNode;
// selectedEdge << track.selectedEdge;
// mapDistance << track.mapDistance;

 info=getMessageTrackInfo(i);

 text=getMessageTrackDescription(i);
// qDebug() << mapTrack.at(i).mapDistance;
 // caching calculations
 foreach(TMapNode *node,nodeByID)
  if(mapTrack.at(i).mapDistance[node->ID]<INFINIT)
       node->Distance=QString::number(int(mapTrack.at(i).mapDistance[node->ID]/60));
  else node->Distance="";

 emit nodeHasInfo(0);
 emit trackDescription(text);
 if(config.MobileVersion)
      text=(mapTrack.count()>1 ? QString::number(i+1)+") ":"")+info;
 else text=info+text;
 emit trackText(text);

 if(config.MobileVersion && mapTrack.count()>1)
   info="<small><font face=monospace>&nbsp;"+QString::number(i+1)+".</font></small>"+info;
 emit trackInfo(info);
 emit trackFinded(1);

 trackIndex=i;
 selectedNodesToFront(1);
 //qDebug() << id;
 state.mapLoaded=1;
// viewport()->update();
// qDebug() << testTimer.elapsed();
 QApplication::processEvents();
}

void TMapView::showNextTrack()
{
 if(mapTrack.count()<=1) return;
 int i=trackIndex+1;
 if((config.MobileVersion && i==4) || i==mapTrack.count())  i=0;

 if(config.MobileVersion)
      showTrack(i);
 else emit trackShowIndex(i);
}

void TMapView::showPreviousTrack()
{
 if(mapTrack.count()<=1) return;
 int i=trackIndex-1;
 if(i==-1)  i=((config.MobileVersion && mapTrack.count()<4) ? mapTrack.count()-1:3);

 if(config.MobileVersion)
      showTrack(i);
 else emit trackShowIndex(i);
}

void TMapView::clearTrack()
{
// qDebug() << "clear track";
 nodeBeginID=-1;
 nodeEndID=-1;
 if(nodeBegin) nodeBegin->update();
 if(nodeEnd) nodeEnd->update();
 nodeBegin=NULL;
 nodeEnd=NULL;
 nodeClick=NULL;
 selectedNodesToFront(0);
 mapTrack.clear();
 trackIndex=0;
 //if(nodeByID.count()) foreach(TMapNode *node,nodeByID) node->update();
 emit trackText("");
 emit nodeClicked(0);
 emit nodeHasInfo(0);

 if(config.MobileVersion) emit trackFinded(0); // hide LabelInfo & Search
 else                     emit trackNames(QStringList()); // clear list of routes
 // viewport()->repaint();
 viewport()->update();
}



void TMapView::dijkstra(int nodeBeginID, QVector< QVector<qreal> > &workGraph)
{
 QQueue<QPair<int,int> > queue;
 QPair <int,int> qtmp;
 int i,j,count=workGraph.count();
 QVector<bool> visit;

 visit.resize(count);
 mapDistance.resize(count);
 mapParentNode.resize(count);

 for (int i=0; i<count; i++)
 {
  mapDistance[i] = INFINIT;
  mapParentNode[i] = -1;
  visit[i] = false;
 }

 mapDistance[nodeBeginID] = 0;
 queue.push_back(QPair <int,int> (mapDistance[nodeBeginID], nodeBeginID));
 while(!queue.empty())
 {
  qtmp = queue.first();
  queue.pop_front();
  i = qtmp.second;

  if (!visit[i])
  {
   visit[i] = true;
   for (j = 0; j<count; j++)
    if (!visit[j] && workGraph[i][j] && workGraph[i][j]>0)
    {
     qreal curDistance=mapDistance[i] + workGraph[i][j];
     if(curDistance < mapDistance[j])
     {
      mapDistance[j] = curDistance;
      mapParentNode[j] = i;
      queue.push_back(QPair <int,int>(-curDistance, j));
     }
    }
  }
 }
}

QList< QPair<QString,QColor> > TMapView::getNamesAndColors()
{
 QList< QPair<QString,QColor> > data;
 foreach(TMapNode *node,nodeByID)
 {
  QPair<QString,QColor> pair;
  pair.first=node->DisplayName;
  pair.second=node->Route->color;
  data << pair;
 }
 return data;
}

void TMapView::defaultConfig()
{
 QLocale::Country country=QLocale::system().country();
 QString city;

 city="Moscow";
 if(country==QLocale::Ukraine) city="Kiev"; else
 if(country==QLocale::Mexico) city="Mexico"; else
 if(country==QLocale::France) city="Paris"; else
 if(country==QLocale::Netherlands) city="Amsterdam"; else
 if(country==QLocale::Country(QLocale::Czech)) city="Praha"; else
 if(country==QLocale::Country(QLocale::Spanish)) city="Madrid"; else
 if(country==QLocale::Canada) city="Montreal"; else
 if(country==QLocale::UnitedStates) city="New-York"; else
 if(country==QLocale::Belgium) city="Brussels"; else
 if(country==QLocale::Germany) city="Berlin"; else
 if(country==QLocale::Austria) city="Wienn"; else
 if(country==QLocale::UnitedKingdom) city="London"; else
 if(country==QLocale::Italy) city="Milano"; else
 if(country==QLocale::Finland) city="Helsinki"; else
 if(country==QLocale::Brazil) city="San-Paulo"; else
 if(country==QLocale::Thailand) city="Bangkok"; else
 if(country==QLocale::India) city="Delhi"; else
 if(country==QLocale::China) city="Beijing"; else
 if(country==QLocale::HongKong) city="Hong-Kong"; else
 if(country==QLocale::Taiwan) city="Taipei"; else
 if(country==QLocale::RepublicOfKorea) city="Seoul"; else
 if(country==QLocale::Japan) city="Tokyo";

 config.mapLastFile=params.appMapPath+city+".pmz";

 if(!QFile::exists(config.mapLastFile))
 {
  QDir Dir=QDir(params.appMapPath,"*.pmz",QDir::Name,QDir::Files);
  if(!Dir.entryList().empty()) config.mapLastFile=params.appMapPath+Dir.entryList().first();
 }
 //qDebug() << config.mapLastFile;
 config.enableCustomFont=0;
 config.enableLowqDrag=1;
 config.enableShowScrollBar=0;
 config.enableAntialiasing=1;
 config.enableSimpleTransfer=0;
 config.enableWriteTime=1;
 config.enableSimpleNode=0;
 config.enableDrawBackground=1;
 config.enableCaching=config.MobileVersion;
 config.enableSwipeRoutes=config.MobileVersion;
 config.valueTransparency=60;
 config.enableKineticScroll=0;
 params.mapColorBack=Qt::white;
}

void TMapView::applyConfig()
{
 setRenderHints((config.enableAntialiasing ? Antialiasing:NonAntialiasing));
 if(config.enableShowScrollBar)
 {
  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
 }else
 {
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
 }

 if(config.enableAntialiasing)
      params.nodeFont.setStyleStrategy(QFont::PreferAntialias);
 else params.nodeFont.setStyleStrategy(QFont::PreferDefault);
 //params.nodeFont.setStyleStrategy(QFont::ForceOutline);

 if(config.enableCaching)
      setViewportUpdateMode(FullViewportUpdate);
 else setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

 if(mapFade) mapFade->FadeOpacity=config.valueTransparency/qreal(100);

 if(config.enableCaching && state.mapCached)
 {
  QFile::remove(params.mapCachePath+".cache");
  state.mapCached=0;
 }

 if(config.enableKineticScroll)
 {
  fcharm=new FlickCharm(this);
  fcharm->activateOn(this);
  connect(fcharm,SIGNAL(started()),this,SLOT(mapStartDragging()));
  connect(fcharm,SIGNAL(finished()),this,SLOT(mapStopDragging()));
 }


 // its last operation - load last map on app start
 if(params.mapFile.isEmpty() && !config.mapLastFile.isEmpty()) loadMap(config.mapLastFile);

 viewport()->update();
 //foreach(TMapNode *node,nodeByID) node->update();
}

void TMapView::setDelayMetro(int delay)
{
// workGraph.clear();
 params.delayToMetro=delay;
 if(nodeEnd) findTrack(nodeBegin,nodeEnd);
 else
 if(nodeBegin) findTrack(nodeBegin);
}
void TMapView::setDelayIndex(int index)
{
// workGraph.clear();
 params.delayIndex=index;
 if(nodeEnd) findTrack(nodeBegin,nodeEnd);
 else
 if(nodeBegin) findTrack(nodeBegin);
}


int TMapView::strTimeToSec(const QString &strTime)
{
 if(strTime.isEmpty() || strTime[0]=='0') return 0;

 int Sec=0;
 if(strTime.indexOf(".")!=-1)
      Sec=strTime.section(".",0,0).toInt()*60+strTime.section(".",-1).toInt();
 else Sec=strTime.toInt()*60;

 return Sec;
}

QString TMapView::convertToTextWithTags(QString text)
{
 QRegExp url("((mailto:|(news|(ht|f)tp(s?))://)[^<\\s]*)");
 QString TagText,br=" <br>";

 text.replace("\\n",br);
 text.replace('\n',br);
 TagText=text;

 int offset=0,index=text.indexOf(url); //url.indexIn(text);
 while(index != -1)
 {
  QString StrUrl=url.cap();
  char lastSymbol=StrUrl.right(1).toLatin1().at(0);
  if(lastSymbol==',' || lastSymbol=='.') StrUrl.chop(1);
  int UrlLength=StrUrl.length();

  QString LinkName=QUrl::fromEncoded(StrUrl.toLatin1()).toString();
  LinkName.replace("http://www.","",Qt::CaseInsensitive);
  LinkName.replace("http://","",Qt::CaseInsensitive);
  lastSymbol=LinkName.right(1).toLatin1().at(0);
  if(lastSymbol=='/') LinkName.chop(1);

  TagText.replace(index+offset,UrlLength,QString("<a href=\"%1\">").arg(StrUrl) + QString("%1</a>").arg(LinkName));
  offset+=LinkName.length()+15; //+ count symbols of tag
  StrUrl.fill('x');
  text.replace(index,UrlLength,StrUrl);
  index=text.indexOf(url);
 }

 return TagText;
}

void TMapView::initNodeInfo()
{
 QString HasInfoFileName=".info";
 QSettings inf(params.mapCachePath+HasInfoFileName,QSettings::IniFormat);
 if(QFile::exists(params.mapCachePath+HasInfoFileName))
 {
  foreach(TMapRoute *route,layer.first()->route)
  {
   inf.beginGroup(QString::number(route->ID));
   foreach(TMapNode *node,route->node)
     node->HasInfo=inf.value(QString::number(node->ID),"0").toBool();

   inf.endGroup();
  }
  return;
 }
 QDir Dir=QDir(params.mapCachePath,"*.txt",QDir::Name,QDir::Files);
 QStringList infoList=Dir.entryList();
 foreach(TMapRoute *route,layer.first()->route)
 {
  int CountInfo=0;
  inf.beginGroup(QString::number(route->ID));
  foreach(const QString &infoFile,infoList)
  {
   INISettings Info(params.mapCachePath+infoFile,codePage);
   if(!Info.value("Options/AddToInfo",0).toInt()) continue;

   QString StrInfo;
   foreach(TMapNode *node,route->node)
   {
    if(node->HasInfo) continue;
    StrInfo=Info.value(QString("%1/%2").arg(route->Name).arg(node->Name));
    if(StrInfo.length()<5) continue;
    CountInfo++;
    node->HasInfo=1;
    inf.setValue(QString::number(node->ID),"1");
    if(CountInfo==route->node.count()) break;
   }
   if(CountInfo==route->node.count()) break;
  }
  inf.endGroup();
 }
}

void TMapView::loadNodeInfo(TMapNode *node)
{
 if(node->Info.count()) return;
 QDir Dir=QDir(params.mapCachePath,"*.txt",QDir::Name,QDir::Files);
 QStringList infoList=Dir.entryList();
 foreach(const QString &infoFile,infoList)
 {
  INISettings Info(params.mapCachePath+infoFile,codePage);
  if(!Info.value("Options/AddToInfo",0).toInt()) continue;

  QString Caption,Type=Info.value("Type").toUpper();

  TMapNode::InfoItem Item;
  if(Type=="IMAGE") Item.Type=TMapNode::itImage; else Item.Type=TMapNode::itText;
  Item.Value=Info.value(QString("%1/%2").arg(node->Route->Name).arg(node->Name));
  if(Item.Value.length()<5) continue;
  if(Item.Type==TMapNode::itText) Item.Value=convertToTextWithTags(Item.Value);
  Item.Value=Info.value("Options/StringToAdd").remove('\'')+Item.Value;
  //qDebug()<< Item.Value;
  Caption=Info.value("Options/Caption").remove('\'');
  if(node->Info.contains(Caption))
   Item.Value=((TMapNode::InfoItem)node->Info.value(Caption)).Value+"<br>"+Item.Value;
  node->Info.insert(Caption,Item);
 }
}

void TMapView::setNodeBegin(TMapNode *Node)
{
 if(!Node) Node=nodeClick;
 nodeBeginID=-1;
 if(nodeBegin) nodeBegin->update();
 nodeBegin=NULL;
 findTrack(Node,nodeEnd);
}
void TMapView::setNodeEnd(TMapNode *Node)
{
 if(!Node) Node=nodeClick;
 findTrack(nodeBegin,Node);
}

void TMapView::saveToFile()
{
 if(!QImageWriter::supportedImageFormats().count()) return;

 QList<QByteArray> formats;
 formats << QImageWriter::supportedImageFormats();
 if(formats.contains("jpg")) formats.move(formats.indexOf("jpg"),0);
 if(formats.contains("png")) formats.move(formats.indexOf("png"),0);
 QString supportFormats;
 foreach(QString ext,formats)
  supportFormats+=QString("%1 (*.%2);;").arg(ext.toUpper()).arg(ext);

 QString filter=supportFormats.left(supportFormats.indexOf(";;")),
         fileName=QFileDialog::getSaveFileName(0, tr("Save file"), params.appPath,
                                               supportFormats,&filter);
 if(fileName.isEmpty()) return;

//#ifndef QT_NO_CURSOR
// QApplication::setOverrideCursor(Qt::WaitCursor);
//#endif

 QString ext = filter.left(4).trimmed();
 exportMap(fileName,ext);

//#ifndef QT_NO_CURSOR
// QApplication::restoreOverrideCursor();
//#endif
}

void TMapView::initTranslit()
{
 if(BGN.count()) return;
 QFile translit(":/translit_ru");
 translit.open(QFile::ReadOnly | QFile::Text);
 QString text=translit.readAll();
 QStringList chrs=text.split(',');
 translit.close();

 for(int i=0;i<chrs.count()-1;i=i+2)
 {
     BGN[chrs.at(i)[0]]=chrs.at(i+1);
     //qDebug() << chrs.at(i)[0] << chrs.at(i+1);
 }
}

void TMapView::convertToTranslit(QString &text)
{
 if(!BGN.count()) return;
 int max=text.count()-1;

 for(int i=max;i>=0;i--)
 {
  QString chr=BGN[ text[i] ];
  if(chr!="") text.replace(i,1,chr);
 }
}

void TMapView::findedNodesToFront(QList<int> NodeID)
{
 if(mapTrack.count() &&  mapTrack.at(trackIndex).selectedEdge.count()) return;
 selectedNodesToFront(0);
 selectedNode.clear();
 foreach(int ID,NodeID) selectedNode << nodeByID[ID];
 selectedNodesToFront(1);
 emit nodeClicked(1);
}

bool TMapView::viewportEvent(QEvent *event)
{
 switch (event->type()) {
 case QEvent::TouchBegin:
 {
// case QEvent::TouchUpdate:
// case QEvent::TouchEnd:
// {
//  QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
//  QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
//  if(touchPoints.count() == 2)
//  {
//   const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
//   const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
//   qreal currentScaleFactor=
//         QLineF(touchPoint0.pos(),touchPoint1.pos()).length() /
//         QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();
//   if(touchEvent->touchPointStates() & Qt::TouchPointReleased)
//   {
//    params.mapZoomFactor *= currentScaleFactor;
//    currentScaleFactor = 1;
//   }
//   setTransform(QTransform().scale(params.mapZoomFactor * currentScaleFactor,params.mapZoomFactor * currentScaleFactor));
//   nodeClick=NULL;
//  }
  return true;
 }
 case QEvent::Gesture:
  if(QGesture *pinch=((QGestureEvent*)event)->gesture(Qt::PinchGesture))
  {
   nodeClick=NULL;
   QPinchGesture::ChangeFlags changeFlags = ((QPinchGesture*)pinch)->changeFlags();
   if(changeFlags & QPinchGesture::ScaleFactorChanged)
   {
    scaleMap(((QPinchGesture*)pinch)->scaleFactor());
   }
   return true;
  }
  if(config.enableSwipeRoutes)
  if(QGesture *swipe=((QGestureEvent*)event)->gesture(Qt::SwipeGesture))
  {
   if(swipe->state()==Qt::GestureFinished)
   {
    if(((QSwipeGesture*)swipe)->verticalDirection()==QSwipeGesture::Up)     showNextTrack();
    if(((QSwipeGesture*)swipe)->verticalDirection()==QSwipeGesture::Down) showPreviousTrack();
   }
//   qDebug() << "swipe test";
   return true;
  }

 break;
 default: ;
 }
 return QGraphicsView::viewportEvent(event);
}


//bool TMapView::OnSwipeGesture(QSwipeGesture* pSwipe)
//{
//   bool result = false;

//   if (pSwipe->state() == Qt::GestureFinished) {
//      qDebug("Swipe angle: %f", pSwipe->swipeAngle());
//      switch (SwipeGestureUtil::GetHorizontalDirection(pSwipe)) {
//         case QSwipeGesture::Left:
//            qDebug("Swipe Left detected");
/////
//            result = true;
//            break;
//         case QSwipeGesture::Right:
//            qDebug("Swipe Right detected");
/////
//            result = true;
//            break;
//         default:
//            break;
//      }
//   }

//   return result;
//}
