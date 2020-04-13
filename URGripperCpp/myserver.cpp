#include "myserver.h"
#include "mythread.h"

MyServer::MyServer(QObject *parent) :
    QTcpServer(parent)
{
}

void MyServer::startServer()
{
    quint16 port = 12345;

    if(!this->listen(QHostAddress::Any, port))
    {
        qDebug() << "Could not start server";
    }
    else
    {
        qDebug() << "Listening to port " << port << "...";
    }
}

// Denne funktion kaldes af  QTcpServer når der er en ny forbindelse tilgængelig
void MyServer::incomingConnection(qintptr socketDescriptor)
{
    // Ny forbindelse
    qDebug() << socketDescriptor << " Connecting...";

    // Hver forbindelse vil blive kørt i sin egen nye thread
    MyThread *thread = new MyThread(socketDescriptor, this);
    if (thread->isFinished()) {
        dataVec_.push_back(thread->readyRead());


    }

    // Forbind signal/slot
    // Når en thread ikke bliver brugt mere, bliver den slettet
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}

std::string MyServer::getNextData() {
    auto sStart = dataVec_.begin();
    auto sStr = *sStart;
    dataVec_.erase(sStart);
    return sStr;
}
bool MyServer::hasNewData() {return !dataVec_.empty();}
