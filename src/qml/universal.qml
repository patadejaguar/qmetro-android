import QtQuick 1.1
import QMLMainWindow 1.0

Rectangle{    
 id: main
 width: 800
 height: 600

 PinchArea {
  property int pinchcount: 0;
  property real pinchscale: 1;

  pinch.maximumScale : 2
  pinch.minimumScale : 0.1
  anchors.fill: parent

  onPinchStarted:
  {
   pinchcount=0;
   pinchscale=1;
  }
  onPinchUpdated:
  {
   if(pinch.scale == pinch.previousScale) return;
   if((pinch.scale > 1 && pinch.previousScale < 1) || (pinch.scale < 1 && pinch.previousScale > 1) )
   {
    pinchcount=0;
    pinchscale=1;
   }
   pinchcount++;
   if(pinch.scale>1)
        pinchscale = 1 + ((pinch.scale-pinchscale)/pinchcount);
   else pinchscale = 1 - ((pinchscale-pinch.scale)/pinchcount*3);

//   console.log(pinch.scale + " " + pinch.previousScale);
   mainWnd.scaleMap(pinchscale);
  }

  onPinchFinished:
  {
   pinchcount=0;
   pinchscale=1;
  }
 }



 MainWindow{
  id: mainWnd
  anchors.fill: parent
  onToggledSearch:
  {
      search.x = mainWnd.searchX;
      search.y = mainWnd.searchY-search.height/2;
      search.width = mainWnd.searchWidth;
      search.height = mainWnd.searchHeight;
      search.visible = mainWnd.searchVisible;
  }
 }

 TextEdit {
     id: search
     visible: mainWnd.searchVisible
     text: mainWnd.searchText
     font.bold: true
//      font.family: "Helvetica"
//      font.pointSize: 20
//      color: "blue"
//      focus: true
     fillColor: Qt.rgba(255,0,0,0.5)
     textMargin: 10

     onTextChanged: mainWnd.searchText=search.text; //console.log(mainWnd.searchX+","+mainWnd.searchY+" "+mainWnd.searchWidth+" "+mainWnd.searchVisible);
 }


 Component.onDestruction: mainWnd.close();
}
