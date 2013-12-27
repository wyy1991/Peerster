#include "filedb.hh"
#include "QFile"

FileDB::FileDB()
{
    fileDB = new QHash <QByteArray, File>;
    allContent = new QHash <QByteArray, QByteArray>;
    fileHashes = new QHash <QByteArray, QByteArray>;
    blockToFile = new QHash <QByteArray, QByteArray> ;

}

bool FileDB::allContentContains(QByteArray key)
{
    return this->allContent->contains(key);
}


/*
void FileDB::insertContent(QByteArray key, QByteArray content)
{
    this->allContent->insert(key, content);
}
*/
/*
void FileDB::insertFile(QByteArray blockListHash, File f)
{
    this->fileDB->insert(blockListHash, f);
    //@@@ put the content into allContent
}
*/
// create a File and insert it into the fileDB
QByteArray FileDB::insertNewFile(QString name)
{
    //create new file
    File ff (name); // this only sets the file name and size

    int blockNum;
    int carry;
    quint64 file_size;
    QByteArray blockHash; // hash of block
    QByteArray block; // block content
    QByteArray blockListMeta_tmp;
    QByteArray blockListHash_tmp;

    QFile qfile (name);
    ff.setFileSize(qfile.size());
    file_size = ff.getFileSize();

    qfile.open(QIODevice::ReadOnly);
    // get total number of blocks
    carry = file_size % BLOCKSIZE;
    if (carry > 0)
    {
        blockNum = (file_size/BLOCKSIZE) + 1;
    }else
    {
        blockNum = file_size / BLOCKSIZE ;
    }
    qDebug() << "blocks num = " << blockNum;
    // read each block and do shaHash
    for (int i = 0; i < blockNum; i++)
    {
        // read each block and SHA-256
        // append into block list meta
        block = qfile.read(BLOCKSIZE);
        QCA::Hash shaHash ("sha256");
        blockHash = shaHash.hash(block).toByteArray();  // should be 32 bytes
        blockListMeta_tmp.append(blockHash);

        // save the block into allContent
        this->allContent->insert(blockHash, block);
    }
    // set sha hash
    ff.setBlockListMeta(blockListMeta_tmp);
    // get blocklisthash
    QCA::Hash shaHash2 ("sha256");
    blockListHash_tmp = shaHash2.hash(blockListMeta_tmp).toByteArray();
    ff.setBlockListHash(blockListHash_tmp);

    qfile.close();

    // insert into fileDB
    this->fileDB->insert(blockListHash_tmp, ff);
    this->allContent->insert(blockListHash_tmp, blockListMeta_tmp);
    this->fileHashes->insert(blockListHash_tmp, blockListMeta_tmp);

    return blockListHash_tmp;
}

QByteArray FileDB::getContent(QByteArray key)
{
    return this->allContent->value(key);
}
bool FileDB::isFileID(QByteArray hash)
{
    return this->fileHashes->contains(hash);
}

void FileDB::insertIntoFileHashes(QByteArray id, QByteArray content)
{
    this->fileHashes->insert(id, content);
}

QByteArray FileDB::getFileIDfromRqstBlockID(QByteArray blockID)
{
    QByteArray fileID = this->blockToFile->value(blockID);
    return fileID;
}

void FileDB::insertBtoF(QByteArray blockID, QByteArray fileID)
{
    this->blockToFile->insert(blockID, fileID);
}

bool FileDB::blockRequested(QByteArray blockID)
{
    return this->blockToFile->contains(blockID);
}

bool FileDB::fileNameRelateToKey(QString fname, QString k)
{
    bool rtn;
    int index = fname.indexOf(k);
    if (index >= 0)
    {
        rtn = true;
    }
    else
    {
        rtn = false;
    }
    return rtn;
}

QList<QByteArray> FileDB::keysOfFileDB()
{
    return this->fileDB->keys();
}

QString FileDB::getFileNameinFileDB(QByteArray fileID)
{
    File f = this->fileDB->value(fileID);
    return f.getFileName();
}

File FileDB::getFileFromID(QByteArray fileid)
{
    return this->fileDB->value(fileid);
}

QByteArray FileDB::getFileIDfromFileName(QString fileName)
{
    QList <QByteArray> keys = this->fileDB->keys();

    foreach (QByteArray k, keys)
    {
        File f = this->fileDB->value(k);
        //@@@ what is same file name??
        if (f.getFileName() == fileName)
        {
            return k;
        }
    }
    return NULL;
}


bool FileDB::likeOneFile(QByteArray fid, QString origin, QHostAddress ip, quint16 port ,int like)
{
    File file = this->getFileFromID(fid);
    bool success = false;
    if (LIKE == like)
    {
        success = file.likeFile(origin, ip, port);
        this->fileDB->insert(fid, file);
    }
    else if(UNLIKE == like)
    {
        success = file.unlikeFile(origin);
        this->fileDB->insert(fid,file);
    }

    return success;
}

