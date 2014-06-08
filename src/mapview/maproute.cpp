//#include <QDebug>
#include "maproute.h"
#include "mapedge.h"
#include "mapview.h"

TMapRoute::TMapRoute()
{
}

//TMapNode* TMapRoute::findNode(const QString &nodeName,const QString &nodePairName)
//{
// if(!node.count() || nodeName.isEmpty()) return NULL;

// for(int i=0;i<node.count();i++)
// {
//  if(this->node.at(i)->Name==nodeName) return this->node.at(i);
// }
// return NULL;
//}



TMapNode* TMapRoute::findNode(const QString &nodeName,const QString &nodePairName)
{
 if(!node.count() || nodeName.isEmpty()) return NULL;
 int found=-1;
 TMapView *map;
 map=(TMapView*)node[0]->scene()->parent();

 for(int i=0;i<node.count();i++)
 {
  if(node.at(i)->Name!=nodeName) continue;
  if(nodePairName.isEmpty())     return node.at(i);

  int id=node.at(i)->ID;
  //qDebug() << nodeName << nodePairName << id;
  if( (id>0 && map->nodeByID[id-1] && map->nodeByID[id-1]->Name==nodePairName) ||
      (id<map->nodeByID.count()-1 && map->nodeByID[id+1] && map->nodeByID[id+1]->Name==nodePairName))
       return node.at(i);
//  if( (id>0 && map->nodeByID[id-1] &&
//       map->mapGraph[id][id-1] && !map->mapGraph[id][id-1]->assigned) ||
//      (id<map->nodeByID.count()-1 &&
//       map->mapGraph[id][id+1] && map->mapGraph[id][id+1]->assigned ))
//       return node.at(i);

//  if((id>0 && map->mapGraph[id][id-1] && !map->mapGraph[id][id-1]->assigned) ||
//      id<map->nodeByID.count()-1 && map->mapGraph[id][id+1] && !map->mapGraph[id][id+1]->assigned)
//      return node.at(i);
  else found=i;
 }

 if(found!=-1)
      return node.at(found);
 else return NULL;
}
