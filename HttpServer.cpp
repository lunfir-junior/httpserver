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

  QTextStream mime(&content);
  while(!mime.atEnd()) {
    QString line = mime.readLine().trimmed();
    if ( !line.isEmpty() ) {
      QStringList list = line.split(QString("  "), QString::SkipEmptyParts);
      m_contentTypes.insert(list.at(0), list.at(1));
    }
  }
  content.close();

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
  QString responseFileName, contentType, statusCode;
  int contentLength;

  if ( data.at(0) != QString("GET").toUtf8() ) {
    responseFileName = QString("405.html");
    response.setFileName(path.append(responseFileName));
    response.open(QIODevice::ReadOnly);
    content = response.readAll();
    contentLength = content.size();
    statusCode = QString("405 Method Not Allowed");
    response.close();
  } else {
    responseFileName = QString(data.at(1));
    responseFileName.remove(0, 1); // remove '/'

    if ( responseFileName.isEmpty() )
      responseFileName = QString("index.html");

    response.setFileName(path.append(responseFileName));
    if ( !response.open(QIODevice::ReadOnly) ) {
      path = m_settings.value("root_dir");
      responseFileName = QString("404.html");
      response.setFileName(path.append(responseFileName));
      response.open(QIODevice::ReadOnly);
      content = response.readAll();
      contentLength = content.size();
      statusCode = QString("404 Not Found");
      response.close();
    } else {
      content = response.readAll();
      contentLength = content.size();
      statusCode = QString("200 OK");
      response.close();
    }
  }

  QString tmp = responseFileName.split('.').at(1);

  if ( m_contentTypes.keys().contains(tmp) )
    contentType = m_contentTypes.value(tmp);
  else
    contentType = QString("application/octet-stream");

  QByteArray bytes;
  bytes.push_back(QString("HTTP/1.1 ").toUtf8());
  bytes.push_back(statusCode.toUtf8());
  bytes.push_back(QString("\nConnection: close\nContent-Length: ").toUtf8());
  bytes.push_back(QString::number(contentLength).toUtf8());
  bytes.push_back(QString("\nContent-Type: ").toUtf8());
  bytes.push_back(contentType.toUtf8());
  bytes.push_back(QString("\nDate: ").toUtf8());
  bytes.push_back(QDateTime::currentDateTime().toString().toUtf8() );
  bytes.push_back(QString("\r\n\r\n").toUtf8());
  bytes.push_back(content);

  socket->write(bytes);
  socket->disconnectFromHost();

//  qDebug().noquote() << bytes;
}

void HttpServer::slotDisconnected()
{
  QTcpSocket *socket = (QTcpSocket*) sender();
  socket->close();
  socket->deleteLater();
}


