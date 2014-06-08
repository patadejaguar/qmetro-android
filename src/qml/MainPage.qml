import QtQuick 1.1
import com.nokia.meego 1.0
import QMLMainWindow 1.0

Page {
 property string dirMap: mainWnd.dirMap
 property string msgInstalledMaps: mainWnd.msgInstalledMaps
 property string msgAddMap: mainWnd.msgAddMap
 property bool firstStart: mainWnd.firstStart
 property bool fullScreen: mainWnd.fullScreen

 id: mainPage
 tools: commonTools

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
 }

 function runOpen(){ mainWnd.runOpen(); }
 function runSaveAs(){ mainWnd.runSaveAs(); }
 function runSettings(){ mainWnd.runSettings(); }
 function runAddMap(){ mainWnd.runAddMap(); }
 function runAbout(){ mainWnd.runAbout(); }

 function loadMap(fileName){ mainWnd.loadMap(fileName); }
 function getActionText(index){ return mainWnd.getActionText(index); }

 Component.onDestruction: mainWnd.close();
}

