#include "mythread.h"

MyThread::MyThread(qintptr ID, QObject *parent) :
    QThread(parent)
{
    this->socketDescriptor = ID;
}

void MyThread::run()
{
    // Thread starter her
    qDebug() << " Thread started";

    socket = new QTcpSocket();

    // set the ID
    if(!socket->setSocketDescriptor(this->socketDescriptor))
    {
        // Lidt error handling, hvis der ikke opstår forbindelse
        emit error(socket->error());
        return;
    }

    // connect socket and signal

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));


    qDebug() << socketDescriptor << " Client connected";


    exec();
}

std::string MyThread::readyRead()
{
    // Reads byte data
    QByteArray Data = socket->readAll();
    // Sends aknowledgement
    socket->write("HALT");
    return QString(Data).toStdString();
}

void MyThread::disconnected()
{
    socket->deleteLater();
    exit(0);
}
