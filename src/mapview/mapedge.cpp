#include <qdebug.h>
#include "mapedge.h"
#include "maproute.h"
#include "mapview.h"

TMapEdge::TMapEdge(TMapRoute *mapRoute, QString nodeName1, QString nodeName2,
                   QString edgeCost,EdgeType edgeType,EdgeStyle edgeStyle,
                   TMapRoute *mapRouteTransfer)
{
 TMapView *map;
 int id1,id2;
 route=mapRoute;
 type=edgeType;
 style=edgeStyle;

// if(nodeName1=="Bedford Pk Blvd" || nodeName2=="Bedford Pk Blvd")
// {
//  if(mapRouteTransfer!=NULL) qDebug() << "tr>> "+mapRouteTransfer->name;

//  qDebug() << route->name << nodeName1 << nodeName2;
// }


 if(mapRouteTransfer==NULL)
 {
  node[0]=route->findNode(nodeName1,nodeName2);
  node[1]=route->findNode(nodeName2,nodeName1);
 }
 else
 {
  node[0]=route->findNode(nodeName1);
  node[1]=mapRouteTransfer->findNode(nodeName2);
 }

 if(node[0]==NULL || node[1]==NULL) return;

 map=(TMapView*)node[0]->scene()->parent();
 id1=node[0]->ID;
 id2=node[1]->ID;

 if(mapRouteTransfer==NULL)
 if(map->mapGraph[id1][id2]!=NULL || map->mapGraph[id2][id1]!=NULL)
 {
  node[0]=NULL;
  node[1]=NULL;
  return;
 }

 if(type==etTransfer)
 {
  node[0]->HasTransfer=1;
  node[1]->HasTransfer=1;
 }

 if(edgeCost.isEmpty()) edgeCost="0";

 // cost in sec
// if(edgeCost.indexOf(".")!=-1)
//      cost=edgeCost.section(".",0,0).toInt()*60+edgeCost.section(".",-1).toInt();
// else cost=edgeCost.toInt()*60;
 cost=map->strTimeToSec(edgeCost);

 if(cost && type!=etTransfer)
 {
  type=etNormal;
  node[0]->TypeNode=TMapNode::ntNormal;
  node[1]->TypeNode=TMapNode::ntNormal;
 }//else cost=INFINIT;


// if(type!=etConstruct)
// {
//  node[0]->type=TMapNode::ntNormal;
//  node[1]->type=TMapNode::ntNormal;
// }

 if(node[0]->pos()==QPoint(0,0))
 {
  node[0]->setVisible(0);
  style=esInvisible;
 }
 if(node[1]->pos()==QPoint(0,0))
 {
  node[1]->setVisible(0);
  style=esInvisible;
 }

 if(map->mapGraph[id1][id2]==NULL) map->mapGraph[id1][id2]=this;
 if(mapRouteTransfer==NULL &&
    map->mapGraph[id2][id1]==NULL) map->mapGraph[id2][id1]=this;

  assigned=1;
}
