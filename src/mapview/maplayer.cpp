#include "maplayer.h"
#include "maproute.h"
//#include <QDebug>

TMapLayer::TMapLayer()
{

}

TMapRoute* TMapLayer::findRoute(QString routeName)
{
 if(!route.count() || routeName.isEmpty()) return NULL;

 foreach(TMapRoute *r,route)
 {
  //if(routeName=="Hammersmith & City") qDebug() << r->name;
  if(r->Name==routeName) return r;
 }

 return NULL;
}
