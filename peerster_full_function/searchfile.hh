#ifndef SEARCHFILE_HH
#define SEARCHFILE_HH

#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QTextEdit>
#include <QHash>
#include <QPair>
#include <QHostAddress>

class SearchFile
{
public:
    SearchFile();
    void setCurrRequest(QVariantMap request);
    bool replyIsForCurrRequest(QString keyWord);
    bool isNewResult(QByteArray fileID);
    QString getNameFromID(QVariantList nameList, QVariantList idList, QByteArray id);
    void insertInSResultFiles(QByteArray id, QString name,QString origin);
    QByteArray getFileIDFromName(QString name);
    QString getFileOriginFromID(QByteArray fileid);
    QVariantMap getCurrentRequest();
    void clearSResultFiles();
    QList<QByteArray> getSearchResultID();
    QString getNameFromID_sResultFiles(QByteArray fid);
    bool isFileCurrentResult(QByteArray fid);

private:
    // request message received -for me
    // reply message received - for me
    QVariantMap myCurrRequest;
    //QHash <QString, QVariantMap> *sRequestReplied; // if I already replyed to the request
    QHash <QByteArray, QPair <QString, QString> > *sResultFiles; // < fileid, <filename, origin>>

};

#endif // SEARCHFILE_HH
