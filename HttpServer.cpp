#include "HttpServer.h"

HttpServer::HttpServer(QObject *parent) : QTcpServer(parent)
{
  QFile config(QCoreApplication::applicationDirPath().append("/http.conf"));
  QFile content(QCoreApplication::applicationDirPath().append("/mime.types"));

  if ( !config.open(QIODevice::ReadOnly) )
      qDebug() << "config error: ", config.errorString();

  QTextStream in(&config);
  while(!in.atEnd()) {
    QString line = in.readLine().trimmed();
    QStringList list = line.split(' ', QString::SkipEmptyParts);
    m_settings.insert(list.at(0), list.at(1));
  }
  config.close();

  if ( !content.open(QIODevice::ReadOnly) )
      qDebug() << "content error: ", content.errorString();

//  QTextStream mime(&content);
//  while(!mime.atEnd()) {
//    QString line = mime.readLine().trimmed();
//    QStringList list = line.split(' ', QString::SkipEmptyParts);
//    m_contentTypes.insert(list.at(0), list.at(1));
//  }
//  content.close();

  if ( this->listen(QHostAddress(m_settings.value("address")), m_settings.value("port").toInt()) )
    qDebug() << "server is listening...";
  else
    qDebug() << "error: " << this->errorString();
}

void HttpServer::incomingConnection(qintptr socketDescriptor)
{
  QTcpSocket *socket = new QTcpSocket();

  socket->setSocketDescriptor(socketDescriptor);

  connect(socket, &QTcpSocket::readyRead, this, &HttpServer::slotReadyRead);
}

void HttpServer::slotReadyRead()
{
  QTcpSocket *socket = (QTcpSocket*) sender();
  QList<QByteArray> data = socket->readAll().split(' ');
  QString path = m_settings.value("root_dir");
  QByteArray content;
  QFile response;
  QString responseFileName;
  int contentLength;

  if ( data.at(0) != QString("GET").toUtf8() ) {
    response.setFileName(path.append("405.html"));
    response.open(QIODevice::ReadOnly);
    content = response.readAll();
    contentLength = content.size();
    response.close();
  } else {
    responseFileName = QString(data.at(1));
    responseFileName.remove(0, 1); // remove '/'

    if ( responseFileName.isEmpty() )
      responseFileName = QString("index.html");

    response.setFileName(path.append(responseFileName));
    if ( !response.open(QIODevice::ReadOnly) ) {
      path = m_settings.value("root_dir");
      response.setFileName(path.append("404.html"));
      response.open(QIODevice::ReadOnly);
      content = response.readAll();
      contentLength = content.size();
      response.close();
    } else {
      content = response.readAll();
      contentLength = content.size();
      response.close();
    }
  }

//  QString response = "HTTP/1.1 200 OK\nConnection: close\n";
//  response.append("Content-Length: " + QString::number(contentLength) + "\n");
//  response.append("Content-Type: text/html; charset=UTF-8\n");
//  response.append("Date: " + QDateTime::currentDateTime().toString() + "\r\n\r\n");

  socket->write(content);
  socket->disconnectFromHost();


//  qDebug().noquote() << response;
}

void HttpServer::slotDisconnected()
{
  QTcpSocket *socket = (QTcpSocket*) sender();
  socket->close();
  socket->deleteLater();
}


