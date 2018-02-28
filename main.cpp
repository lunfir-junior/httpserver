#include <QCoreApplication>
#include <HttpServer.h>

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  HttpServer server;

  QObject::connect(&server, &HttpServer::signalDestroyed, &a, &QCoreApplication::quit);

  return a.exec();
}
