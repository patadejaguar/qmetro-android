#ifndef INISETTINGS_H
#define INISETTINGS_H

#include "QString"
#include "QStringList"
#include "QTextCodec"

enum ValueType{vtString,vtBoolean};

class INISettings
{
public:

    INISettings(const QString FileName,QTextCodec *TextCodec);
    ~INISettings();

    void beginGroup(QString GroupName);
    void endGroup(){ IndexGroup=0; }

    QString value(QString Key,const QString DefaultValue="",ValueType Type=vtString);
    QString valueNext();
    QStringList valueToStringList(QString Key);
    QStringList toStringList(QString Value);


    void setDelimiter(const QString Delimiter);
    void setQuoter(const QString Quoter);

    QString getDelimiter(){ return DelimiterValue; }
    QString getQuoter(){ return QuoterValue; }
    bool atEndKey(){ return EndKey; }
    //bool atEndGroup(){ return EndGroup; }

    QStringList Line;


private:
    int findGroup(const QString GroupName);
    void findGroupInKey(QString &Key,const QString Slash);

    QString DelimiterValue, QuoterValue;
    int IndexGroup,IndexValue;
    bool EndKey;//,EndGroup;

};

#endif // INISETTINGS_H
