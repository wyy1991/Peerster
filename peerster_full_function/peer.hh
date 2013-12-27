#ifndef PEER_HH
#define PEER_HH

#include <QVariant>
#include <QHostInfo>
#include <QDialog>



//******************************************************
class Peer
{

public:
    Peer();
    Peer(QHostAddress ip, quint16 port);
    QHostAddress getIP();
    quint16 getPort();
    void setIP(QHostAddress ip);
    void setPort(quint16 port);
    bool equals(Peer p);
public slots:


private:
    QHostAddress ipAddress;
    quint16 portNumber;
};


#endif // PEER_HH
