#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include "mapview.h"
#include "maproute.h"
#include "mapnode.h"
#include "mapgraphics.h"

static const QRect  nullR=QRect ();
static const QPoint nullP=QPoint();
extern qreal INFINIT;


TMapNode::TMapNode(TMapView *nodeMap,TMapRoute *nodeRoute, QString nodeName, int nodeID)
{
 ID=nodeID;
 Name = nodeName;
 Route=nodeRoute;
 TypeNode=ntConstruct;
 HasTransfer=0;
 HasInfo=0;

 setPos(QPoint(0,0));
 //setFlag(ItemIsSelectable);
 setFlag(ItemIsFocusable);
 setZValue(-1);
 //setFlag(ItemSendsGeometryChanges);
 //setCacheMode(DeviceCoordinateCache);
 setBoundingRegionGranularity(1);

 Map=nodeMap;
 NRect=nullR;
 Rect=nullR;

#ifndef Q_OS_WINCE
 if(!Map->config.MobileVersion) setCursor(Qt::PointingHandCursor);
#endif
}
void TMapNode::init()
{
 qreal sWidth=Map->params.nodeWidth;
 QPixmap pix(1,1);
 QPainter p(&pix); // fake
 p.setFont(Map->params.nodeFont);
 DisplayName=(Alias.isEmpty() ? Name:Alias);
 if(Map->config.enableTranslit) Map->convertToTranslit(DisplayName);
 if(!Map->config.MobileVersion) setToolTip(DisplayName);

 if(Rect!=nullR) NRect=QRect(Rect.topLeft()-pos().toPoint()-QPoint(1,0),QSize(Rect.width(),Rect.height()));
 else NRect=QRect(QPoint(-sWidth,-sWidth),QSize(0,0));
 RectEllipse=QRectF(QPointF(-sWidth,-sWidth),QPointF(sWidth,sWidth));
 Align=Map->params.nodeFontStyle;
 Vertical=0;

 if(Rect!=nullR)
 {
 if(NRect.width()<NRect.height())
 {
  if(NRect.topLeft().y()<-sWidth)
  {
   NRect90=QRect(NRect.bottomRight()+QPoint(3,3),QSize(NRect.height()+NRect.width()+10,NRect.width()*2));
   Vertical=-1;
  }
  else
  {
   NRect90=QRect(NRect.topRight()+QPoint(3,-NRect.width()),QSize(NRect.height()+NRect.width()+10,NRect.width()*2));
   Vertical=1;
  }
  Align+=Qt::AlignLeft;
  QMatrix matrix;
  QPainterPath pRect;

  p.drawText(NRect90,Align,DisplayName,&NRect90);
  pRect.addRect(NRect90);
  matrix.rotate(90*Vertical);
  pRect=matrix.map(pRect);
  NRect=pRect.boundingRect().toRect();

 }else
 {
  if(Rect.center().x()-sWidth>pos().x())
       Align+=Qt::AlignLeft;
  else Align+=Qt::AlignRight;
  p.drawText(NRect,Align,DisplayName,&NRect);
 }
 }

////////////////////////////////////////////
// TextLayout with setCacheEnabled(1) more perfomance but trouble...
//
// TextLayout = new QTextLayout(DisplayName,Map->params.nodeFont);
// QTextOption option;
// option.setAlignment(Qt::Alignment(Align));
// option.setUseDesignMetrics(1);
// //option.setWrapMode(QTextOption::WordWrap);
// TextLayout->setTextOption(option);
// //TextLayout->setFont();
//
// qreal vertPos=0;
// QTextLine line;
// TextLayout->beginLayout();
// line=TextLayout->createLine();
// while (line.isValid())
// {
//  line.setLineWidth(NRect.width());
//  line.setPosition(QPointF(0, vertPos));
//  vertPos+=line.height()-2;
//  line=TextLayout->createLine();
// }
// TextLayout->endLayout();
// TextLayout->setCacheEnabled(1);
////////////////////////////////////////////


 // caching calculations

 if(Map->config.enableSimpleNode)
 {
//  const int size=int(Map->conf.StationWidth/2);
//  QPolygon points(7);
//  points[0] = QPoint(2*size,0);
//  points[1] = QPoint(size,-size*173/100);
//  points[2] = QPoint(-size,-size*173/100);
//  points[3] = QPoint(-2*size,0);
//  points[4] = QPoint(-size,size*173/100);
//  points[5] = QPoint(size,size*173/100);
//  points[6] = points[0];
//  PathNode.addPolygon(points);
  PathNode.addRect(QRect(-sWidth,-sWidth,sWidth*2,sWidth*2));
 }
 else
  PathNode.addEllipse(nullP, sWidth,sWidth);


 int scale=(Map->config.MobileVersion ? 3:2);
 qreal nodeSize=Map->params.nodeWidth*scale;
 QPainterPath NodeShape;
 NodeShape.addRect(-nodeSize/2,-nodeSize/2,nodeSize,nodeSize); //addEllipse(nullP, Map->params.nodeWidth*scale+more,Map->params.nodeWidth*scale+more);
 if(Map->config.MobileVersion)
 {
  int offset=3;
  PathShape.addRect(NRect.adjusted(-offset,-offset,offset,offset));
 }
 else
 PathShape.addRect(NRect);

 PathShape=PathShape.united(NodeShape);
 RectBound=PathShape.boundingRect();

// EndPen=QPen(Qt::red,sWidth/qreal(2));
// BeginPen=QPen(Qt::green,sWidth/qreal(2));
// TransferPen=QPen(Qt::white,1);
// NormalPen=QPen(Qt::yellow,1);
// ConstructPen=QPen(Route->color,2);
// RouteColorDark=Route->color.darker();

 if(Vertical)
      TextRect=NRect90;
 else TextRect=NRect;
 TextRectAdjustedTL=TextRect.adjusted(qreal(-0.6),qreal(-0.6),qreal(-0.6),qreal(-0.6));
 TextRectAdjustedBR=TextRect.adjusted(qreal(0.6),qreal(0.6),qreal(0.6),qreal(0.6));
}

QRectF TMapNode::boundingRect() const
{
 return RectBound;
}

QPainterPath TMapNode::shape() const
{
 return PathShape;
}

void TMapNode::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *)
{Q_UNUSED(option);
 if(!Map->state.mapLoaded) return;

 if(Map->config.enableCaching && Map->state.mapCached)
 {
  bool sign=1;
  foreach(TMapNode *node,Map->selectedNode)
   if(node->ID==ID) {sign=0; break;}

  if(Map->nodeBeginID!=ID && Map->nodeEndID!=ID && sign) return;
 }

 if(HasTransfer) (HasInfo ? p->setPen(Route->TransferInfoPen):p->setPen(Route->TransferPen));
 else            (HasInfo ? p->setPen(Route->NormalInfoPen):p->setPen(Route->NormalPen));

 p->setBrush(Route->color);

 if(TypeNode==ntConstruct)
 {
  (HasInfo ? p->setPen(Route->ConstructInfoPen):p->setPen(Route->ConstructPen));
  p->setBrush(Map->params.mapColorBack);
 }else
 if(Map->nodeBeginID==ID)
 {
  p->setPen(Route->BeginPen);
  p->setBrush(Qt::black);
 }else
 if(Map->nodeEndID==ID)
 {
  p->setPen(Route->EndPen);
  p->setBrush(Qt::black);
 }

 p->drawPath(PathNode);
 if(Map->config.enableWriteTime && Map->nodeBegin && TypeNode!=ntConstruct && Map->mapDistance.count()
    && (!Map->config.enableCaching || (Map->config.enableCaching && Map->state.mapCached)) )
 {
  // write times
  p->setFont(Map->params.nodeTimeFont);
  p->setPen(Qt::white);
  p->setBackgroundMode(Qt::TransparentMode);
  p->drawText(RectEllipse,Qt::AlignCenter,Distance);                                          //QString::number(ID));
 }
 //else { p->setFont(QFont("Tahoma",5)); p->drawText(EllipseRect,Qt::AlignCenter,QString::number(int(ID)));}

 // Station caption
 if(Rect!=nullR)
 {
  p->setFont(Map->params.nodeFont);
//  if(TextLayout->font()!=Map->params.nodeFont)
//   TextLayout->setFont(Map->params.nodeFont);
  p->setBrush(Route->labelsBColor);

  if(Vertical) p->rotate(90*Vertical);


  if(Map->config.enableCustomFont)
  {
   if(Map->config.enableStationBackColor)
   {
    p->setBackgroundMode(Qt::OpaqueMode);
    p->setBackground(Map->params.nodeColorBack);
   }
  }
  else // backcolor
  if(Route->labelsBColor.alpha())
  {
   p->setBackgroundMode(Qt::OpaqueMode);
   p->setBackground(Map->params.mapColorBack);
  }
  else // shadow
  {
   p->setBackgroundMode(Qt::TransparentMode);
   p->setPen(Map->params.mapColorBack);
   p->drawText(TextRectAdjustedTL,Align,DisplayName);

   p->setPen(Route->ColorDark);
   p->drawText(TextRectAdjustedBR,Align,DisplayName);
  }

  if(Map->config.enableCustomFont && Map->config.enableStationShadowColor)
  {
   p->setBackgroundMode((Map->config.enableStationBackColor ? Qt::OpaqueMode:Qt::TransparentMode));
   p->setPen(Map->params.nodeColorShadow);
   p->drawText(TextRectAdjustedBR,Align,DisplayName);
   p->setBackgroundMode(Qt::TransparentMode);
  }

  p->setPen(Route->color);

  if(Vertical)
       p->drawText(NRect90,Align,DisplayName,&NRect90);
  else p->drawText(NRect,Align,DisplayName,&NRect);


// debug regions
//  p->setPen(QPen(QBrush(Route->color),2));
//  p->setBrush(Qt::NoBrush);
//  if(Vertical)
//       p->drawRect(NRect90);
//  else p->drawRect(NRect);

//  p->setPen(QPen(QBrush(Route->color),1));
//  p->drawPath(PathShape);

  //TextLayout->draw(p,TextRect.topLeft());

 }
}

int TMapNode::type() const
{
 return elNode;
}


