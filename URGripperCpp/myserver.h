#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>
#include <vector>
#include <string>

class MyServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyServer(QObject *parent = nullptr);
    void startServer();
    bool hasNewData();
    std::string getNextData();
signals:

public slots:

protected:
    void incomingConnection(qintptr socketDescriptor);

private:
    std::vector<std::string> dataVec_;

};

#endif // MYSERVER_H
