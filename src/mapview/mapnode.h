#ifndef MAPNODE_H
#define MAPNODE_H

#include <QGraphicsItem>
#include <QMap>
#include <QTextLayout>

class QTextLayout;
class TMapRoute;
class TMapView;

const int elNode=-1;

class TMapNode : public QGraphicsItem
{

public:
    enum NodeType{ntNormal,ntConstruct};

    TMapNode(TMapView *nodeMap,TMapRoute *nodeRoute, QString nodeName, int nodeID/*,NodeType nodeType=ntConstruct,QPoint nodePoint=QPoint(0,0)*/);
    void init();
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    int type() const;

    //void mousePressEvent(QGraphicsSceneMouseEvent *event);
    //void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    // params
    int ID;
    QString Name,Alias;
    QRect Rect;
    int Height;
    TMapRoute *Route;
    NodeType TypeNode;
    QTextLayout *TextLayout;

    // info
    enum InfoItemType{itText,itImage};
    struct InfoItem
    {
     QString Value;
     InfoItemType Type;
    };

    QMap <QString,InfoItem> Info;

    // support
    bool HasTransfer;
    bool HasInfo;
    int Vertical,Align;
    QString DisplayName;
    QRect NRect,NRect90;
    QRectF RectEllipse,RectBound;
    QPainterPath PathNode,PathShape;

    // cache
    QString Distance;
    QRectF TextRect,TextRectAdjustedTL,TextRectAdjustedBR;


private:
    TMapView *Map;

//protected:
//   // bool eventFilter(QObject *obj, QEvent *event);
//    bool sceneEvent( QEvent *event);

signals:

public slots:

};

#endif // MAPNODE_H
