#ifndef FILEDB_HH
#define FILEDB_HH

#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QHash>
#include "file.hh"
#include "filercving.hh"


#define LIKE (1)
#define UNLIKE (0)


class FileDB
{
public:

    FileDB();
    //void insertFile(QByteArray blockListHash, File f);
    //void insertContent(QByteArray key, QByteArray content);
    QByteArray getContent(QByteArray key);
    QByteArray insertNewFile(QString name);
    bool allContentContains(QByteArray key);
    bool isFileID(QByteArray hash);
    void insertIntoFileHashes(QByteArray id, QByteArray content);
    QByteArray getFileIDfromRqstBlockID(QByteArray blockID);
    void insertBtoF(QByteArray blockID, QByteArray fileID);
    bool blockRequested(QByteArray blockID);
    bool fileNameRelateToKey(QString fname,QString k);
    QList<QByteArray> keysOfFileDB();
    QString getFileNameinFileDB(QByteArray fileID);

    File getFileFromID(QByteArray fileid);
    QByteArray getFileIDfromFileName(QString fileName);

    //dsybil
    bool likeOneFile(QByteArray fid, QString origin, QHostAddress ip, quint16 port ,int like);

private:
    // save the File corresponding to the blockListHash as key
    QHash <QByteArray, File> *fileDB;
    // save all the ids and content blocks in random order
    // including the blockListHash with value of blockListMeta
    QHash <QByteArray, QByteArray> *allContent;
    // store all blocklisthashes for files
    // if there is value, it means the file  is already in local
    //@@@ not yet updated when endof receivinga file
    QHash <QByteArray, QByteArray> *fileHashes;
    // for getting the requested fileID from blockID
    // <blockID, fileID>
    QHash <QByteArray, QByteArray> *blockToFile;

};

#endif // FILEDB_HH
