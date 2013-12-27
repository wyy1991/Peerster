#include "peer.hh"

Peer::Peer()
{

}

Peer::Peer(QHostAddress ip, quint16 port)
{
    this->ipAddress = ip;
    this->portNumber = port;
}

QHostAddress Peer::getIP()
{
    return this->ipAddress;
}

quint16 Peer::getPort()
{
    return this->portNumber;
}

void Peer::setIP(QHostAddress ip)
{
    this->ipAddress = ip;
}

void Peer::setPort(quint16 port)
{
    this->portNumber = port;
}

bool Peer::equals(Peer p)
{

    if ((this->portNumber == p.portNumber) &&
            (this->ipAddress == p.ipAddress))
    {
        return true;
    }
    else
    {
        return false;
    }

}


