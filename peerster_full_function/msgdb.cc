#include "msgdb.hh"


MsgDB::MsgDB()
{
    database = new QMap < QString, QList< QVariantMap > > ;
    dbStatus = new QVariantMap;//<QString, QVariantMap>
    routingTable = new QHash <QString, QPair<QHostAddress, quint16> >;

    QVariantMap *wantMap = new QVariantMap();
    dbStatus->insert("Want", *wantMap);

}


QPair <QHostAddress, quint16> MsgDB::getRouteContent(QString origin)
{
    QPair<QHostAddress, quint16> address = this->routingTable->value(origin);
    return address;
}

QString MsgDB::getOriginName(QHostAddress ip, quint16 port)
{
    QList <QString> originList = this->routingTable->keys();
    foreach (QString origin, originList)
    {
        QPair< QHostAddress, quint16>  pair = this->routingTable->value(origin);
        if ((pair.first == ip) && (pair.second == port))
        {
            return origin;
        }
    }
    return NULL;
}

int MsgDB::insertRouteToTable(QString origin, QHostAddress addr, quint16 port)
{
    // insert route
    int rtn = ROUTE_INSERT;
    // check if already have the origin

    if (FALSE == routingTable->contains(origin))
    {
       rtn = ROUTE_INSERT;
    }
    else
    {
       rtn = ROUTE_UPDATE;
    }

    // if not, create new tupper, insert the value

    QPair<QHostAddress, quint16> pair(addr, port);
    routingTable->insert(origin, pair);  //@@@ does this overwrite???

    // if already contains the value,

    //@@@ check if it is the direct or indirect route ???

    // update the route, @@@ now just overwrite

    return rtn;

}

// part6, if it contains no LastIp and LastPort -- direct route
// if it is direct route, override the indirect one with the new direct one
int MsgDB::updateDirectRoute(QVariantMap rummorMsg, QHostAddress sender, quint16 senderPort)
{
    int rtn = RTN_NO_ROUTE_UPDATE;

    if ((FALSE == rummorMsg.contains("LastIP")) && (FALSE == rummorMsg.contains("LastPort")))
    {

        // @@@ check if the old route is indirect, see if it is the same as the coming one
        QString origin = rummorMsg.value("Origin").toString();
        QPair< QHostAddress, quint16> pair =this-> getRouteContent(origin);

        if ((pair.first != sender) || (pair.second != senderPort) )
        {
            QPair< QHostAddress, quint16> newpair (sender, senderPort);
            this->routingTable->insert(origin,newpair);
            rtn = RTN_OK;
        }
    }
    return rtn;
}

QStringList MsgDB::outputRouteTable()
{
    QStringList strlist("Route Table");
    QString str;
    foreach (QString k, this->routingTable->keys())
    {
        str.clear();
        str.append(k);
        str.append(" ");
        str.append(routingTable->value(k).first.toString());
        str.append(" : ");
        str.append(QString::number(routingTable->value(k).second));

        strlist.append(str);
    }
    return strlist;
}

int MsgDB::checkMsginDBStatus(QVariantMap msgMap)
{
    // get message Origin and SeqNo
    QString origin = msgMap.value("Origin").toString();
    quint32 seq = msgMap.value("SeqNo").toUInt(0);

    // check if the dbStatus is empty,
    // if empty, return receive
    if (FALSE == dbStatus->contains("Want"))
    {
        if (1 == seq)
        {
            return MSG_ST_EXACTLY_NEEDED;
        }
        else
        {
            return MSG_ST_DB_EMPTY;
        }
    }
    // get see if the dbStatus has the origin
    QVariantMap tmpMap = dbStatus->value("Want").toMap();
    if (FALSE == tmpMap.contains(origin))
    {
        if (1 == seq)
        {
            return MSG_ST_EXACTLY_NEEDED;
        }
        else
        {
            return MSG_ST_DB_NO_SUCH_ORIGIN;
        }
    }

    // check if the origin value is equal to
    quint32 seqInDb = tmpMap.value(origin).toUInt(); // seqNo in db for the origin
    if (seq == seqInDb)
    {
        return MSG_ST_EXACTLY_NEEDED;
    }
    else if (seq < seqInDb)
    {
        return MSG_ST_ALREADY_RECEIVED;
    }
    else
    {
        return MSG_ST_RECEIVE_LATER;
    }

}

//-----------insertMsg----------------------------------------
// return error code, 0 ok
// Assume message input is valid. insert without check
// include update status
int MsgDB::insertMsg(QVariantMap singleMsg)
{
    QString msgCText;
    QString msgOrigin;
    quint32 msgSeq;

    QList< QVariantMap > *msgList;
    QVariant msgListVar;

    //@@@ assume message is valid
    // msgCText = singleMsg.value("ChatText").toString();  // no ChatText for route msg
    msgOrigin = singleMsg.value("Origin").toString();
    msgSeq = singleMsg.value("SeqNo").toUInt(0);

    // look for the list for the specific origin name
    // if no such list, create new list for it
    if (FALSE == database->contains(msgOrigin))
    {
        msgList = new QList<QVariantMap>();
        database->insert(msgOrigin, *msgList);
    }

    // add into the list, in the right place due to sequence
    QList<QVariantMap> datalist = database->value(msgOrigin);
    datalist.append(singleMsg);
    database->insert(msgOrigin,datalist);
    //qDebug() << "Insert message: " << msgOrigin << " Seq=" << msgSeq;

    // update dbStatus
    insertDbStatus(msgOrigin,msgSeq);
    return RTN_OK;
}

//-----------insertDbStatus----------------------------------------
int MsgDB::insertDbStatus(QString origin, quint32 seq)
{
    QVariantMap *statusMap;
    quint32 seqNumWanted;
    // check if it is empty
    // if empty, insert <"Want", <origin, seq> >
    if (FALSE == dbStatus->contains("Want"))
    {
        statusMap = new QVariantMap();
        statusMap->insert(origin,(quint32)1);
        dbStatus->insert("Want", *statusMap);
    }
    //if does not contain the origin, insert the <origin, 1>
    QVariantMap tmpMap = dbStatus->value("Want").toMap(); //copy to tmpMap
    if (FALSE == tmpMap.contains(origin))
    {
        tmpMap.insert(origin,(quint32)1);
        dbStatus->insert("Want", tmpMap);
    }

    // Start to check for update:
    // if seq number is equal to wanted,
    // wanted sequence num + 1.
    // else do not change wanted seq num
    // The type should be alright, since it is not from outside

    seqNumWanted = tmpMap.value(origin).toUInt(0);
    if (seq == seqNumWanted)
    {
        seqNumWanted++;
        tmpMap.insert(origin,seqNumWanted);
    }
    dbStatus->insert("Want",tmpMap);

    //qDebug() << "Update Status for: " << origin << " Seq=" << seqNumWanted;

    return RTN_OK;
}


//-------------compareStatusMap-----------------------------------------
// return:  QMap <"NextToGiveTarget", <origin, seq> >
//               <"NextNeedFromTarget", <origin, seq> >
// assume input is valid status map
QMap<QString, QVariantMap> MsgDB::compareStatusMap(QVariantMap stMap)
{
    QMap<QString, QVariantMap> *diffMap = new QMap<QString, QVariantMap>();
    QVariantMap *mapGive = new QVariantMap();
    QVariantMap *mapNeed = new QVariantMap();

    // @@@ what if my dbStatus does not have "Want" now

    QVariantMap localWantMap = dbStatus->value("Want").toMap();
    QVariantMap targetWantMap = stMap.value("Want").toMap();

    QList<QString> lKeys = localWantMap.keys();
    QList<QString>::iterator it;
    QString key;

    for(it = lKeys.begin(); it != lKeys.end(); it++)
    {
        key = *it;
        if (FALSE == targetWantMap.contains(key))
        {
            // target does not have key, add <key, 1>
            // add it to the toGiveTarget map
            mapGive->insert(key, 1);
        }
        else
        {
            // target have key
            if (localWantMap.value(key).toUInt(0) >
                    targetWantMap.value(key).toUInt(0))
            {
                //add to the giveTarget Map, we can give what target want
                mapGive->insert(key, targetWantMap.value(key));
            }
            else if (localWantMap.value(key).toUInt(0) <
                     targetWantMap.value(key).toUInt(0))
            {
                // add to needFromTarget Map, target can give what we want
                mapNeed->insert(key, localWantMap.value(key));
            }
            else
            {
                // equal do nothing
            }
        }
        // remove in both list
        localWantMap.remove(key);
        targetWantMap.remove(key);
    }
    // now what is left are, what target have and we do not have
    QList<QString> tKeys = targetWantMap.keys();
    for (it = tKeys.begin(); it != tKeys.end(); it++)
    {
        key = *it;
        //add them all to needList, with value 1
        mapNeed->insert(key, 1);
        targetWantMap.remove(key);
    }
    // now both WantMap should be empty,
    // give and need list contains the next value we can give or ask

    // check both map, insert if not empty
    // put these into the map we want to return, diffMap
    if (FALSE == mapGive->empty())
    {
        diffMap->insert("NextToGiveTarget", *mapGive);
    }
    if (FALSE == mapNeed->empty())
    {
        diffMap->insert("NextNeedFromTarget",*mapNeed);
    }

    return *diffMap;
}


//-----------updateDbStatus----------------------------------------
void MsgDB::updateDbStatus()
{


}
//-----------getDataBase----------------------------------------
QMap < QString, QList< QVariantMap > > * MsgDB::getDataBase()
{
    return this->database;

}
//-----------getDBStatus----------------------------------------
QVariantMap MsgDB::getDBStatus()
{
    return *(this->dbStatus);
}

void MsgDB::printStatusMessage(QTextEdit * textView)
{
    QString output;

    // @@@ to do , TESTCODE
    if (FALSE == this->dbStatus->contains("Want"))
    {
        output.append("empty.");
    }
    else
    {
        QVariantMap wantMap = this->dbStatus->value("Want").toMap();
        QList<QString> keys = wantMap.keys();
        foreach(QString k, keys)
        {
            output.append("<");
            output.append(k);
            output.append(", ");
            output.append(wantMap.value(k).toString());
            output.append("> \n ");

        }
    }
    output.append("---------");
    //textView->append(output);
    qDebug() << output;
}

//-----------getSeqNo----------------------------------------
// insert a socket name, return the sequence number going to
// need to insert into the socket
quint32 MsgDB::getSeqNo(QString originName)
{
    // check dbStatus
    // if no "Want", return 1 (SEQ_NUM_START);
    if (FALSE == dbStatus->contains("Want"))
    {
        return SEQ_NUM_START;
    }
    // look at the value for "Want"
    // if no socketName, return 1
    QVariantMap tmpMap = dbStatus->value("Want").toMap();
    if (FALSE == tmpMap.contains(originName))
    {
        return SEQ_NUM_START;
    }
    quint32 seqNumber = tmpMap.value(originName).toUInt(0);
    // if has socketName return seq number of socketName
    return seqNumber; //@@@ to do
}
