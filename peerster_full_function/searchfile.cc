#include "searchfile.hh"

void SearchFile::clearSResultFiles()
{
    //@@@ now
    this->sResultFiles->clear();
}

SearchFile::SearchFile()
{
    //sRequestReplied = new QHash <QString, QVariantMap>();
    sResultFiles = new  QHash <QByteArray, QPair <QString, QString > > ();
}


void SearchFile::setCurrRequest(QVariantMap request)
{
    myCurrRequest = request;
}

QVariantMap SearchFile::getCurrentRequest()
{
    return myCurrRequest;
}

bool SearchFile::replyIsForCurrRequest(QString keyWord)
{
    bool rtn;
    QString currSearch = myCurrRequest.value("Search").toString();
    if (keyWord == currSearch )
    {
        rtn = true;
    }
    else
    {
        rtn = false;
    }
    return rtn;
}

// check id
bool SearchFile::isNewResult(QByteArray fileID)
{
    bool contains = sResultFiles->contains(fileID);
    if (TRUE == contains)
    {
        return false;
    }
    else
    {
        return true;
    }
}

QString SearchFile::getNameFromID(QVariantList nameList, QVariantList idList, QByteArray id)
{
    int findex = idList.indexOf(id);
    QString name = nameList.at(findex).toString();
    return name;
}

void SearchFile::insertInSResultFiles(QByteArray id, QString name, QString origin)
{
    QPair <QString, QString> value;
    value.first = name;
    value.second = origin;
    sResultFiles->insert(id, value);
}


QByteArray SearchFile::getFileIDFromName(QString name)
{
    QList <QByteArray> keys = sResultFiles->keys();
    foreach (QByteArray k, keys)
    {
        QPair<QString, QString> p = sResultFiles->value(k);
        if (name == p.first)
        {
            return k;
        }
    }
    return NULL;

}

QString SearchFile::getFileOriginFromID(QByteArray fileid)
{
    QPair<QString, QString> p = sResultFiles->value(fileid);
    return p.second;
}

QList<QByteArray> SearchFile::getSearchResultID()
{
    QList<QByteArray> keys = this->sResultFiles->keys();
    return keys;
}

QString SearchFile::getNameFromID_sResultFiles(QByteArray fid)
{
    return this->sResultFiles->value(fid).first;
}

bool SearchFile::isFileCurrentResult(QByteArray fid)
{
    return this->sResultFiles->contains(fid);
}
