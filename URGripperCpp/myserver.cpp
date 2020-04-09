#include "myserver.h"
#include "mythread.h"

MyServer::MyServer(QObject *parent) :
    QTcpServer(parent)
{
}

void MyServer::startServer()
{
    int port = 1234;

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

    // Forbind signal/slot
    // Når en thread ikke bliver brugt mere, bliver den slettet
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}
