#ifndef RECOMMEND_HH
#define RECOMMEND_HH



#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QTextEdit>
#include <QHash>
#include <QPair>
#include <QHostAddress>

#define REC_S (0.2)  // initial node trust score
#define REC_ALPHA (5)    // alpha
#define REC_BETA (0)    // beta
#define THRESHOLD_C (1)    // threshold

class Recommend
{
public:
    Recommend();
    void setMyOriginName(QString name);
    bool hasNode(QString origin);
    void insertNewNode(QString origin);
    void updateTrust(QString origin, float trustScore);
    void trust_times_alpha(QString origin);
    void trust_times_beta(QString origin);
    float getTrustScore(QString origin);
    float computeFileReputation(QList<QString> plist);
    bool isOverWhelming(float reputation);
    QHash <QString, float> getTrustRecords();

    void clearSearchResultReputation();
    void insertReputation(QByteArray fid, float reputation);
    bool hasReputation(QByteArray fid);
    float getReputation(QByteArray fid);
    QByteArray recommendFromSearchResult();

    void insertWhoLikeLocalList(QByteArray fid, QList<QString> plist);
    QList<QString> getWhoLikeFile(QByteArray fid);
    void clear_sr_WhoLikeLocalList();
    bool inLocalDislikeList(QByteArray fid);
    void insertDislikeFile(QByteArray fid);
    void insertLikeFile(QByteArray fid);
    bool inLocalLikeList(QByteArray fid);

private:
    QString myOriginName;
    // a list of nodes and their trust score <origin name, trust score>
    QHash <QString, float> *trustRecords; // <node name, trust score>, does not store my trust score
    QHash <QByteArray, float> *searchResultReputation; // <file name, reputation score>
    QHash <QByteArray, QList<QString> > *sr_WhoLikeLocalList; // <fileid, list of <node name> >
    QSet <QByteArray> *localLikeList; // <fileid>
    QSet <QByteArray> *localDislikeList; // <fileid> set of fileIDs that I voted bad
};

#endif // RECOMMEND_HH
