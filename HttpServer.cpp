#include "HttpServer.h"

HttpServer::HttpServer(QObject *parent) : QTcpServer(parent)
{
  QFile config(QCoreApplication::applicationDirPath().append("/http.conf"));
  QMap<QString, QString> settings;

  if ( !config.open(QIODevice::ReadOnly) )
      qDebug() << "config error: ", config.errorString();

  QTextStream in(&config);
  while(!in.atEnd()) {
    QString line = in.readLine().trimmed();
    QStringList list = line.split(' ', QString::SkipEmptyParts);
    settings.insert(list.at(0), list.at(1));
  }
  config.close();

  if ( this->listen(QHostAddress(settings.value("address")), settings.value("port").toInt()) )
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


