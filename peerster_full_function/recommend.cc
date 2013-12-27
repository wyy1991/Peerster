#include "recommend.hh"

Recommend::Recommend()
{
    trustRecords = new QHash <QString, float>();
    searchResultReputation = new QHash <QByteArray, float>();
    localLikeList = new QSet <QByteArray>();
    localDislikeList = new QSet <QByteArray>();
    sr_WhoLikeLocalList = new QHash <QByteArray, QList<QString> > ;
}

void Recommend::setMyOriginName(QString name)
{
    this->myOriginName = name;
}
void Recommend::insertWhoLikeLocalList(QByteArray fid, QList<QString> plist)
{
    this->sr_WhoLikeLocalList->insert(fid, plist);
}

QList<QString> Recommend::getWhoLikeFile(QByteArray fid)
{
    return this->sr_WhoLikeLocalList->value(fid);
}

void Recommend::clear_sr_WhoLikeLocalList()
{
    this->sr_WhoLikeLocalList->clear();
}

bool Recommend::hasNode(QString origin)
{
    if (TRUE == this->trustRecords->contains(origin))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Recommend::insertNewNode(QString origin)
{
    if (origin != this->myOriginName)
    {
        this->trustRecords->insert(origin, REC_S);
    }
}

void Recommend::updateTrust(QString origin, float trustScore)
{
    if (TRUE == this->trustRecords->contains(origin))
    {
        this->trustRecords->insert(origin, trustScore);
    }
}

void Recommend::trust_times_alpha(QString origin)
{
    float trust = this->trustRecords->value(origin);
    trust *= REC_ALPHA;
    this->trustRecords->insert(origin, trust);
}

void Recommend::trust_times_beta(QString origin)
{
    float trust = this->trustRecords->value(origin);
    trust *= REC_BETA;
    this->trustRecords->insert(origin, trust);
}

float Recommend::getTrustScore(QString origin)
{
    return this->trustRecords->value(origin);
}

// compute the files reputation
float Recommend::computeFileReputation(QList<QString> plist)
{
    // compute the sum of the trust score for each node,
    float sum = 0.0;
    float trust;
    foreach (QString p, plist)
    {
        // check if it is in the list
        if (TRUE == this->hasNode(p))
        {
            trust = this->getTrustScore(p);
            sum += trust;
        }
        else
        {
            qDebug() << "Node not in the local trust list."<< endl;
        }
    }
    return sum;
}

bool isOverWhelming(float reputation)
{
    if (reputation > THRESHOLD_C)
    {
        return true;
    }else
    {
        return false;
    }
}

QHash <QString, float> Recommend::getTrustRecords()
{
    return *this->trustRecords;
}


void Recommend::clearSearchResultReputation()
{
    this->searchResultReputation->clear();
}

void Recommend::insertReputation(QByteArray fid, float reputation)
{

    this->searchResultReputation->insert(fid, reputation);
}

bool Recommend::hasReputation(QByteArray fid)
{
    if (TRUE == this->searchResultReputation->contains(fid))
    {
        return true;
    }
    else
    {
        return false;
    }
}

float Recommend::getReputation(QByteArray fid)
{
    return this->searchResultReputation->value(fid);
}

QByteArray Recommend::recommendFromSearchResult()
{
    QByteArray fileIDPicked;
    int random;
    // check all the searchResultReputation, find if they above or below C
    QList<QByteArray> overwhelmFiles;
    QList<QByteArray> nonoverwhelmFiles;

    QList<QByteArray> keys = searchResultReputation->keys();
    foreach(QByteArray k, keys)
    {
        if (this->searchResultReputation->value(k) >= THRESHOLD_C)
        {
            overwhelmFiles.append(k);
        }
        else
        {
            nonoverwhelmFiles.append(k);
        }
    }
    // if some are above c, select one from those
    int sizeOver = overwhelmFiles.size();
    int sizeUnder = nonoverwhelmFiles.size();
    if (sizeOver > 0)
    {
        random = qrand() %  sizeOver;
        fileIDPicked = overwhelmFiles.at(random);
    }
    else if (sizeUnder > 0)
    {
        random = qrand() % sizeUnder;
        fileIDPicked = nonoverwhelmFiles.at(random);
    }
    else
    {
        fileIDPicked = (QByteArray) 0;
    }
    // if all below c, randomly recommend one
    return fileIDPicked;


}

bool Recommend::inLocalDislikeList(QByteArray fid)
{
    return this->localDislikeList->contains(fid);
}

void Recommend::insertDislikeFile(QByteArray fid)
{
    this->localDislikeList->insert(fid);
}
void Recommend::insertLikeFile(QByteArray fid)
{
    this->localLikeList->insert(fid);
}
bool Recommend::inLocalLikeList(QByteArray fid)
{
    return this->localLikeList->contains(fid);
}
