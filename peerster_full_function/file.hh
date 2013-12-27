#ifndef FILE_HH
#define FILE_HH

#include <QDebug>
#include <QVariant>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QtCrypto>
#include "peer.hh"

#define BLOCKSIZE (1024 * 8)
#define SHA_HASH_LENGTH (32)

class File
{

public:
    File();
    File(QString name);
    //@@@ File(QString name, QString nothing);
    QByteArray getBlockListHash();
    QByteArray getBlockListMeta();
    quint64 getFileSize();
    QString getFileName();
    void setBlockListMeta(QByteArray meta);
    void setBlockListHash(QByteArray h);
    void setFileSize(quint64 s);
    bool likeFile(QString origin, QHostAddress ip, quint16 port);
    bool unlikeFile(QString origin);
    QList<QString> getPeerLikedKeys();
    int getNumOfLike();

private:
    QString filename;
    quint64 filesize; // in bytes
    QByteArray blockListMeta;// block list metafile
    QByteArray blockListHash;// SHA-256 hash of the blocklist metafile

    int numOfLike;  // number of peers that liked
    //QList<Peer> *peerLiked; // list of peers that liked this file
    QHash < QString, QPair< QHostAddress, quint16> > *peerLiked;

};

#endif // FILE_HH
