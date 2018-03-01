#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
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

private:
  QMap<QString, QString> m_settings;
  QMap<QString, QString> m_contentTypes;

public slots:
  void slotReadyRead();
  void slotDisconnected();
};

#endif // HTTPSERVER_H
