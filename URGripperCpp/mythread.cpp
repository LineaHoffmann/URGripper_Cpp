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

void MyThread::readyRead()
{
    // Læser information
    QByteArray Data = socket->readAll();

    // Svarer i vinduet
    qDebug() << socketDescriptor << " Data in: " << Data;

    socket->write(Data);
}

void MyThread::disconnected()
{
    qDebug() << socketDescriptor << " Disconnected";


    socket->deleteLater();
    exit(0);
}
