#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "mapelement.h"
#include "maproute.h"
#include "mapnode.h"
#include "mapedge.h"
#include "mapview.h"

TMapElement::TMapElement(ElementType type,QRectF rect)
{
 eType=type;
 eRect=rect;
 setZValue(1);
 FadeOpacity=qreal(0.6);
 //if(type==elScreen) setEnabled(0);

// setAcceptHoverEvents(0);
// setAcceptTouchEvents(0);
// setAcceptedMouseButtons(0);

}

QRectF TMapElement::boundingRect() const
{
 return eRect;
}

int TMapElement::type() const
{ return eType; }

//void TMapElement::mousePressEvent(QGraphicsSceneMouseEvent *event)
//{

// QGraphicsItem::mousePressEvent(event);
//}


void TMapElement::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{Q_UNUSED(option); Q_UNUSED(widget);

 switch(eType)
 {
  case elFade:
  {
//   QList<TPaintCmd> paintTrack;
   TMapView *map=(TMapView*)scene()->parent();

   // fade rect
   p->setOpacity(FadeOpacity);
   p->setPen(Qt::NoPen);
   p->setBrush(map->params.mapColorBack);
   p->drawRect(eRect);

   // draw stroking track
   p->translate(QPoint(100,100)); // fade screen rect more than scene rect
   p->setOpacity(1);

   if(map->mapTrack.count()) // nodes
   {
    qreal width=map->params.nodeWidth*qreal(1.3);
    p->setPen(Qt::NoPen);
    p->setBrush(Qt::black);
    foreach(TMapNode *node,map->mapTrack.at(map->trackIndex).selectedNode)
      p->drawEllipse(node->pos(),width,width);
   }

   if(map->mapTrack.count()) // edges
   {
    QPainterPath tPath;
    QColor color=QColor(0,0,0,0);
    qreal width;
    p->setBrush(Qt::NoBrush);
    foreach(TMapEdge *edge, map->mapTrack.at(map->trackIndex).selectedEdge)
     if(edge->type==TMapEdge::etTransfer)
     {
      qreal w=map->params.routeWidth/qreal(1.7);
      tPath.addEllipse(edge->node[0]->pos(),w,w);
      tPath.addEllipse(edge->node[1]->pos(),w,w);
      tPath.moveTo(edge->node[0]->pos());
      tPath.lineTo(edge->node[1]->pos());
     }

    // black border for transfers
    p->setPen(QPen(Qt::black,map->params.routeWidth*qreal(1.5),Qt::SolidLine,Qt::FlatCap,Qt::RoundJoin));
    p->drawPath(tPath);
    foreach(TMapEdge *edge, map->mapTrack.at(map->trackIndex).selectedEdge)
    {
     if(edge->route && edge->type!=TMapEdge::etTransfer)
     {
      color=edge->route->color;
      width=edge->route->width;
     }
     else continue;

     // black border for lines
     p->setPen(QPen(Qt::black,width*qreal(1.4),Qt::SolidLine,Qt::FlatCap,Qt::RoundJoin));
     p->drawPath(edge->path);
     p->setPen(QPen(color,width,Qt::SolidLine,Qt::FlatCap,Qt::RoundJoin));
     p->drawPath(edge->path);
     //qDebug() << edge->node[0]->ID << edge->node[1]->ID;
    }
    p->setPen(QPen(map->params.mapColorBack.lighter(150),map->params.routeWidth*1.2,Qt::SolidLine,Qt::FlatCap,Qt::MiterJoin));
    p->drawPath(tPath);
   }

   break;
  }
  case elControl: break;
  default: break;
 }

}
