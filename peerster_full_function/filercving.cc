#include "filercving.hh"

FileRcving::FileRcving()
{
    this->blkNumWant = 0;
}


FileRcving::FileRcving(QByteArray fileID)
{
    this->blHash = fileID;
    this->blkNumWant = 0;
}

void FileRcving::receivedMeta(QByteArray meta)
{
    this->blMeta = meta;
    this->blkNumWant = 1;
}

void FileRcving::appendToRawFile(QByteArray block)
{
    this->rawfile.append(block);
    this->blkNumWant ++;
}

int FileRcving::getBlkNumWant()
{
    return this->blkNumWant;
}

QByteArray FileRcving::getMeta()
{
    return this->blMeta;
}

bool FileRcving::rawDataIsGood()
{
    //@@@ to do
    return true;
}

int FileRcving::endOfRcvThisFile()
{
    QString rand = QString::number(qrand() %  1000);
    this->fname = "wyyGotFile";
    this->fname.append(rand);
    // @@@check if the file received correct ???
    if (FALSE == rawDataIsGood())
    {
        return RTN_ERR;
    }
    // size
    QFile file (this->fname);
    file.open(QIODevice::WriteOnly);
    file.write(this->rawfile);
    file.close();
    // @@@ check the file???

    // save to folder, give it a name

    //@@@ do we need to save in the shared file list?
    return RTN_OK;
}
