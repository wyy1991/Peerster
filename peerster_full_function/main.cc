
// /c/cs426/runqemu.sh vm1.img &
// scp -r yw384@node.zoo.cs.yale.edu:./Documents/peerster .
// # !/bin/sh


// @@@ current problem, check if the search reply is new,
// seems it does not accept files that has been found by search before
#include "main.hh"
#include "msgdb.hh"
#include "peer.hh"
#include "file.hh"
#include "filedb.hh"
#include "searchfile.hh"
#include "recommend.hh"



/**********************************************************************
 * Class Name: ChatDialog
 */
ChatDialog::ChatDialog()
{

    checkRecServerMode();
    checkRecServerMode();

    //-------------Layout--------------------------------------------
	setWindowTitle("Peerster");

	// Read-only text box where we display messages from everyone.
    // This widget expands both horizontally and vertically.
    textview = new QTextEdit;
    textview->setReadOnly(true);

    // new text view showing the peers
    textPeers = new QTextEdit;
    textPeers->setReadOnly(true);

	// Small text-entry box the user can enter messages.
	// so that the user can easily enter multi-line messages.
    textline = new QTextEdit(this);
    textline->setReadOnly(false);
    textline->installEventFilter(this);

    buttonPrivateCon = new QPushButton("Start Private Msg");
    buttonAddPeer = new QPushButton("Add neighbour");
    inputPeer = new QLineEdit();
    QLabel *label_peer = new QLabel(this);
    label_peer->setText("Host:Port");

    // list view
    // listview showing the route table
    listWidget = new QListWidget(this);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Share file
    buttonShareFile = new QPushButton("Share File");
    buttonRequestFile = new QPushButton("Download File");
    textTargetID = new QTextEdit();
    textTargetID->setReadOnly(false);
    textFileID = new QTextEdit();
    textFileID->setReadOnly(false);
//    QLabel *label_target = new QLabel(this);
//    label_target->setText("Target node ID:");
//    QLabel *label_file_id = new QLabel(this);
//    label_file_id->setText("File ID:");

    // Search layout
    lineKeyW = new QLineEdit();
    QLabel *label_searchResult = new QLabel(this);
    label_searchResult->setText("Search result:");
    buttonSearch = new QPushButton("Search");
    searchResult = new QListWidget(this);
    searchResult->setSelectionMode(QAbstractItemView::SingleSelection);

    // recommendation server
    // button share file
    // button file list, with the number of likes
    // list showing nodes that liked this node
    QLabel *label_server = new QLabel(this);
    label_server->setText("Server Mode");
    buttonShare = new QPushButton("Share");
    listFileShared = new QListWidget(this);
    listFileShared->setSelectionMode(QAbstractItemView::SingleSelection);
    textVotedClients = new QListWidget();
    textVotedClients->setSelectionMode(QAbstractItemView::SingleSelection);

    // recommendation user
    // search file
    // list showing the file, and the rec score, separated by threshold, like it by double click
    // text view showing the local trust score for each node
    buttonSetServer = new QPushButton("Add server");
    QLabel *label_client = new QLabel(this);
    label_client->setText("Local trust score");
    listNodeTrust = new QListWidget(this);
    buttonLike = new QPushButton("Like");
    buttonWhoLike = new QPushButton("Who liked");
    buttonDislike = new QPushButton("Dislike");
    listNodeTrust->setSelectionMode(QAbstractItemView::SingleSelection);

    // Lay out the widgets to appear in the main window.
	// For Qt widget and layout concepts see:
    // http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
    QHBoxLayout *mainHLayout = new QHBoxLayout(); // outside

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(textview);
    layout->addWidget(textline);
    layout->addWidget(label_peer);
    layout->addWidget(inputPeer);
    //layout->addWidget(buttonAddPeer);
    layout->addWidget(buttonSetServer);
    //layout->addWidget(textPeers,1,1);
    layout->addWidget(listWidget);
    //layout->addWidget(buttonPrivateCon,2,1);

    QVBoxLayout *fileLayout = new QVBoxLayout(); // for file share
//    fileLayout->addWidget(buttonShareFile);
//    fileLayout->addWidget(label_target);
//    fileLayout->addWidget(textTargetID);
//    fileLayout->addWidget(label_file_id);
//    fileLayout->addWidget(textFileID);
//    fileLayout->addWidget(buttonRequestFile);

    QVBoxLayout *searchLayout = new QVBoxLayout();
    searchLayout->addWidget(lineKeyW);
    searchLayout->addWidget(buttonSearch);
    searchLayout->addWidget(label_searchResult);
    searchLayout->addWidget(searchResult);
    searchLayout->addWidget(buttonLike);
    searchLayout->addWidget(buttonDislike);
//    searchLayout->addWidget(buttonWhoLike);


    //recommendation server,
    QVBoxLayout *recServerLayout = new QVBoxLayout();
    recServerLayout->addWidget(label_server);
    recServerLayout->addWidget(buttonShare);
    recServerLayout->addWidget(listFileShared);
    recServerLayout->addWidget(textVotedClients);
    //recommendation client
    QVBoxLayout *recClientLayout = new QVBoxLayout();
    recClientLayout->addWidget(label_client);
    recClientLayout->addWidget(listNodeTrust);


    mainHLayout->addLayout(layout);
    //mainHLayout->addLayout(fileLayout);
    mainHLayout->addLayout(searchLayout);
    mainHLayout->addLayout(recClientLayout);
    mainHLayout->addLayout(recServerLayout);


    setLayout(mainHLayout);

    //set focus when dialog appear
    textline->setFocus();

    //-------------Socket--------------------------------------------
    netSocket = new NetSocket();
    // init socket
    if (!netSocket->bind())
    {
        exit(1);
    }
    QHostInfo hostInfo = QHostInfo::fromName(QHostInfo::localHostName());
    QList<QHostAddress> hostList = hostInfo.addresses();
    QHostAddress myhost = hostList.at(0);
    QString showLocal = "Local: < ";
    showLocal.append(myhost.toString());
    showLocal.append(":");
    showLocal.append(QString::number(this->netSocket->getLocalPortNumber()));
    showLocal.append("> Origin: ");
    showLocal.append(this->netSocket->getSocketOriginName());
    textview->append(showLocal);

    //-------------forward mode---------------------------------------------
    noForwardMode = FALSE;
    //-------------pDialog---------------------------------------------
    pDialog = new PrivateD();
    //-------------fileDB---------------------------------------------
    fileDBlocal = new FileDB();
    //-------------FileRcving hash table---------------------------------------------
    receivingFiles = new QHash <QByteArray, FileRcving>();
    //-------------SearchFile, store the current search status---------------------------------------------
    searchStatus = new SearchFile();
    //-------------MsgDB---------------------------------------------
    msgDatabase = new MsgDB();
    //-------------peerList---------------------------------------------

    peerList = new QList<Peer>();
    initPeerList(); // set after bind local socket
    showPeers();

    //-------------Timer---------------------------------------------
    timer_anti_entropy = new QTimer(this);
    timer_route_rumor = new QTimer(this);
    timer_search = new QTimer(this);

    //---------recommendation--------------------------------------------
    serverAddress = new QPair <QHostAddress, quint16>;
    trustRec = new Recommend();
    trustRec->setMyOriginName(this->netSocket->getSocketOriginName());
    //-------------Event---------------------------------------------
	// Register a callback on the textline's returnPressed signal
	// so that we can send the message entered by the user.

    connect(netSocket, SIGNAL(readyRead()),this, SLOT(readPendingDatagrams()));
    connect(timer_anti_entropy, SIGNAL(timeout()), this, SLOT(timeOutAntiEntropy()));
    connect(timer_route_rumor,SIGNAL(timeout()),this, SLOT(timerRouteRumorHandler()));
    connect(timer_search,SIGNAL(timeout()),this, SLOT(timeOutRepeatSearch()));
    connect(buttonAddPeer, SIGNAL(clicked()),this, SLOT(addNewNeighbour()));
    connect(buttonPrivateCon,SIGNAL(clicked()),this, SLOT(startPrivateCon()));
    connect(buttonShareFile,SIGNAL(clicked()),this, SLOT(shareFileClicked()));
    connect(buttonRequestFile,SIGNAL(clicked()),this, SLOT(downloadFileClicked()));
    connect(buttonSearch,SIGNAL(clicked()),this, SLOT(searchButtonClicked()));

    connect(searchResult, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(doubleClickedSearchResult(QListWidgetItem*))); // something clicked
    connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(doubleClickedWidgetItem(QListWidgetItem*))); // something clicked
    connect(listWidget,
              SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
              this, SLOT(selectedChanged(QListWidgetItem *))); // currentItem changed
    connect(this->pDialog, SIGNAL(gotPrivateInput(QString, QString)),this, SLOT(gotTextFromPrivate(QString, QString)) );

    // recommend
    connect(buttonShare,SIGNAL(clicked()),this, SLOT(serverShareClicked()));
    connect(listFileShared, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(clickedServerSharedFile(QListWidgetItem*))); // something clicked
    connect(buttonSetServer, SIGNAL(clicked()),this, SLOT(addNewNbAndSetServer())); // add peer and set to server
    connect(buttonLike, SIGNAL(clicked()),this, SLOT(likeClicked()));
    connect(buttonDislike, SIGNAL(clicked()),this, SLOT(dislikeClicked()));
    connect(buttonWhoLike, SIGNAL(clicked()),this, SLOT(wholikeClicked()));

    timer_anti_entropy->start(10000); // 10 s
    timer_route_rumor->start(10000);   //@@@ should be 1 min

    // when start the send route to all neighbours, to init table
    QVariantMap routeRummor = createNewRouteRummor();
    sendRouteToAllPeers(routeRummor);

    checkNoForwardMode();

}



void ChatDialog::likeClicked()
{
    // check if any value selected
    if (FALSE == this->searchResult->selectedItems().isEmpty())
    {
        QString selected = this->searchResult->selectedItems().first()->text();
        QStringList list = selected.split('\t');
        QString selectedName = list.at(0);
        QByteArray fileid = this->searchStatus->getFileIDFromName(selectedName);

        // @@@ to do, locally update the trust scores
        // is it already been like?
        // like the file, add the fileid into dislike set
        if (TRUE == this->trustRec->inLocalLikeList(fileid))
        {
            this->textview->append("Already liked!");
        }
        else if (TRUE == this->trustRec->inLocalDislikeList(fileid))
        {
            this->textview->append("Already disliked!");
        }
        else
        {
            // if the file's reputation is larger than threshold C
            float fileRep = this->trustRec->getReputation(fileid);
            if (fileRep < THRESHOLD_C)
            {
                // who liked the file?
                // the file should be in the current search
                QList <QString> peers_liked = this->trustRec->getWhoLikeFile(fileid);
                // for every file that liked for this file,
                // time their reputation with beta
                foreach(QString peer, peers_liked)
                {
                    this->trustRec->trust_times_alpha(peer);
                }
                this->trustRec->insertLikeFile(fileid);
                this->updateLocalTrustList();
            }
            // if overwhelm, do nothing

            //@@@ to do find the file
            // send message to the server to like the file
            likeFile(fileid);
        }
    }else{
        this->textview->append("No file selected for like!");
    }
}


void ChatDialog::dislikeClicked()
{
    // check if any value selected
    if (FALSE == this->searchResult->selectedItems().isEmpty())
    {
        QString selected = this->searchResult->selectedItems().first()->text();
        QStringList list = selected.split('\t');
        QString selectedName = list.at(0);
        QByteArray fileid = this->searchStatus->getFileIDFromName(selectedName);

        // is it already been disliked?
        // dislike the file, add the fileid into dislike set
        if (TRUE == this->trustRec->inLocalLikeList(fileid))
        {
            this->textview->append("Already liked!");
        }
        else if (TRUE == this->trustRec->inLocalDislikeList(fileid))
        {
            this->textview->append("Already disliked!");
        }
        else
        {
            // who liked the file?
            // the file should be in the current search
            QList <QString> peers_liked = this->trustRec->getWhoLikeFile(fileid);
            // for every file that liked for this file,
            // time their reputation with beta
            foreach(QString peer, peers_liked)
            {
                this->trustRec->trust_times_beta(peer);
            }
            // insert into local dislike list
            this->trustRec->insertDislikeFile(fileid);
            this->updateLocalTrustList();
        }

    }else{
        this->textview->append("No file selected for dislike!");
    }
}

void ChatDialog::wholikeClicked()
{
    // check if any value selected
    if (FALSE == this->searchResult->selectedItems().isEmpty())
    {

        QString selected = this->searchResult->selectedItems().first()->text();
        QStringList list = selected.split('\t');
        QString selectedName = list.at(0);
        QByteArray fileid = this->searchStatus->getFileIDFromName(selectedName);


        // request who like the file from server
        requestWhoLikeTheFile(fileid);
    }else{
        this->textview->append("No file selected for who like request!");
    }
}


void ChatDialog::likeFile(QByteArray fileid)
{

    this->textview->append("like file");   //@@@ test code
    this->checkServerAddress();

    // create a message like some file
    QVariantMap likeMsg = createLikeMsg(fileid, LIKE);
    // send the like message to server
    sendMsg(likeMsg, this->serverAddress->first, this->serverAddress->second);
    // @@@ modify local value?
}

void ChatDialog::checkServerAddress()
{
    this->textview->append("Server:" + this->serverAddress->first.toString() + ": " + QString::number(this->serverAddress->second));
    if ((this->serverAddress->first.toIPv4Address()==0)||(this->serverAddress->second ==0))
    {
        this->textview->append("Server set error!");
    }
}

void ChatDialog::updateSearchResultWidget()
{
    this->searchResult->clear();
    QList<QByteArray> idlist = this->searchStatus->getSearchResultID();

    //@@@ to do sort?

    // for each file, print name,
    // also check if there is trust score, update
    foreach(QByteArray id, idlist)
    {
        QString fname = this->searchStatus->getNameFromID_sResultFiles(id);
        float freputation;
        if (TRUE == this->trustRec->hasReputation(id))
        {
            freputation = this->trustRec->getReputation(id);
            this->searchResult->addItem(QString(fname + "\t" + QString::number(freputation)));
        }
        else
        {
            this->searchResult->addItem(fname);
        }

    }
}


void ChatDialog::updateSearchResultWidget_Rec(QByteArray recFileID)
{
    this->searchResult->clear();
    QList<QByteArray> idlist = this->searchStatus->getSearchResultID();

    //@@@ to do sort?

    // for each file, print name,
    // also check if there is trust score, update
    foreach(QByteArray id, idlist)
    {

        QString fname = this->searchStatus->getNameFromID_sResultFiles(id);
        float freputation;
        if (TRUE == this->trustRec->hasReputation(id))
        {
            freputation = this->trustRec->getReputation(id);
            if (freputation >= THRESHOLD_C)
            {
                if(recFileID == id)
                {
                    this->searchResult->addItem(QString(fname + "\t" + QString::number(freputation) + " Overwhelm") + "\t <-recommend");
                }
                else
                {
                    this->searchResult->addItem(QString(fname + "\t" + QString::number(freputation) + "  Overwhelm"));
                }

            }
            else
            {
                if(recFileID == id)
                {
                    this->searchResult->addItem(QString(fname + "\t" + QString::number(freputation) + "\t<-recommend"));
                }
                else
                {
                    this->searchResult->addItem(QString(fname + "\t" + QString::number(freputation)));
                }
            }

        }
        else
        {
            this->searchResult->addItem(fname);
        }

    }
}


void ChatDialog::updateLocalTrustList()
{
    this->listNodeTrust->clear();
    QHash <QString, float> recList = this->trustRec->getTrustRecords();
    QList<QString> keys = recList.keys();
    float score;
    float rr;
    QString line;
    foreach (QString k, keys)
    {
        score = recList.value(k);
        rr = qPow(10, 3);
        score = qRound(score * rr) / rr;

        line = QString(k + "\t" + QString::number(score));
        this->listNodeTrust->addItem(line);
    }
    //@@@ do i need to make it by some order???
    //@@@ mark overwhelming
}

// repeat current search, double the budget
// stop when reached 10,
void ChatDialog::timeOutRepeatSearch()
{

    // check if need to continue,
    QVariantMap currRqst = this->searchStatus->getCurrentRequest();
    quint32 budget = currRqst.value("Budget").toUInt(0);
    // check if already got 10 request
    if ((this->searchResult->count() >= STOP_RQST_SEARCH_R_NUM) || ((budget*2) > MAX_BUDGET) )
    {
        timer_search->stop();
    }
    else
    {
        // double budget, resend
        budget = budget * 2;
        currRqst.insert("Budget", budget);
        this->searchStatus->setCurrRequest(currRqst);
        sendSearchRequest(currRqst);
    }
}

void ChatDialog::searchButtonClicked()
{
    // @@@ get search key words
    QString inputKW = this->lineKeyW->text();
    this->lineKeyW->clear();
    this->searchResult->clear();
    if (FALSE == inputKW.isEmpty())
    {
        // create search request
        QVariantMap searchRqMsg = createSearchRequest(inputKW);
        // update current searchStatus
        this->searchStatus->setCurrRequest(searchRqMsg);
        //@@@ clear sresultfiles
        this->searchStatus->clearSResultFiles();
        // clear local stored who liked search
        this->trustRec->clear_sr_WhoLikeLocalList();
        // clear search result reputation
        this->trustRec->clearSearchResultReputation();
        // send search request - through neighbour
        sendSearchRequest(searchRqMsg);
        this->timer_search->start(TIME_INC_BUDGET);
    }
}


void ChatDialog::requestWhoLikeTheFile(QByteArray fileid)
{
    // @@@ to do
    // send a request to server
    QVariantMap whoLikedRequest = createWhoLikeRequest(fileid);

    //if not inited the server value
    if (0 == serverAddress->second)
    {
        this->textview->append("Server not set!");
        return;
    }
    // message request for the fileid
    sendMsg(whoLikedRequest, serverAddress->first, serverAddress->second);

}


void ChatDialog::sendSearchRequest(QVariantMap searchRequest)
{
    Peer nb_peer;
    QHostAddress nb_address;
    quint16 nb_portNum;
    quint32 pickNbNum = searchRequest.value("Budget").toUInt(0);
    int nbNum = this->peerList->size();
    /*
    if (pickNbNum < nbNum)
    {
        pickNbNum = nbNum;
    }
    // @@@ todo, pick the first few nb for now
    for (int i = 0; i < pickNbNum; i++)
    {
        nb_peer = peerList->at(i);
        nb_address = nb_peer.getIP();
        nb_portNum = nb_peer.getPort();

        // @@@ not yet assigned budget
        // send to neighbour
        sendMsg(searchRequest, nb_address, nb_portNum);
    }
    */
    //@@@ test code, send to all neighbours now
    for (int i = 0; i < nbNum; i++)
    {
        nb_peer = peerList->at(i);
        // send to neighbour
        sendMsg(searchRequest, nb_peer.getIP(), nb_peer.getPort());
    }

    // @@@ to do ,need a timer, double the budget every 1 second
    // send again

}

//@@@ to do, neighbour select need to be random later
void ChatDialog::forwardSearchReqest_Bgt(QVariantMap searchRequest)
{
    Peer nb_peer;
    int nbNum = this->peerList->size();
    quint32 budget = searchRequest.value("Budget").toUInt(0);
    budget --; // decrease the budget by one
    searchRequest.insert("Budget", budget);

    // if budget < neighbour, random
    // pick budget number of neighours and send, each with budget 1
    if (budget <= nbNum)
    {
        searchRequest.insert("Budget", 1);
        // @@@ todo, pick the first few nb for now
        for (int i = 0; i < budget; i++)
        {
            nb_peer = peerList->at(i);
            // send to neighbour
            sendMsg(searchRequest, nb_peer.getIP(), nb_peer.getPort());
        }
    }
    else
    {
        // budget > neighbour
        quint32 base = budget / nbNum;
        quint32 carry = budget % nbNum;
        quint32 basePlusOne = base + 1;
        int j;
        for (j = 0; j< carry; j++)
        {
            searchRequest.insert("Budget", basePlusOne);
            nb_peer = peerList->at(j);
            // send to neighbour
            sendMsg(searchRequest, nb_peer.getIP(), nb_peer.getPort());
        }
        for (j = carry; j < (nbNum-carry); j++)
        {
            searchRequest.insert("Budget", base);
            nb_peer = peerList->at(j);
            // send to neighbour
            sendMsg(searchRequest, nb_peer.getIP(), nb_peer.getPort());
        }
    }
}

void ChatDialog::serverShareClicked()
{
    // to do file
    QStringList fNames = QFileDialog::getOpenFileNames();
    QByteArray fileID;
    QByteArray bkListHashHex;
    // create a file for it
    //this->textview->append("ShareFile:");
    foreach (QString name, fNames)
    {
        //@@@ test code show file shared
        QString nameNoPath = "File name:";
        QStringList list = name.split('/');
        int size = list.size();
        nameNoPath = list.at(size-1);
        //this->textview->append(nameNoPath);
        fileID = this->fileDBlocal->insertNewFile(name);
        // create HEX, show in the textview
        bkListHashHex = fileID.toHex();
        //this->textview->append(bkListHashHex);
        qDebug() << "File hex:" << bkListHashHex;

        //@@@ what if add the same item??
        //QString showItem  = QString(nameNoPath + "\t 0");
        //this->listFileShared->addItem(showItem);
    }
    updateListFileShared();
    //this->textview->append("-----------------");
}

void ChatDialog::updateListFileShared()
{
    this->listFileShared->clear();
    QList<QByteArray> fileIDlist = this->fileDBlocal->keysOfFileDB();
    File file;
    QString fname;
    int likeNum;
    QString nameNoPath;
    int size;
    QString showItem;
    foreach (QByteArray fid, fileIDlist)
    {
        file = this->fileDBlocal->getFileFromID(fid);
        fname = file.getFileName();
        likeNum = file.getNumOfLike();

        QStringList list = fname.split('/');
        size = list.size();
        nameNoPath = list.at(size-1);
        showItem  = QString(nameNoPath + "\t" + QString::number(likeNum));
        this->listFileShared->addItem(showItem);
    }

}

void ChatDialog::shareFileClicked()
{
    // to do file
    QStringList fNames = QFileDialog::getOpenFileNames();
    QByteArray fileID;
    QByteArray bkListHashHex;
    // create a file for it
    this->textview->append("ShareFile:");
    foreach (QString name, fNames)
    {
        //@@@ test code show file shared
        QString nameNoPath = "File name:";
        QStringList list = name.split('/');
        int size = list.size();
        nameNoPath = list.at(size-1);
        this->textview->append(nameNoPath);
        fileID = this->fileDBlocal->insertNewFile(name);
        // create HEX, show in the textview
        bkListHashHex = fileID.toHex();
        this->textview->append(bkListHashHex);
        qDebug() << "File hex:" << bkListHashHex;

    }
    this->textview->append("-----------------");
}

void ChatDialog::downloadFileClicked()
{
    // check if entered valid target node name and target
    QString targetID = this->textTargetID->toPlainText();
    QString fileID = this->textFileID->toPlainText();
    this->textTargetID->clear();
    this->textFileID->clear();
    if ((targetID == NULL) || (fileID == NULL))
    {
        qDebug() << "invalid target or file id.";
        return;
    }
    // convert from hex to qbytearray
    QByteArray blockListHash = QByteArray::fromHex(fileID.toUtf8());
    // request to download -> prepare -> create message -> send request
    requestToDownload(blockListHash, targetID);

}

void ChatDialog::requestToDownload(QByteArray fileID, QString targetName)
{
    QVariantMap requestMsg = createBlockRequest(targetName, fileID);

    // insert into the fileHashes table, so we know this is a fileID, with null
    this->fileDBlocal->insertIntoFileHashes(fileID,NULL);

    // create FileRcving insert into the receiving list
    FileRcving rcvFile (fileID);
    this->receivingFiles->insert(fileID, rcvFile);

    // send a block request to download the blocklist metafile,
    // through routing table
    QPair <QHostAddress, quint16> destContent = this->msgDatabase->getRouteContent(targetName);
    sendMsg(requestMsg, destContent.first, destContent.second);

    // wait for valid reply to that request
    // hash matching the requested hash and content

    // retransmitting request periodically, and download

}

QVariantMap ChatDialog::createSearchRequest(QString keyword)
{
    QVariantMap searchRequest;
    QString originName = netSocket->getSocketOriginName();
    searchRequest.insert("Origin",originName); // the orignial sender
    searchRequest.insert("Search",keyword);
    searchRequest.insert("Budget", (quint32)INIT_BUDGET); // sha256 hash of the block requested
    return searchRequest;
}

QVariantMap ChatDialog::createSearchReply(QString dest, QString searchkey, QVariantList matchNames, QVariantList matchIDs)
{
    QVariantMap searchReply;
    QString originName = netSocket->getSocketOriginName();
    searchReply.insert("Dest",dest);
    searchReply.insert("Origin",originName);
    searchReply.insert("HopLimit",(quint32)HOP_LIMIT);
    searchReply.insert("SearchReply",searchkey);
    searchReply.insert("MatchNames",matchNames);
    searchReply.insert("MatchIDs",matchIDs);
    return searchReply;
}

QVariantMap ChatDialog::createBlockRequest(QString target,  QByteArray bkHash)
{
    // put message into QVariantMap
    QVariantMap blockRequest;
    QString originName = netSocket->getSocketOriginName();
    blockRequest.insert("Dest",target); // from input
    blockRequest.insert("Origin",originName); // the orignial sender
    blockRequest.insert("HopLimit",(quint32)HOP_LIMIT);
    blockRequest.insert("BlockRequest", bkHash); // sha256 hash of the block requested
    return blockRequest;
}

QVariantMap ChatDialog::createBlockReply(QString target, QString originName, QByteArray rqstRpl, QByteArray data)
{
    QVariantMap blockReply;
    blockReply.insert("Dest",target);
    blockReply.insert("Origin",originName);
    blockReply.insert("HopLimit",(quint32)HOP_LIMIT);
    blockReply.insert("BlockReply", rqstRpl);
    blockReply.insert("Data", data);

    return blockReply;
}

QVariantMap ChatDialog::createLikeMsg(QByteArray fileid, quint32 like)
{
    // like = 1 means like, like = -1 unlike
    QVariantMap likeMsg;
    QString originName = netSocket->getSocketOriginName();
    likeMsg.insert("Origin",originName); // the orignial sender
    likeMsg.insert("ServerIP", serverAddress->first.toIPv4Address());
    likeMsg.insert("ServerPort", serverAddress->second);
    likeMsg.insert("FileID",fileid);
    likeMsg.insert("Like",(quint32)like);
    return likeMsg;
}

QVariantMap ChatDialog::createWhoLikeRequest(QByteArray fileID)
{
    QVariantMap whoLikeRequest;
    QString originName = netSocket->getSocketOriginName();
    whoLikeRequest.insert("Origin",originName); // the orignial sender
    whoLikeRequest.insert("ServerIP", serverAddress->first.toIPv4Address());
    whoLikeRequest.insert("ServerPort", serverAddress->second);
    whoLikeRequest.insert("FileAsked",fileID);
    return whoLikeRequest;
}

QVariantMap ChatDialog::createWhoLikeReply(QString dest, QByteArray fid)
{
    // put message into QVariantMap
    this->textview->append("create who like reply");
    QVariantMap whoLikeReply;
    QString originName = netSocket->getSocketOriginName(); //@@@ = getSocketName

    // find out who liked the file from server's fileDB
    File f = this->fileDBlocal->getFileFromID(fid);
    QList<QString> plist =  f.getPeerLikedKeys();
    QVariant pl = (QVariant)plist;
    int numlike = f.getNumOfLike();

    whoLikeReply.insert("Origin",originName);// should be send from server
    whoLikeReply.insert("Dest",dest); // dest should be the requested node
    whoLikeReply.insert("FileID", fid);
    whoLikeReply.insert("OriginList", pl);
    whoLikeReply.insert("NumOfLike",numlike);

    return whoLikeReply;
}

// check argument, if it is no forward mode,
// noforward mode = true
void ChatDialog::checkNoForwardMode()
{
    //@@@ check the add peer command line, have problem
    if (QCoreApplication::argc() > 1)
    {
        QStringList args = QCoreApplication::arguments();
        foreach(QString str,args)
        {
            if ("-noforward" == str)
            {
                this->noForwardMode = TRUE;
                this->textview->append("no forward");
                break;
            }
        }
    }
}

void ChatDialog::checkRecServerMode()
{
    //@@@ check the add peer command line, have problem
    if (QCoreApplication::argc() > 1)
    {
        QStringList args = QCoreApplication::arguments();
        foreach(QString str,args)
        {
            if ("-rec_server" == str)
            {
                this->recServerMode = TRUE;
                this->textview->append("[Server]");
                break;
            }
        }
    }
}
void ChatDialog::checkRecClientMode()
{
    //@@@ check the add peer command line, have problem
    if (QCoreApplication::argc() > 1)
    {
        QStringList args = QCoreApplication::arguments();
        foreach(QString str,args)
        {
            if ("-rec_client" == str)
            {
                this->recClientMode = TRUE;
                this->textview->append("[Client]");
                break;
            }
        }
    }
}



// the origin should be the destination going to send
void ChatDialog::gotTextFromPrivate(QString origin, QString txt)
{
    if (FALSE == this->noForwardMode)
    {
        // create private message, and send
        QVariantMap privateMsg = createPrivateMessage(origin, txt);

        QPair <QHostAddress, quint16> destContent =
                this->msgDatabase->getRouteContent(origin);
        sendMsg(privateMsg, destContent.first, destContent.second);
    }
}

void ChatDialog::doubleClickedSearchResult(QListWidgetItem* item)
{
    QString selectedFileName = item->text();
    QByteArray fileid = this->searchStatus->getFileIDFromName(selectedFileName);
    QString origin = this->searchStatus->getFileOriginFromID(fileid);

    //@@@ do download
    requestToDownload(fileid, origin);

}

void ChatDialog::clickedServerSharedFile(QListWidgetItem* item)
{
    //@@@ to do
    //show the clicked peers that like the file
    this->textVotedClients->clear();

    // find this file in db
    QString selectedName = item->text().split("\t").at(0);
    QByteArray fileid =this->fileDBlocal->getFileIDfromFileName(selectedName);
    QString header = "Voters for " + selectedName + ":";
    this->textVotedClients->addItem(header);
    // find the file's voters
    File file = this->fileDBlocal->getFileFromID(fileid);

    QList<QString> peerLikedNames = file.getPeerLikedKeys();
    // show these voters
    foreach (QString s, peerLikedNames)
    {
        // show the origin name
        this->textVotedClients->addItem(s);

    }
}


void ChatDialog::doubleClickedWidgetItem(QListWidgetItem* item)
{
    QString txt = item->text();
    pDialog->p_setPrivateOrigin(txt);
    this->pDialog->show();
}
void ChatDialog::selectedChanged(QListWidgetItem *)
{
    //textPrivate->append("select changed");
}
// listWidget routes
void ChatDialog::insertRoute(QString origin, QHostAddress address, quint16 port)
{
    // if origin is not myself, insert
    if (this->netSocket->getSocketOriginName() != origin)
    {
        // insert in route table
        // should replace the old value if already exsist
        int insertRouteSt = this->msgDatabase->insertRouteToTable(origin, address, port);
        // if does not have this origin in the list, added to the listWidget
        if (ROUTE_INSERT == insertRouteSt)
        {
            this->listWidget->addItem(origin);
        }
    }
}

void ChatDialog::startPrivateCon()
{
    // check if any value selected
    if (FALSE == this->listWidget->selectedItems().isEmpty())
    {
        QString selected = this->listWidget->selectedItems().first()->text();
        pDialog->p_setPrivateOrigin(selected);
        // start conversation
        pDialog->show();
    }
}

void ChatDialog::showPeers()
{
    this->textPeers->clear();
    QList<Peer> *lp = this->peerList;
    QString str;
    this->textPeers->append("My neighbours:");
    foreach (Peer p, *lp)
    {
        str.clear();
        str.append("<");
        str.append(p.getIP().toString());
        str.append(":");
        str.append(QString::number(p.getPort()));
        str.append(">");

        this->textPeers->append(str);
    }
}

void ChatDialog::initPeerList()
{
    // insert the other three local peer
    for (quint16 p = netSocket->getMyPortMin(); p <= netSocket->getMyPortMax(); p++)
    {   // if not myself insert into peerList
        if (netSocket->getLocalPortNumber() != p)
        {
            //Peer peer (QHostAddress::LocalHost, p);
            Peer peer;
            peer.setIP(QHostAddress::LocalHost);
            peer.setPort(p);
            peerList->append(peer); // add to the front
        }
    }

    // insert the command line peer if there is one
    // and it is not already in there
    if (QCoreApplication::argc() > 1)
    {
        QStringList args = QCoreApplication::arguments();
        if ("-noforward" != args.at(1))
        addPeer(args.at(1), false);
    }
}

Peer ChatDialog::pickRandomNeighbour()
{
    // check if the neighbourlist is empty
    Peer peer;
    int listSize = this->peerList->size();
    // pick a random value in the neibourlist size
    int random = qrand() %  listSize;
    peer = peerList->at(random);

    // return the peer in the position
    //@@@ testcode
    /*
     qDebug()<< "random=" << random <<
               "pick <" << peer.getIP().toString() <<
              ":"<< QString::number(peer.getPort()) << ">";
              */
    return peer;

}

void ChatDialog::addNewNeighbour()
{
    QString inputPeerStr = this->inputPeer->text();
    this->inputPeer->clear();
    addPeer(inputPeerStr, false);
    showPeers();
}


void ChatDialog::addNewNbAndSetServer()
{
    QString inputPeerStr = this->inputPeer->text();
    this->inputPeer->clear();
    addPeer(inputPeerStr, true);
    showPeers();
}


// addPeer
void ChatDialog::addPeer(QString inputPeer, bool isServer)
{
    QStringList inputList = inputPeer.split(":");
    if (2 != inputList.size())
    {
        qDebug() << "bad peer input";
    }
    else
    {
        QHostAddress ip_name = QHostAddress(inputList.at(0));
        quint16 port = inputList.at(1).toUInt(0);
        //@@@ check
        // assume ip is address, check if the peer is new
        bool newPeer = TRUE;
        foreach (Peer pr, *(this->peerList))
        {
            if ((pr.getIP() == ip_name) && (pr.getPort() == port))
            {
                // pr already in the list
                newPeer = FALSE;
                break;
            }
        }
        // if it is new
        if (TRUE == newPeer)
        {
            // add the peer in to list
            Peer *peer = new Peer(ip_name, port);
            this->peerList->append(*peer);
            // set server for recommendation dsybil
            if (TRUE == isServer)
            {
                this->serverAddress->first = ip_name;
                this->serverAddress->second = port;
                //this->textview->append("Server:" + ip_name.toString() + ": " + QString::number(port));
                this->textview->append("Server:" + this->serverAddress->first.toString() + ": " + QString::number(this->serverAddress->second));
            }

            //if passed in Host name, lookup host ip
            if (TRUE == ip_name.isNull())
            {
                QHostInfo::lookupHost(inputList.at(0),this, SLOT(lookedUp(QHostInfo)));

            }
        }
    }
}


void ChatDialog::lookedUp(const QHostInfo &host)
{
    if (host.error() != QHostInfo::NoError)
    {
        qDebug() << "Lookup failed:" << host.errorString();
        return;
    }

    foreach (const QHostAddress &address, host.addresses())
    {
        // find the neighbour, change it to
        QList<Peer> *list = this->peerList;
        QList<Peer>::iterator pIter;
        for (pIter = list->begin(); pIter != list->end();pIter++)
        {
            if (TRUE == pIter->getIP().isNull())
            {
                // set ip
                pIter->setIP(address);
                qDebug() << "set ip:" << pIter->getIP().toString();
                break;            }
        }
    }
   showPeers();

}

bool ChatDialog::eventFilter(QObject *obj, QEvent *event)
{
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return)
            {
                gotReturnPressed();
                return true;
            }
            return false;
        }
        else
        {
            return QObject::eventFilter(obj, event);
        }
}

void ChatDialog::timeOutHandler()
{
    //@@@ to do, time out, flip coin??
    // qDebug() << "Time Out!" ;

}

void ChatDialog::sendRouteToAllPeers(QVariantMap routeRummor)
{
    foreach (Peer nb_peer,*(this->peerList))
    {
        QHostAddress nb_address = nb_peer.getIP();
        quint16 nb_portNum = nb_peer.getPort();
        // send routeRummor
        sendMsg(routeRummor, nb_address, nb_portNum);

    }
    // qDebug() << "route to all peers.";
}

// send route rummor of my latest needed msg to random neighbour
void ChatDialog::timerRouteRumorHandler()
{
    //@@@ what should we send here???
    // create a route rumor, contain latest msg of myself
   QVariantMap routeRummor = createNewRouteRummor();

   // send to all peers
   this->sendRouteToAllPeers(routeRummor);


   // @@@ copied from pickNBSendStatus()
   // pick random nb and send status message
   /*
   Peer nb_peer = pickRandomNeighbour();
   QHostAddress nb_address = nb_peer.getIP();
   quint16 nb_portNum = nb_peer.getPort();
   sendMsg(routeRummor, nb_address, nb_portNum);
   */

}
void ChatDialog::timeOutAntiEntropy()
{
    qDebug() << "10 seconds send status.";
    pickNBSendStatus();
}

void ChatDialog::flipCoin()
{
    int random = qrand();
    if (0 == random % 2)
    {
        // pick new neighbor and send status msg
        pickNBSendStatus();
    }
    else
    {
        // cease, excit = 0
    }
}

void ChatDialog::pickNBSendStatus()
{
    // pick random nb and send status message
    Peer nb_peer = pickRandomNeighbour();
    QHostAddress nb_address = nb_peer.getIP();
    quint16 nb_portNum = nb_peer.getPort();

    QVariantMap localStatus = this->msgDatabase->getDBStatus();
    // send current status
    sendMsg(localStatus, nb_address, nb_portNum);
}

void ChatDialog::gotReturnPressed()
{
    // Initially, just echo the string locally.
    // Insert some networking code here...
    QString input = textline->toPlainText();

    //@@@ do we need to consider empty input??
    // create new message
    QVariantMap myNewMsg = createNewMsg(input);


    QString showInput = "I say: ";
    showInput.append(input);
    textview->append(showInput);

    // Clear the textline to get ready for the next input message.
    textline->clear();

    if (FALSE == this->noForwardMode)
    {
        //@@@ got new message !
        afterSavedNewMsg_sendToOneNB(myNewMsg);

    }
}


void ChatDialog::afterSavedNewMsg_sendToOneNB(QVariantMap newMsg)
{

    // pick random neighbor as target,
    Peer nb_peer = pickRandomNeighbour();
    QHostAddress nb_address = nb_peer.getIP();
    quint16 nb_portNum = nb_peer.getPort();
    quint16 localPortNum = netSocket->getLocalPortNumber();

    // resend to it
    sendMsg(newMsg,nb_address, nb_portNum);

    // wait
    // if time out, go to timerOutHandler
    //@@@ timer_1s->start(TIMER_LENGTH);

}

QVariantMap ChatDialog::createNewMsg(QString inputMsg)
{

    // put message into QVariantMap
    QVariantMap msgMap;
    QString originName = netSocket->getSocketOriginName(); //@@@ = getSocketName
    quint32 seqNo = msgDatabase->getSeqNo(originName); //@@@ = getSeqNo

    msgMap.insert("ChatText", inputMsg);
    msgMap.insert("Origin",originName);
    msgMap.insert("SeqNo",seqNo);

    // the msgMap into local database
    msgDatabase->insertMsg(msgMap);
    return msgMap;

}

QVariantMap ChatDialog::createPrivateMessage(QString targetName, QString inputTxt)
{
    // put message into QVariantMap
    QVariantMap msgMap;
    quint32 hlimit = HOP_LIMIT;
    msgMap.insert("Dest", targetName);
    msgMap.insert("ChatText",inputTxt);
    msgMap.insert("HopLimit",hlimit);

    // do not save private message
    return msgMap;
}


QVariantMap ChatDialog::createNewRouteRummor()
{
    // put message into QVariantMap
    QVariantMap msgMap;
    QString originName = netSocket->getSocketOriginName(); //@@@ = getSocketName
    quint32 seqNo = msgDatabase->getSeqNo(originName); //@@@ = getSeqNo

    msgMap.insert("Origin",originName);
    msgMap.insert("SeqNo",seqNo);

    msgDatabase->insertMsg(msgMap);

    // qDebug()<< "create route rummor " << originName <<": "<< seqNo;
    return msgMap;
}


QByteArray ChatDialog::createByteArrayFromMsg(QVariantMap msgMap)
{
    // put into QDataStream
    QByteArray msgByteArray;
    QDataStream msgStream(&msgByteArray,QIODevice::ReadWrite);
    msgStream << msgMap;
    return msgByteArray;

}


void ChatDialog::readPendingDatagrams()
{
    while(netSocket->hasPendingDatagrams())
    {
        //@@@ textview->append("TEST: Got pending datagrams");
        QByteArray datagram; // the byte array sended in
        datagram.resize(netSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        int proc_status;

        netSocket->readDatagram(datagram.data(), datagram.size(),
                             &sender, &senderPort);

        //@@@ check if the sender is a new peer,
        // if so, add into the peerList

        checkAndInsertPeer(sender, senderPort);

        // @@@
        // check which kind of message is this
        int mtype = checkDiagramType(datagram);
        // RECEIVE_STATUS 1: got status message
        if (MTYPE_STATUS == mtype)
        {
            proc_status = gotStatusDatagram(datagram, sender, senderPort);

        }
        // RECEIVE_STATUS 2: got message
        else if (MTYPE_CHAT_RUMMOR == mtype)
        {
            // process the message datagram, end in waiting
            proc_status = processChatRummorDatagram(datagram, sender, senderPort);
        }
        else if (MTYPE_ROUTE_RUMMOR == mtype)
        {
            // process the route rummor, similar to chat rummor,
            // but not display chat text
            proc_status = processRouteRummorDatagram(datagram, sender, senderPort);
        }
        else if (MTYPE_PRIVATE == mtype)
        {
            // process private message
            proc_status = processPrivateMsg(datagram, sender, senderPort);
        }
        else if (MTYPE_BLOCK_REQUEST == mtype)
        {
            // process request
            proc_status = processBlockRequest(datagram, sender, senderPort);

        }
        else if (MTYPE_BLOCK_REPLY == mtype)
        {
            // process  reply
            proc_status = processBlockReply(datagram, sender, senderPort);
        }
        else if (MTYPE_SEARCH_REQUEST == mtype)
        {
            // process  search request
            proc_status = processSearchRequest(datagram, sender, senderPort);
        }
        else if (MTYPE_SEARCH_REPLY == mtype)
        {
            // process search reply
            proc_status = processSearchReply(datagram, sender, senderPort);
        }
        else if (MTYPE_WHOLIKE_RQST == mtype)
        {
            proc_status = processWhoLikeRequest(datagram, sender, senderPort);
        }
        else if (MTYPE_WHOLIKE_REPLY == mtype)
        {
            proc_status = processWhoLikeReply(datagram, sender, senderPort);
        }
        else if (MTYPE_LIKE_FILE == mtype)
        {
            proc_status = processLikeFile(datagram, sender, senderPort);
        }
        else
        {
            //@@@ error
            qDebug() << "Recieved invalid byte array." ;
        }
        //@@@ need to check status
    }
}


int ChatDialog::processSearchRequest(QByteArray msgByteArray,
                                        QHostAddress sender, quint16 senderPort)
{
    // locally do the search
    // return file names and fileIDs
    // create search reply
    // modify the budget and send to neigbours
    QVariantList mNames;
    QVariantList mIDs;
    int rtn = RTN_OK;
    bool wordRelated;
    bool isRelatedFile;
    QString fileName_tmp;
    QVariantMap msgMap; // the message we got
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
    msgStream >> msgMap;

    QString msgOrigin = msgMap.value("Origin").toString();
    QString msgSearch= msgMap.value("Search").toString();
    quint32 msgBudget = msgMap.value("Budget").toUInt();

    //@@@ to do, need to check if I have processed this request before.
    // seems i don't need to check that, just the receiver check.


    // split to get keywords
    QStringList keyWords = msgSearch.split(" ");

    if (keyWords.size() < 1)
    {
        rtn = RTN_NO_KEYWORD;
        return rtn;
    }
    // for each shared file, find if it is related,
    // if it is insert in the both file
    QList<QByteArray> allFileIDs = this->fileDBlocal->keysOfFileDB();
    foreach (QByteArray k, allFileIDs)
    {
        isRelatedFile = FALSE;
        // get file name
        fileName_tmp = this->fileDBlocal->getFileNameinFileDB(k);
        // see if the name is related to any of the keywords
        foreach (QString word, keyWords)
        {
            wordRelated = this->fileDBlocal->fileNameRelateToKey(fileName_tmp, word);
            if (TRUE == wordRelated)
            {
                isRelatedFile = TRUE;
                break;
            }
        }
        if (TRUE == isRelatedFile)
        {
            // insert in both variant map
            mNames.append(fileName_tmp);
            mIDs.append(k);
        }
    }
    // check if I have related file.
    if ((mNames.size() <= 0) && (mIDs.size() <= 0))
    {
        rtn = RTN_NO_MATCHING_FILE; //@@@ but still need to send to neighbour
    }
    else
    {
        // create search reply, send through routing table
        QVariantMap sReplyMsg = this->createSearchReply(msgOrigin,msgSearch,mNames,mIDs);
        QPair <QHostAddress, quint16> destContent = this->msgDatabase->getRouteContent(msgOrigin);
        sendMsg(sReplyMsg, destContent.first, destContent.second);
    }

    //@@@ send the request to neighbours
    forwardSearchReqest_Bgt(msgMap);

    rtn = RTN_OK;
    return rtn;
}

int ChatDialog::processLikeFile(QByteArray msgByteArray,
                                QHostAddress sender, quint16 senderPort)
{
    this->textview->append("procesLikeFile"); //@@@ test code
    //decode Msg
    int rtn = RTN_OK;
    QVariantMap msgMap; // the message we got
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
    msgStream >> msgMap;

    QString msgOrigin = msgMap.value("Origin").toString();
    quint32 msgServerIP = msgMap.value("ServerIP").toUInt();
    quint16 msgServerPort = msgMap.value("ServerPort").toUInt();
    QByteArray msgFileID = msgMap.value("FileID").toByteArray();
    quint32 msgLike = msgMap.value("Like").toUInt();


    // check if the message is for me
    quint32 localip = netSocket->getLocalIP().toIPv4Address();
    quint16 localport = netSocket->getLocalPortNumber();
    if  ((msgServerIP == localip) && (msgServerPort == localport))
    {

        // server when receiving the request,
        // find the file in server
        bool success = false;
        QPair<QHostAddress, quint16>addr = this->msgDatabase->getRouteContent(msgOrigin);
        success = this->fileDBlocal->likeOneFile(msgFileID,msgOrigin, addr.first,addr.second, msgLike);

        if (true == success)
        {
            // update file like list
            updateListFileShared();
        }
        //@@@ send response back?

    }
    return rtn;

}

int ChatDialog::processWhoLikeRequest(QByteArray msgByteArray,
                                      QHostAddress sender, quint16 senderPort)
{
    //decode Msg
    int rtn = RTN_OK;
    QVariantMap msgMap; // the message we got
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
    msgStream >> msgMap;

    QString msgOrigin = msgMap.value("Origin").toString();
    quint32 msgServerIP = msgMap.value("ServerIP").toUInt();
    quint16 msgServerPort = msgMap.value("ServerPort").toUInt();
    QByteArray msgFileID = msgMap.value("FileAsked").toByteArray();

    // check if the message is for me
    quint32 localip = netSocket->getLocalIP().toIPv4Address();
    quint16 localport = netSocket->getLocalPortNumber();
    if  ((msgServerIP == localip) && (msgServerPort == localport))
    {
        // server when receiving the request,
        // find in db who liked the file
        // and create new message and send back
        QVariantMap whoLikeReply = createWhoLikeReply(msgOrigin, msgFileID);
        QPair<QHostAddress, quint16> addr = this->msgDatabase->getRouteContent(msgOrigin);
        sendMsg(whoLikeReply, addr.first, addr.second );
    }
    return rtn;

}

int ChatDialog::processWhoLikeReply(QByteArray msgByteArray,
                                      QHostAddress sender, quint16 senderPort)
{
    //decode Msg

    int rtn = RTN_OK;
    QVariantMap msgMap; // the message we got
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
    msgStream >> msgMap;

    QString msgOrigin = msgMap.value("Origin").toString();
    QString msgDest = msgMap.value("Dest").toString();
    QByteArray msgFileID = msgMap.value("FileID").toByteArray();
    int msgNumOfLike = msgMap.value("NumOfLike").toInt();

    // check if the message is for me
    // check if it is for current
    this->textview->append(QString("Num of Like = "+QString::number(msgNumOfLike)));
    QString myName = netSocket->getSocketOriginName();

    if (msgDest == myName)
    {
        // @@@ now save in the client local db
        QList<QVariant> plist_var = msgMap.value("OriginList").toList();
        QList<QString> plist;

        //@@@ maybe update the trust value for each peer

        foreach(QVariant p, plist_var)
        {
            QString nodeName = p.toString();
            // insert into the local trust list if new
            if (FALSE == this->trustRec->hasNode(nodeName))
            {
                this->trustRec->insertNewNode(nodeName);
                this->updateLocalTrustList();
            }
            plist.append(nodeName);
        }

        //@@@ to do, append store plist into trustRec locally
        this->trustRec->insertWhoLikeLocalList(msgFileID, plist);

        // compute the file's reputation score,
        // update reputation score in trustRec, if it is for current search
        if (TRUE == this->searchStatus->isFileCurrentResult(msgFileID))
        {
            float reputation = this->trustRec->computeFileReputation(plist);
            this->trustRec->insertReputation(msgFileID, reputation);
        }

        // recommend along the way
        QByteArray recommendation = this->trustRec->recommendFromSearchResult();

        //update search list
        this->updateSearchResultWidget_Rec(recommendation);

    }
    return rtn;

}


int ChatDialog::processSearchReply(QByteArray msgByteArray,
                                    QHostAddress sender, quint16 senderPort)
{
    // check if this reply is for me, if not send through routing table
    // if it is, see if I have got it before, save and display

    // "Dest", "ChatText", "HopLimit"
    //decode Msg
    int rtn = RTN_OK;
    QVariantMap msgMap; // the message we got
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
    msgStream >> msgMap;

    QString msgDestStr = msgMap.value("Dest").toString();
    QString msgOrigin = msgMap.value("Origin").toString();
    quint32 msgHopLimit = msgMap.value("HopLimit").toUInt();
    QString msgKeyWords = msgMap.value("SearchReply").toString();
    QVariantList msgMatchNames = msgMap.value("MatchNames").toList();
    QVariantList msgMatchIDs = msgMap.value("MatchIDs").toList();


    // check if the message is for me, show in the private dialog
    QString myName = netSocket->getSocketOriginName();
    QByteArray fileID;
    QString name;
    bool isNew;
    if  (myName ==  msgDestStr)
    {
        // check if the file is for the current search
        if (TRUE == this->searchStatus->replyIsForCurrRequest(msgKeyWords))
        {

            //@@@ check each file, if it contains something new
            // display in the listView in order
            foreach (QVariant item, msgMatchIDs)
            {
                fileID = item.toByteArray();
                isNew = this->searchStatus->isNewResult(fileID);
                if (TRUE == isNew)
                {
                    // get name from id, insert into sResultFiles, show int the list
                    name = this->searchStatus->getNameFromID(msgMatchNames, msgMatchIDs, fileID);
                    this->searchStatus->insertInSResultFiles(fileID, name, msgOrigin);

                    //@@@ update search result widget
                    //this->searchResult->addItem(name);
                    updateSearchResultWidget();

                    // ask server about how many people liked it
                    // ask server who liked it

                    // @@@ to do
                    requestWhoLikeTheFile(fileID);

                }
            }
            rtn = RTN_OK;
        }
    }
    else
    {
        // send it to someone else, if hoplimit is zero
        // decrese the HopLimit when send out
        if ((msgHopLimit > 0) && (FALSE == this->noForwardMode))
        {
            msgMap.insert("HopLimit", msgHopLimit-1);
            // should send to, route table
            QPair <QHostAddress, quint16> destContent =
                    this->msgDatabase->getRouteContent(msgDestStr);
            sendMsg(msgMap, destContent.first, destContent.second);
        }
        rtn = RTN_OK;
    }
    return rtn;
}


int ChatDialog::processBlockRequest(QByteArray msgByteArray,
                                    QHostAddress sender, quint16 senderPort)
{
    //@@@
    // "Dest", "ChatText", "HopLimit"
    //decode Msg
    int rtn = RTN_OK;
    QVariantMap msgMap; // the message we got
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
    msgStream >> msgMap;

    QString msgDestStr = msgMap.value("Dest").toString();
    QString msgOrigin = msgMap.value("Origin").toString();
    quint32 msgHopLimit = msgMap.value("HopLimit").toUInt();
    QByteArray msgBlockRqst = msgMap.value("BlockRequest").toByteArray();

    QString myName = netSocket->getSocketOriginName();

    QVariantMap replyMsg;
    // check if the request is for me
    if  (myName ==  msgDestStr)
    {
        qDebug() << "[File]got request for me";

        // create a reply message, with the hash and its content,
        // if received request for file, content should be the blocklist metafile
        // if received requset for block, content should be the block content
        QByteArray content;
        // check if I have the file
        // get the data from the database
        if (FALSE == this->fileDBlocal->allContentContains(msgBlockRqst))
        {
            //@@@
            qDebug()<< "I don't have the file." << msgBlockRqst;
            rtn = RTN_NOT_IN_FILE_CONTENT;
        }
        else
        {
            content = this->fileDBlocal->getContent(msgBlockRqst);
            // and send, @@@ send through the routing table
            // reverse the target and origin

            replyMsg = createBlockReply(msgOrigin, msgDestStr,msgBlockRqst,content);
            QPair <QHostAddress, quint16> destContent =
                    this->msgDatabase->getRouteContent(msgOrigin);
            sendMsg(replyMsg, destContent.first, destContent.second);
            rtn = RTN_OK;
        }

    }
    else
    {
        // if the hop limit is zero, do nothing
        // if > 0,
        // send it to someone else
        // decrese the HopLimit when send out
        if ((msgHopLimit > 0) && (FALSE == this->noForwardMode))
        {
            msgMap.insert("HopLimit", msgHopLimit-1);
            // should send to, route table
            QPair <QHostAddress, quint16> destContent =
                    this->msgDatabase->getRouteContent(msgDestStr);
            sendMsg(msgMap, destContent.first, destContent.second);
        }
        rtn = RTN_OK;
    }

    return rtn;
}

int ChatDialog::processBlockReply(QByteArray msgByteArray,
                      QHostAddress sender, quint16 senderPort)
{
    //@@@
    // "Dest", "ChatText", "HopLimit"
    //decode Msg
    int rtn = RTN_OK;
    QVariantMap msgMap; // the message we got
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
    msgStream >> msgMap;

    QString msgDestStr = msgMap.value("Dest").toString();
    QString msgOrigin = msgMap.value("Origin").toString();
    quint32 msgHopLimit = msgMap.value("HopLimit").toUInt();
    QByteArray msgBlockReply = msgMap.value("BlockReply").toByteArray();
    QByteArray msgContent = msgMap.value("Data").toByteArray();
    int reqNextRtn;

    // check if the message is for me, show in the private dialog
    QString myName = netSocket->getSocketOriginName();
    if  (myName ==  msgDestStr)
    {
        //check if the hash matches the block content
        QCA::Hash shaHash3 ("sha256");
        QByteArray hash_tmp = shaHash3.hash(msgContent).toByteArray();
        if (hash_tmp != msgBlockReply)
        {
            qDebug() << "reply content does not match.";
            rtn = RTN_RPL_CONTENT_MISSMATCH;
            return rtn;
        }
        // Check if this is a fileID,
        if (TRUE == this->fileDBlocal->isFileID(msgBlockReply))
        {
            //and if it is one we requested
            if(TRUE == this->receivingFiles->contains(msgBlockReply))
            {

                FileRcving rcvFile = this->receivingFiles->value(msgBlockReply);

                // if it is the one we want, insert the meta into the FileRcving
                if (0 == rcvFile.getBlkNumWant())
                {
                    rcvFile.receivedMeta(msgContent);
                }

                // @@@ save into mydata file, way to save???
                // @@@ todo
                this->receivingFiles->insert(msgBlockReply,rcvFile);
                // send request for the first block
                reqNextRtn = requestNextFileBock(msgOrigin, msgBlockReply);
                if (RTN_RQST_BLOCK_END == reqNextRtn)
                {
                    // process file when transfer finishes
                    if (RTN_OK == rcvFile.endOfRcvThisFile())
                    {
                        this->textview->append("Empty file created !");
                    }
                    //@@@now and release the space in receivingFiles
                }
            }
        }
        else
        {
            // check if it is in my requested block list
            bool bkRqsted = this->fileDBlocal->blockRequested(msgBlockReply);
            if (FALSE == bkRqsted)
            {
                rtn = RTN_ERR;
                return  rtn;
            }
            // this reply is block id and content,
            QByteArray fID = this->fileDBlocal->getFileIDfromRqstBlockID(msgBlockReply);
            // check if it is the one I want, if it is
            // problem  insert into new node
            FileRcving tmpF = this->receivingFiles->value(fID);
            tmpF.appendToRawFile(msgContent);
            this->receivingFiles->insert(fID, tmpF);
            // @@@ send request into next block
            reqNextRtn = requestNextFileBock(msgOrigin, fID);
            if (RTN_RQST_BLOCK_END == reqNextRtn)
            {
                //@@@ process file when transfer finishes
                if (RTN_OK ==  tmpF.endOfRcvThisFile())
                {
                    this->textview->append("file created!");
                }
                //@@@ release??? in hashmap?
            }
        }


        // todo
        rtn = RTN_OK;
    }
    else
    {
        // if the hop limit is zero, do nothing
        // if > 0,
        // send it to someone else
        // decrese the HopLimit when send out
        if ((msgHopLimit > 0) && (FALSE == this->noForwardMode))
        {
            msgMap.insert("HopLimit", msgHopLimit-1);
            // should send to, route table
            QPair <QHostAddress, quint16> destContent =
                    this->msgDatabase->getRouteContent(msgDestStr);
            sendMsg(msgMap, destContent.first, destContent.second);
        }
        rtn = RTN_OK;
    }


    return rtn;

}




int ChatDialog::processPrivateMsg(QByteArray msgByteArray,
                      QHostAddress sender, quint16 senderPort)
{
    // "Dest", "ChatText", "HopLimit"
    //decode Msg
    int rtn = RTN_OK;
    QVariantMap msgMap; // the message we got
    QString msgDestStr;
    QString msgChatText;
    quint32 msgHopLimit;
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
    msgStream >> msgMap;


    msgDestStr = msgMap.value("Dest").toString();
    msgChatText = msgMap.value("ChatText").toString();
    msgHopLimit = msgMap.value("HopLimit").toUInt();

    QString myName = netSocket->getSocketOriginName();

    // check if the message is for me, show in the private dialog
    if  (myName ==  msgDestStr)
    {
        this->pDialog->p_showMessage(msgChatText);
        QString show = "[private ";
        show.append(msgDestStr);
        show.append("]");
        show.append(msgChatText);
        this->textview->append(show);
        rtn = RTN_OK;
    }
    else
    {
        // if the hop limit is zero, do nothing
        // if > 0,
        // send it to someone else
        // decrese the HopLimit when send out
        if ((msgHopLimit > 0) && (FALSE == this->noForwardMode))
        {
            msgMap.insert("HopLimit", msgHopLimit-1);
            // should send to, route table
            QPair <QHostAddress, quint16> destContent =
                    this->msgDatabase->getRouteContent(msgDestStr);
            sendMsg(msgMap, destContent.first, destContent.second);
        }
        rtn = RTN_OK;
    }


    return rtn;
}

int ChatDialog::requestNextFileBock(QString dest, QByteArray fileID)
{
    // get fileID -> nextNumNeeded -> get the block's id
    QByteArray blockID; // the blockID going to send for request
    FileRcving rfile = this->receivingFiles->value(fileID);
    QByteArray meta = rfile.getMeta();
    int blockNo = rfile.getBlkNumWant();
    if (blockNo < 1)
    {
        qDebug() << "something wrong" ;
        return RTN_ERR;
    }
    int metalen = meta.size();
    int pos = (blockNo-1) * SHA_HASH_LENGTH;
    int len = SHA_HASH_LENGTH;
    if (pos >= metalen)
    {
        // if the block's id meets end
        return RTN_RQST_BLOCK_END;
    }
    blockID = meta.mid(pos, len);// got block's id
    // @@@ save into the fileDB somewhere,
    // so we can get fileID from blockID
    this->fileDBlocal->insertBtoF(blockID, fileID);

    // create block request and send
    QVariantMap rqBkMsg = createBlockRequest(dest, blockID);
    QPair <QHostAddress, quint16> destContent =
            this->msgDatabase->getRouteContent(dest);
    sendMsg(rqBkMsg, destContent.first, destContent.second);

    return RTN_OK;
}




// check the lastIP and LastPort of rummor message,
// if not already in the peers, add to peer list.
// intend for A B to know each other, through route rummor
int ChatDialog::rummorCheckLast_addToPeers(QVariantMap msgMap)
{
    if ((FALSE == msgMap.contains("LastIP")) ||
            (FALSE == msgMap.contains("LastPort")) )
    {
        return RTN_NO_LAST_PATH;
    }
    // if contains a last path
    quint32 ip = msgMap.value("LastIP").toUInt();
    QHostAddress hostAddress = QHostAddress(ip);
    quint16 port = msgMap.value("LastPort").toUInt();

    // check if it is in the peerlist
    // copied from addPeer
    checkAndInsertPeer(hostAddress, port);
    return RTN_OK;
}

void ChatDialog::checkAndInsertPeer(QHostAddress address, quint16 port)
{
    //@@@ check if the sender is a new peer,
    // if so, add into the peerList
    bool newPeer = TRUE;
    foreach (Peer pr, *(this->peerList))
    {
        if ((pr.getIP() == address) && (pr.getPort() == port))
        {
            // pr already in the list
            newPeer = FALSE;
            break;
        }
    }
    if (TRUE == newPeer)
    {
        // insert into peerList
        Peer peer (address, port);
        peerList->append(peer);
        showPeers();
        // @@@ test
        //qDebug() << address.toString() << " " << QString::number(port);
    }

}

int ChatDialog::checkDiagramType(QByteArray byteArray)
{
    QVariantMap receivedMap;
    QDataStream msgStream(&byteArray, QIODevice::ReadWrite);
    msgStream >> receivedMap;

    if (TRUE == receivedMap.contains("Want"))
    {
        return MTYPE_STATUS;
    }
    else if ((TRUE == receivedMap.contains("ChatText")) &&
             (TRUE == receivedMap.contains("Origin")) &&
             (TRUE == receivedMap.contains("SeqNo")) )
    {
        return MTYPE_CHAT_RUMMOR;
    }
    else if ((TRUE == receivedMap.contains("Origin")) &&
             (TRUE == receivedMap.contains("SeqNo")) &&
             (FALSE == receivedMap.contains("ChatText")) )
    {
        return MTYPE_ROUTE_RUMMOR;
    }
    else if ((TRUE == receivedMap.contains("Dest")) &&
             (TRUE == receivedMap.contains("ChatText")) &&
             (TRUE == receivedMap.contains("HopLimit")))
    {
        return MTYPE_PRIVATE;
    }
    else if ((TRUE == receivedMap.contains("Dest")) &&
             (TRUE == receivedMap.contains("Origin")) &&
             (TRUE == receivedMap.contains("HopLimit")) &&
             (TRUE == receivedMap.contains("BlockRequest")))
    {
        return MTYPE_BLOCK_REQUEST;
    }
    else if ((TRUE == receivedMap.contains("Dest")) &&
             (TRUE == receivedMap.contains("Origin")) &&
             (TRUE == receivedMap.contains("HopLimit")) &&
             (TRUE == receivedMap.contains("BlockReply")) &&
             (TRUE == receivedMap.contains("Data")))
    {
        return MTYPE_BLOCK_REPLY;
    }
    else if ((TRUE == receivedMap.contains("Origin")) &&
             (TRUE == receivedMap.contains("Search")) &&
             (TRUE == receivedMap.contains("Budget")))
    {
        return MTYPE_SEARCH_REQUEST;
    }
    else if ((TRUE == receivedMap.contains("Dest")) &&
             (TRUE == receivedMap.contains("Origin")) &&
             (TRUE == receivedMap.contains("HopLimit")) &&
             (TRUE == receivedMap.contains("SearchReply")) &&
             (TRUE == receivedMap.contains("MatchNames")) &&
             (TRUE == receivedMap.contains("MatchIDs")))
    {
        return MTYPE_SEARCH_REPLY;
    }
    else if ((TRUE == receivedMap.contains("FileAsked")) &&
             (TRUE == receivedMap.contains("ServerIP")) &&
             (TRUE == receivedMap.contains("ServerPort")) &&
             (TRUE == receivedMap.contains("Origin")) )
    {
        return MTYPE_WHOLIKE_RQST;
    }
    else if ((TRUE == receivedMap.contains("OriginList")) &&
             (TRUE == receivedMap.contains("FileID")) &&
             (TRUE == receivedMap.contains("NumOfLike")) &&
             (TRUE == receivedMap.contains("Dest")) &&
             (TRUE == receivedMap.contains("Origin")) )
    {
        return MTYPE_WHOLIKE_REPLY;
    }
    else if ((TRUE == receivedMap.contains("Like")) &&
             (TRUE == receivedMap.contains("FileID")) &&
             (TRUE == receivedMap.contains("ServerIP")) &&
             (TRUE == receivedMap.contains("ServerPort")) &&
             (TRUE == receivedMap.contains("Origin")) )
    {
        return MTYPE_LIKE_FILE;
    }
    else
    {
        return MTYPE_ERR;
    }
}


int ChatDialog::gotStatusDatagram(QByteArray statusByteArray,
                                  QHostAddress sender, quint16 senderPort)
{

    // decode the status
    QVariantMap statusMap;
    QDataStream msgStream(&statusByteArray, QIODevice::ReadOnly);
    msgStream >> statusMap;
    // check availability and decode
    // already checked that contain "Want"
    // want map should already contain <origin, seq_needed>
    QVariantMap wantMap = statusMap.value("Want").toMap();

    //@@@ what if the wantMap is bad??

    // compare vector, statusMap and dbStatus
    // see if I have msg target does not have
    QMap<QString, QVariantMap> difference =
            this->msgDatabase->compareStatusMap(statusMap);
    // if so, send one msg that target does not have to target

    // CASE 1: i have sth target do not have
    //         - send one message to target, and wait
    if (difference.contains("NextToGiveTarget"))
    {
        //@@@ qDebug() << "I have more.";
        // pick the first message in the map
        QVariantMap tmpMap = difference.value("NextToGiveTarget"); 
        QVariantMap::const_iterator iter1 = tmpMap.begin();
        QString needOrigin;
        quint32 needSeqNo;
        QVariantMap msgSend;

        needOrigin = iter1.key();
        needSeqNo = tmpMap.value(needOrigin).toUInt(0);
        // @@@ get message frmakeom local database
        msgSend = getMsgInfo(needOrigin, needSeqNo);

        // need to check the message
        if (FALSE == msgIsValid(msgSend))
        {
            qDebug() << "bad message";
            return RTN_INVALID_MSG_ERR;
        }

        // if no forward mode, only send route rummor
        if ((TRUE == this->noForwardMode) && (TRUE == msgSend.contains("ChatText")))
        {
            // don't send
        }
        else
        {
            //send the message
            sendMsg(msgSend, sender, senderPort);
            // wait
            //@@@ timer_1s->start(TIMER_LENGTH);
        }


    }
    // CASE 2: target have sth i don't have
    //         - send my status
    else if (difference.contains("NextNeedFromTarget"))
    {
        // @@@ qDebug() << "Target have more";
        QVariantMap statusMessage = this->msgDatabase->getDBStatus();
        sendMsg(statusMessage, sender, senderPort);
    }
    // CASE 3: me and target msg same
    else
    {
        // @@@ qDebug() << "same status";
        //@@@ stop, maybe flipcoin
        flipCoin();

    }

    return RTN_OK;
}

// need to check return msg
QVariantMap ChatDialog::getMsgInfo(QString origin, quint32 seq)
{
    QVariantMap msg_rtn;
    // QMap < QString, QList< QVariantMap > > *database;

    //@@@ Check if the database does not have this message
    if (FALSE == this->msgDatabase->getDataBase()->contains(origin))
    {
        qDebug() << "ERROR: no such origin in database.";
        return msg_rtn;

    }

    // look for the list that contains message from origin
    QList<QVariantMap> msg_list =
            this->msgDatabase->getDataBase()->value(origin);

    // look for the message with the SeqNo in the list
    foreach (QVariantMap msgItem, msg_list)
    {
        // @@@ do we need to check message here??
        // the database should be good here

        // look for message with right seqNo,
        // also check its origin
        if (msgItem.value("SeqNo") == seq)
        {
            if ((msgItem.value("Origin") != origin)) // used to check if has ChatText
            {
                //@@@ got bad message in database, need to reorganize
                qDebug() << "ERROR: bad message in database.";
                //@@@ to do
                return msg_rtn;
            }
            // got message we want
            msg_rtn = msgItem;
        }

        //@@@ check if it is a good message before return, maybe do this elsewhere
    }
    return msg_rtn;

}


bool ChatDialog::msgIsValid(QVariantMap msg)
{
    if ((FALSE == msg.contains("Origin")) ||
            (FALSE == msg.contains("SeqNo")) )
    {
        return FALSE;
    }

    return TRUE;
}

//
int ChatDialog::processChatRummorDatagram(QByteArray msgByteArray,
                                   QHostAddress sender, quint16 senderPort)
{
    //decode Msg
    QVariantMap msgMap; // the message we got
    QString msgTextStr; // output msg str
    QString msgOriginStr;
    quint32 msgSeqUint;
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);

    msgStream >> msgMap;

    //Check if message is valid
    if (FALSE == msgIsValid(msgMap))
    {
        textview->append("TEST: Got invalid message"); //@@@ testcode
        qDebug() << "got invalid message";
        return RTN_INVALID_MSG_ERR;
    }

    msgTextStr = msgMap.value("ChatText").toString();
    msgOriginStr = msgMap.value("Origin").toString();
    msgSeqUint = msgMap.value("SeqNo").toUInt();

    rummorCheckLast_addToPeers(msgMap);

    // part 5 insert LastIP and LastPort, before insert
    insertRummorMsgLastIPPort(&msgMap, sender, senderPort);


    //check if it is my own message??
    //No need, since my own message is already shown in my dbStatus

    // if it is not my message
    // check if need to receive, receive if needed
    int statusCheckResult = msgDatabase->checkMsginDBStatus(msgMap);
    if (statusCheckResult == MSG_ST_EXACTLY_NEEDED)
    {
        qDebug() << "Exactly need " << msgTextStr;
        // if new, save into the database
        // update the dbstatus
        this->msgDatabase->insertMsg(msgMap);

        // show the output on the chat dialog
        textview->append(msgTextStr);

        // when new message received and inserted, update route table
        insertRoute(msgOriginStr, sender, senderPort);

        // if noforward mode, do not send chat rummor
        if (FALSE == this->noForwardMode)
        {
            //Got New Message, send!
            afterSavedNewMsg_sendToOneNB(msgMap);
        }


    }
    else if (statusCheckResult == MSG_ST_DB_EMPTY)
    {

    }
    else if (statusCheckResult == MSG_ST_DB_NO_SUCH_ORIGIN)
    {

    }
    else if (statusCheckResult == MSG_ST_ALREADY_RECEIVED)
    {
        // part6, if it contains no LastIp and LastPort -- direct route
        // if it is direct route, override the indirect one with the new direct one
        // update, so no show
        this->msgDatabase->updateDirectRoute(msgMap, sender, senderPort);
    }

    else if (statusCheckResult == MSG_ST_RECEIVE_LATER)
    {
         insertRoute(msgOriginStr, sender, senderPort);
    }


    //@@@ send my status message to the sender
    quint16 localPN = this->netSocket->getLocalPortNumber();
    if ((sender != QHostAddress::LocalHost) ||(senderPort != localPN))
    {
        //@@@ test
        //this->msgDatabase->printStatusMessage(textview);
        sendMsg(this->msgDatabase->getDBStatus(), sender, senderPort);
    }

    return RTN_OK;

}


// simmilar to process RummorDatagram without printout the process
int ChatDialog::processRouteRummorDatagram(QByteArray msgByteArray, QHostAddress sender, quint16 senderPort)
{
    //decode Msg
    QVariantMap msgMap; // the message we got
    QString msgOriginStr;
    quint32 msgSeqUint;
    QDataStream msgStream(&msgByteArray, QIODevice::ReadOnly);
    msgStream >> msgMap;

    msgOriginStr = msgMap.value("Origin").toString();
    msgSeqUint = msgMap.value("SeqNo").toUInt();


    rummorCheckLast_addToPeers(msgMap);

    // update or insert msg LastIP and LastPort, before insert
    insertRummorMsgLastIPPort(&msgMap, sender, senderPort);



    // if it is not my message
    // check if need to receive, receive if needed
    int statusCheckResult = msgDatabase->checkMsginDBStatus(msgMap);
    if (statusCheckResult == MSG_ST_EXACTLY_NEEDED)
    {

        // save it into the database
        this->msgDatabase->insertMsg(msgMap);

        // when new message received and inserted, update route table
        //@@@ insert for in forward mode???
        insertRoute(msgOriginStr, sender, senderPort);

        //new route message forward to all peers
        sendRouteToAllPeers(msgMap);
        //afterSavedNewMsg_sendToOneNB(msgMap);

    }
    else if (statusCheckResult == MSG_ST_ALREADY_RECEIVED)
    {
        // part6, if it contains no LastIp and LastPort -- direct route
        // if it is direct route, override the indirect one with the new direct one
        // update so no show
        this->msgDatabase->updateDirectRoute(msgMap, sender, senderPort);
    }
    else if (statusCheckResult == MSG_ST_RECEIVE_LATER)
    {
        // to do
         insertRoute(msgOriginStr, sender, senderPort);
    }

    //@@@ send my status message to the sender
    quint16 localPN = this->netSocket->getLocalPortNumber();
    if ((sender != QHostAddress::LocalHost) ||(senderPort != localPN))
    {
        //@@@ test
        //this->msgDatabase->printStatusMessage(textview);
        sendMsg(this->msgDatabase->getDBStatus(), sender, senderPort);
    }

    return RTN_OK;


}

void ChatDialog::sendMsg(QVariantMap msg, QHostAddress tarAddr, quint16 tarPort)
{
    QByteArray msgBA = createByteArrayFromMsg(msg);
    // @@@ qhost
    netSocket->writeDatagram(msgBA.data(), msgBA.size(),tarAddr, tarPort);
    //@@@ testcode qDebug()<< "send msg to <" <<tarAddr.toString() << ":"<< QString::number(tarPort) << ">";
}

//insert or overwrite the current LastIP and LastPort
void ChatDialog::insertRummorMsgLastIPPort(QVariantMap *msgptr, QHostAddress lasthost, quint16 lastport)
{
    quint32 lastip = lasthost.toIPv4Address();
    msgptr->insert("LastIP", lastip);
    msgptr->insert("LastPort", lastport);
}

/**********************************************************************
 * Class Name: NetSocket
 */

NetSocket::NetSocket()
{
    // Pick a range of four UDP ports to try to allocate by default,
    // computed based on my Unix user ID.
    // This makes it trivial for up to four Peerster instances per user
    // to find each other on the same host,
    // barring UDP port conflicts with other applications
    // (which are quite possible).
    // We use the range from 32768 to 49151 for this purpose.
    QTime time = QTime::currentTime();
    myPortMin = 32299 + (getuid() % 4096)*4 +
                (time.msec() * time.second() * time.minute() % 100);
    myPortMax = myPortMin + 3;

    hostName = QHostInfo::localHostName();

}


int NetSocket::getMyPortMin()
{
    return this->myPortMin;
}

int NetSocket::getMyPortMax()
{
    return this->myPortMax;
}

QString NetSocket::getLocalHostName()
{
    return hostName;
}

quint16 NetSocket::getLocalPortNumber()
{
    return portNumber;
}

QString NetSocket::getSocketOriginName()
{
    return this->socketOriginName;
}

void NetSocket::setSocketOriginName()
{
    QString originName;
    originName.append(this->hostName);
    originName.append("_");
    originName.append(QString::number(this->portNumber));
    originName.append("_wyy");
    this->socketOriginName = originName;
}
bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
    for (quint16 p = myPortMin; p <= myPortMax; p++)
    {
        if (QUdpSocket::bind(p))
        {
			qDebug() << "bound to UDP port " << p;
            portNumber= p;
            setSocketOriginName();
			return true;
		}
	}

	qDebug() << "Oops, no ports in my default range " << myPortMin
		<< "-" << myPortMax << " available";
	return false;
}

QHostAddress NetSocket::getLocalIP()
{
    QHostInfo hostInfo = QHostInfo::fromName(QHostInfo::localHostName());
    QHostAddress myIP = hostInfo.addresses().at(0);
    return myIP;
}


/**********************************************************************
 * Class Name: PrivateD()
 */
PrivateD::PrivateD()
{
    title = new QLabel(this);
    title->setText("Private with ");

    textChat = new QTextEdit(this);
    textInput = new QTextEdit(this);
    textChat->setReadOnly(true);
    textInput->setReadOnly(false);
    textInput->installEventFilter(this);

    QVBoxLayout *pLayout = new QVBoxLayout();
    pLayout->addWidget(title);
    pLayout->addWidget(textChat);
    pLayout->addWidget(textInput);
    setLayout(pLayout);

}

bool PrivateD::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return)
        {
            p_GotReturnPressed();
            return true;
        }
        return false;
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }
}

void PrivateD::p_GotReturnPressed()
{
    // read the input
    QString input = this->textInput->toPlainText();
    // clear the input
    textInput->clear();
    // show
    QString showInput = "I say: ";
    showInput.append(input);
    textChat->append(showInput);
    // create private message

    // send it to somewhere

    emit gotPrivateInput(this->targetOrigin, input);  //@@@ signals???
}

// show the passed in message in the box
void PrivateD::p_showMessage(QString chatTxt)
{
    //@@@ to do
    this->textChat->append(chatTxt);

}

void PrivateD::p_setPrivateOrigin(QString origin)
{
    this->targetOrigin = origin;
    QString titletxt = "Talking to ";
    titletxt.append(origin);
    this->title->setText(titletxt);
}

/**********************************************************************
 * Class Name: Main
 */

int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);
    // initialize QCA
    QCA::Initializer qcainit;

	// Create an initial chat dialog window
    ChatDialog dialog;
	dialog.show();

	// Create a UDP network socket

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}



