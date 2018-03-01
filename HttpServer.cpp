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
    m_settings.insert(list.at(0), list.at(1));
  }
  config.close();

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
  int contentLength;

  if ( data.at(0) != QString("GET").toUtf8() ) {
    QFile resp(path.append("405.html"));
    content = resp.readAll();
    socket->write(content);
    contentLength = content.size();
    resp.close();

    return;
  }

  QString responseFileName = QString(data.at(1));
  responseFileName.remove(0, 1); // remove '/'

  if ( responseFileName.isEmpty() )
    responseFileName = QString("index.html");

  QFile tryResponse(path.append(responseFileName));
  content.clear();
  if ( !tryResponse.open(QIODevice::ReadOnly) ) {
    path = m_settings.value("root_dir");

    QFile resp(path.append("404.html"));
    resp.open(QIODevice::ReadOnly);
    content = resp.readAll();
    socket->write(content);
    contentLength = content.size();
    resp.close();
  } else {
    content = tryResponse.readAll();
    socket->write(content);
    contentLength = content.size();
  }
  tryResponse.close();
  socket->disconnectFromHost();

  QString response = "HTTP/1.1 200 OK\nConnection: close\n";
  response.append("Content-Length: " + QString::number(contentLength) + "\n");
  response.append("Content-Type: text/html; charset=UTF-8\n");
  response.append("Date: " + QDateTime::currentDateTime().toString() + "\n");

  qDebug().noquote() << response;
}

void HttpServer::slotDisconnected()
{
  QTcpSocket *socket = (QTcpSocket*) sender();
  socket->close();
  socket->deleteLater();
}


