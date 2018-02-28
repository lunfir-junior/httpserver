#include "HttpServer.h"

HttpServer::HttpServer(QObject *parent) : QTcpServer(parent)
{
  if ( this->listen(QHostAddress::LocalHost, 80) )
    qDebug() << "server is listening...";
  else
    qDebug() << "error: " << this->errorString();
}

void HttpServer::incomingConnection(qintptr socketDescriptor)
{
  QTcpSocket *socket = new QTcpSocket()/*this->nextPendingConnection()*/;

  socket->setSocketDescriptor(socketDescriptor);

  connect(socket, &QTcpSocket::readyRead, this, &HttpServer::slotReadyRead);
}

void HttpServer::slotReadyRead()
{
  QTcpSocket *socket = (QTcpSocket*) sender();
  QByteArray data = socket->readAll();

  qDebug().noquote() << "client data: " << data;
}


