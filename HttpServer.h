#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>

class HttpServer : public QTcpServer
{
  Q_OBJECT

public:
  explicit HttpServer(QObject *parent = nullptr);

protected:
  void incomingConnection(qintptr socketDescriptor);

signals:
  void signalDestroyed();

public slots:
  void slotReadyRead();

};

#endif // HTTPSERVER_H
