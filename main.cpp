#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>

#include <gitmanager.h>

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  GitManager manager(nullptr);
  manager.connect(&manager, &GitManager::gitError, [&](const QString &message){
    std::cout << message.toStdString() << std::endl;
  });
  manager.init();
  manager.openRepository(QDir::currentPath());

  QQmlApplicationEngine engine;
  engine.load(QUrl(QLatin1String("qrc:/main.qml")));

  return app.exec();
}
