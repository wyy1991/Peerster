#include "file.hh"


File::File()
{

}

File::File(QString name)
{
    QString nameNoPath;
    QStringList list = name.split('/');
    int size = list.size();
    nameNoPath = list.at(size-1);
    numOfLike = 0;
    peerLiked = new QHash < QString, QPair< QHostAddress, quint16> > ;
    this->filename  = nameNoPath;

}


//@@@ nothing is added for now to distinguish from the above function
/*
File::File(QString name, QString nothing)
{
    int blockNum;
    int carry;
    QByteArray blockHash;

    this->filename = name;
    //qDebug() << "create file:" << name;

    file  = new QFile(name);
    file->open(QIODevice::ReadOnly | QIODevice::Text);

    this->filesize = file->size();
    QByteArray block;
    // get total number of blocks
    carry = filesize % BLOCKSIZE;
    if (carry > 0)
    {
        blockNum = (filesize/BLOCKSIZE) + 1;
    }else
    {
        blockNum = filesize / BLOCKSIZE ;
    }
    qDebug() << "blocks num = " << blockNum;
    // read each block and do shaHash
    for (int i = 0; i < blockNum; i++)
    {
        // read each block and SHA-256
        block = file->read(BLOCKSIZE);
        QCA::Hash shaHash ("sha256");
        blockHash = shaHash.hash(block).toByteArray();  // should be 32 bytes
        // save into block list
        this->blockListMeta.append(blockHash);
    }

    QCA::Hash shaHash2 ("sha256");
    this->blockListHash = shaHash2.hash(this->blockListMeta).toByteArray();
    qDebug() << "blockListHash:" << blockListHash;
    // create
    QByteArray bkListHashHex = this->blockListHash.toHex();
    qDebug() << "File hex:" << bkListHashHex;

    file->close();
}
*/

QByteArray File::getBlockListHash()
{
    return this->blockListHash;
}

QByteArray File::getBlockListMeta()
{
    return this->blockListMeta;
}

quint64 File::getFileSize()
{
    return this->filesize;
}

QString File::getFileName()
{
    return this->filename;
}

void File::setBlockListMeta(QByteArray meta)
{
    this->blockListMeta = meta;
}
void File::setBlockListHash(QByteArray h)
{
    this->blockListHash = h;
}

void File::setFileSize(quint64 s)
{
    this->filesize = s;
}

QList<QString> File::getPeerLikedKeys()
{
    return this->peerLiked->keys();
}

int File::getNumOfLike()
{
    return this->numOfLike;
}

bool File::likeFile(QString origin, QHostAddress ip, quint16 port)
{
    // check if peer is in the peerlikedlist
    if (true == this->peerLiked->contains(origin))
    {
        // peer already in the file's likedlist
        return false;
    }
    // add in the list and increase num of like
    //@@@ check if need "new"
    QPair<QHostAddress, quint16> *addressPair = new QPair<QHostAddress, quint16>();
    addressPair->first = ip;
    addressPair->second = port;
    this->peerLiked->insert(origin, *addressPair);
    numOfLike ++;
    return true;
}

bool File::unlikeFile(QString origin)
{
    if (false == this->peerLiked->contains(origin))
    {
        return false; // not in liked list
    }
    this->peerLiked->remove(origin);
    numOfLike --;
    return true;
}
