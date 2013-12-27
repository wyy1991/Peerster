#ifndef FILERCVING_HH
#define FILERCVING_HH


#include <QDebug>
#include <QVariant>
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QtCrypto>
#include <QHash>
#include "msgdb.hh"

class FileRcving
{
public:
    FileRcving();
    FileRcving(QByteArray fileID);
    void appendToRawFile(QByteArray block);
    void receivedMeta(QByteArray meta);

    int getBlkNumWant();
    QByteArray getMeta();
    int endOfRcvThisFile();

private:
    QString fname;
    quint64 fsize; // in bytes
    QByteArray blMeta;// block list metafile
    QByteArray blHash;// SHA-256 hash of the blocklist metafile
    QByteArray rawfile;
    int blkNumWant;

    bool rawDataIsGood();

};

#endif // FILERCVING_HH
