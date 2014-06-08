#ifndef MAP_H
#define MAP_H

//#include <QDebug>
#include <QGraphicsView>
#include <QTimer>
#include <QGestureEvent>
#include <QPinchGesture>

#include <QImageWriter>
#include <QThread>

#include "mapelement.h"

class TMapLayer;
class TMapRoute;
class TMapNode;
class TMapEdge;
class FlickCharm;
//class TMapElement;


 class TImageThreadWriter : public QObject
{Q_OBJECT
public:
  TImageThreadWriter(QString FileName, QString Format, QImage Image):
  imageThread(new QThread), imageWriter(new QImageWriter(FileName,Format.toLatin1())), imageData(Image)
  {
   this->moveToThread(imageThread);
   connect(imageThread,SIGNAL(started()),this,SLOT(writeImage()));
   connect(this,SIGNAL(doneWrite()),imageThread,SLOT(quit()));
   connect(this,SIGNAL(doneWrite()),this,SLOT(releaseTimerStart()));
   imageThread->start();
  }
  ~TImageThreadWriter() { delete imageThread; delete imageWriter; }

  QThread *imageThread;
  QImageWriter *imageWriter;
  QImage imageData;

private slots:
  void writeImage()
  {
   if(imageWriter->canWrite()) imageWriter->write(imageData);
   emit doneWrite();
  }
  void releaseTimerStart() { QTimer::singleShot(1000,this,SLOT(releaseImage())); }
  void releaseImage()
  {
   if(imageThread->isRunning()) { releaseTimerStart(); return; }
   imageData = QImage();
   imageThread->deleteLater();
   this->deleteLater();
  }

signals:
  void doneWrite();
};


class TMapTrack
{
//     QString info;
//     QString description;
public:
 TMapTrack(){}
 ~TMapTrack()
 {
  listAlarmTime.clear(); station.clear();
  selectedNode.clear(); selectedEdge.clear(); mapDistance.clear();
 }
 void setParams(int Seconds, int Stations, int Transfers){ countSecond=Seconds; countStation=Stations; countTransfer=Transfers; }
 void addStantion(int ID, /*QString Name, QColor Color,*/ QTime Time, bool IsTransfer=0, bool IsAlarm=1)
 {
  TStation st;
  st.id=ID;
  //st.name=Name;
  //st.color=Color;
  st.time=Time;
  st.isTransfer=IsTransfer;
  st.isAlarm=IsAlarm;
  station << st;
 }

 struct TStation
 {
  int id;
//  QString name;
//  QColor color;
  QTime time;
  mutable bool isTransfer;
  mutable bool isAlarm;
 };

 int countSecond;
 int countStation;
 int countTransfer;

 QString idTrack;

 QList<TStation> station;
 QList<QTime> listAlarmTime;

 QList<TMapNode*> selectedNode;
 QList<TMapEdge*> selectedEdge;
 QVector<qreal> mapDistance;
};






enum DrawType
{dtPenColor,dtBrushColor,dtLine,dtDashed,dtPolygon,dtSpline,dtEllipse,dtTextOut,dtAngleTextOut,dtOpaque,
 dtPeOpaqueAngleTextOut,dtImage,dtRailway,dtStairs,dtAngle,dtTranslate,dtPath,dtArrow,dtSpotCircle,dtSpotRect};

struct TPaintCmd
{
  QString param;
  DrawType type;

  QList<qreal> nums;
  QList<QColor> colors;
  QList<QString> strings;

  QPixmap *image;
  QPen pen;
  QBrush brush;
  QFont font;
  QPolygonF polygon;
  QPainterPath path;

};

class TMapView : public QGraphicsView
{
Q_OBJECT
 //friend class TMapElement;
 //friend void TMapElement::drawPaintCmd(QPainter *painter, QList<TPaintCmd> &paintCmd);

private:

    int timerId;

    struct AddCacheEdge
    {
     QString routeName[2];
     QString nodeName[2];
     QPolygon points;
     int edgeStyle;

     QString cost[2];
     bool transfer;

     bool assigned;
    };

    QList<AddCacheEdge*> edgeCache; // cache for additional nodes & transfers
    QHash<QChar, QString> BGN;
    QString WindowTitle;

    //void drawMapLayer(QPainter *painter, TMapLayer *mapLayer);

    void dijkstra(int nodeBeginID, QVector<QVector<qreal> > &workGraph);
    QString convertToTextWithTags(QString text);

public:
    TMapView(QString AppPath, QString CachePath);
    ~TMapView();
    bool viewportEvent(QEvent *event);
    bool OnSwipeGesture(QSwipeGesture* pSwipe);

    void scaleMap(qreal scaleFactor);
    void defaultConfig();
    void applyConfig();
    void loadMap(QString fileName);
    void loadMapImageToPaintCmd(QString imageFile, QList<TPaintCmd> &paintCmd);
    void loadMapLayers(QString LayerFilesTRP); // !! IN LAYER CLASS
    void initNodeInfo();
    void loadNodeInfo(TMapNode *node);

    void clearMap(); // !! DESTRUCT

    bool extractFiles(QString zipFile, QString extractFiles, QString extractDir);
    void exportMap(QString fileName,QString ext);
    void addPaintCmd(const QString &eParam,DrawType eType,QList<TPaintCmd> &paintCmd);
    void addLayerToPaintCmd(TMapLayer *mapLayer,QList<TPaintCmd> &paintCmd);
    void drawPaintCmd(QPainter *painter, QList<TPaintCmd> &paintCmd);
    int  strTimeToSec(const QString &strTime);

    void initWorkGraph(QVector<QVector<qreal> > &workGraph);
    void findTrackInGraph(TMapNode *begin,TMapNode *end=NULL, QVector< QVector<qreal> > workGraph=QVector< QVector<qreal> >());
    void findTrack(TMapNode *begin,TMapNode *end=NULL);
    void clearTrack();
    void selectedNodesToFront(bool enable);
    QString getMessageTrackInfo(int trackIndex, bool inLine=0);
    QString getMessageTrackDescription(int trackIndex);

    void initTranslit();
    void convertToTranslit(QString &text);

    QThread* writeImage(QString FileName, QString Format, QImage Image)
    { TImageThreadWriter *writer = new TImageThreadWriter(FileName,Format,Image); return writer->imageThread; }

    QTimer TimerPopup, TimerDragging, TimerAlarm;

    QList<TMapLayer*> layer;
    QList<TPaintCmd> paintBkg,paintMap;
    QList< QPair<QString,QColor> > getNamesAndColors();

    QVector< QVector<TMapEdge*> > mapGraph;
//    QVector< QVector<qreal> > workGraph;
    QVector<qreal> mapDistance;
    QVector<int> mapParentNode;
    QVector<TMapNode*> nodeByID;

//    QList<TMapNode*> selectedNode;
//    QList<TMapEdge*> selectedEdge;
    QList<TMapNode*> selectedNode;
    QList<TMapTrack> mapTrack;
    int trackIndex;

    QGraphicsScene *scene;
    QPixmap *CachedMap;
    //QImage *CachedMap;

    struct TParams
    {
     QString mapFile;
     QString mapCachePath;
     QString mapName;
     QString mapCityName;
     QString mapCityDisplay;
     QString mapCountryName;
     QString mapRusName;
     QString mapNeedVersion;
     QString mapAuthors;
     QString mapDelayNames;
     QColor  mapColorBack;
     bool    mapIsVector;
     qreal   mapZoomFactor;

     qreal  nodeWidth;
     qreal  nodeConstructWidth;
     qreal  routeWidth;
     QFont  nodeFont;    // +UpperCase
     QFont  nodeFontDefault;
     int    nodeFontStyle; // +WordWrap
     QFont  nodeTimeFont;
     QColor nodeColorBack;
     QColor nodeColorShadow;
     bool   nodeUpperCase;
     bool   nodeWordWrap;

     QString Transports;
     QString CheckedTransports;

     QString appPath;
     QString appCachePath;
     QString appMapPath;

     int delayToMetro;
     int delayIndex;
    };

    TParams params;

    struct TConfig
    {
     QString mapLastFile;

//     QFont fontNode;
     int valueTransparency;

     bool enableCustomFont;
     bool enableStationBackColor;
     bool enableStationShadowColor;

     bool enableLowqDrag;
     bool enableDrawBackground;
     bool enableShowScrollBar;
     bool enableAntialiasing;
     bool enableSimpleTransfer;
     bool enableSimpleNode;
     bool enableWriteTime;
     bool enableTranslit;
     bool enableCaching;
     bool enableSwipeRoutes;
     bool enableZoomKeys;

     bool enableKineticScroll;

     bool MobileVersion;
     QString locale;


     //bool enableTransliterate;
     //bool enableCaptionHint;
    };
    TConfig config;

    struct TMapState
    {
     bool mapLoaded;
     bool mapCached;
     bool mapDragging;
     bool nodeClick;
    };
    TMapState state;

    QPoint nodeClickPos;
    int nodeBeginID, nodeEndID;
    TMapNode *nodeBegin,*nodeEnd,*nodeClick;
    TMapElement *mapFade;

    QTextCodec *codePage;
    QMenu *nodePopup;
    FlickCharm *fcharm;

    int mapWidth,mapHeight;

public slots:
 void setDelayMetro(int delay);
 void setDelayIndex(int index);
 void zoomIn();
 void zoomOut();
 void fitMap();
 void setNodeBegin(TMapNode *Node=NULL);
 void setNodeEnd(TMapNode *Node=NULL);
 void popupByTimer();
 void saveToFile();
 void findTrackByID(int ID) { findTrack(nodeByID[ID]); }
 void findedNodesToFront(QList<int> NodeID);
 void mapStartDragging();
 void mapStopDragging();
 void showTrack(int trackIndex);
 void showNextTrack();
 void showPreviousTrack();

signals:
 void trackInfo(QString Info);
 void trackDescription(QString Description);
 void trackText(QString Text);
 void trackFinded(bool Check);
 void trackNames(QStringList);
 void trackShowIndex(int);

 void delayNames(QStringList);
 void nodeClicked(bool clicked);
 void nodeHasInfo(bool HasInfo);

protected:
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void drawBackground(QPainter *painter, const QRectF &rect);
    //void drawForeground(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    //void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);



    QRect mapRect;
};

#endif
