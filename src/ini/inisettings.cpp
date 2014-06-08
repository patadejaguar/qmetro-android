#include "inisettings.h"

#include "QRegExp"
#include "QFile"
#include "QTextStream"
#include "QIODevice"
#include "QDebug"

static QString PATTERN="%1(?=[^%2]*(%2[^%2]*%2[^%2]*)*$)";
static QRegExp SEPARATOR;

INISettings::INISettings(const QString FileName,QTextCodec *TextCodec)
:IndexGroup(-1),IndexValue(-1)
{
 DelimiterValue=",";
 QuoterValue="\"";
 SEPARATOR.setPattern(PATTERN.arg(DelimiterValue).arg(QuoterValue));


 QFile ini(FileName);
 ini.open(QFile::ReadOnly | QFile::Text);
 QTextStream data(&ini);
 data.setCodec(TextCodec);
 QString CommentChar="#;/";
 while(!data.atEnd())
 {
  QString str=data.readLine().trimmed();
  if(str.isEmpty() || CommentChar.contains(str[0])) continue;
  Line << str;
 }
 ini.close();
}
INISettings::~INISettings()
{
 Line.clear();
}

int INISettings::findGroup(const QString GroupName)
{
 int Index=0;
 foreach(QString str,Line)
 {
  //qDebug() << str;
  if((str[0]=='[' && str[str.length()-1]==']')
     && str.mid(1,str.length()-2).toUpper()==GroupName)
  {
   return Index;
  }
  Index++;
 }
 return -1;
}

void INISettings::beginGroup(const QString GroupName)
{
 IndexGroup=findGroup(GroupName.toUpper());
 if(IndexGroup!=Line.count()-1 && QString(Line.at(IndexGroup+1))[0]!='[')
      EndKey=0;
 else EndKey=1;
 IndexValue=IndexGroup+1;
 //qDebug() << IndexGroup;
}

void INISettings::findGroupInKey(QString &Key,const QString Slash)
{
 int pos=Key.lastIndexOf(Slash);
 if(pos!=-1)
 {
  QString GroupName;
  GroupName=Key.mid(0,pos);
  Key=Key.mid(pos+1,Key.length());
  beginGroup(GroupName);
  //qDebug() << GroupName << Key << IndexGroup;
 }
 Key=Key.toUpper();
}

QString INISettings::value(QString Key,const QString DefaultValue,ValueType Type)
{
 findGroupInKey(Key,"/");
 //qDebug() << Key;
 if(IndexGroup<0) return DefaultValue;
 int pos;
 QString str,Value="";
 for(int i=IndexGroup+1;i<Line.count();i++)
 {
  str=Line.at(i);
  if(str[0]=='[') break;
  pos=str.indexOf('=');
  //qDebug() << str.left(pos) << Key;
  if(pos!=-1 && str.left(pos).toUpper()==Key)
  {
   if(!Value.isEmpty()) Value+="\n";
   Value+=str.right(str.length()-pos-1);
  }
 }

 if(Type==vtBoolean)
 {
  QString def=Value.toLower();
  if(def=="true" || def=="yes") Value="1";
  if(def=="false" || def=="no") Value="0";
 }

 if(!Value.isEmpty())
      return Value;
 else return DefaultValue;
}

QString INISettings::valueNext()
{
 if(EndKey) return "";

 QString Value="",str;
 int pos=-1;
 while(pos==-1)
 {
  str=Line.at(IndexValue);
  pos=str.indexOf('=');
  if(pos!=-1) Value=str.mid(pos+1,str.length());
  IndexValue++;
 }
 if(IndexValue>Line.count()-1) EndKey=1;
 else
 if(QString(Line.at(IndexValue))[0]=='[') EndKey=1;
 return Value;
}

QStringList INISettings::toStringList(QString Value)
{
 QString str;
 QStringList list=Value.split(SEPARATOR);
 int QuoterLength=QuoterValue.length();
 bool hasQuoter=(Value.indexOf(QuoterValue)!=-1);
 for(int i=0;i<list.count();i++)
 {
  str=QString(list.at(i)).trimmed();
  if(hasQuoter)
  if(str.left(QuoterLength)==QuoterValue && str.right(QuoterLength)==QuoterValue)
  {
   str=str.mid(QuoterLength,str.length()-2*QuoterLength);
   //qDebug() << str;
  }
  list.replace(i,str);
 }
 return list;
}

QStringList INISettings::valueToStringList(QString Key)
{
 return toStringList(value(Key));
}

void INISettings::setDelimiter(const QString Delimiter)
{
 DelimiterValue=Delimiter;
 SEPARATOR.setPattern(PATTERN.arg(DelimiterValue).arg(QuoterValue));
}
void INISettings::setQuoter(const QString Quoter)
{
 QuoterValue=Quoter;
 SEPARATOR.setPattern(PATTERN.arg(DelimiterValue).arg(QuoterValue));
}
