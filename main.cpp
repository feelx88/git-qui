#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QQmlContext>

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

  qmlRegisterType<GitFile>("de.feelx88.GitFile", 1, 0, "GitFile");
  qmlRegisterType<GitDiffLine>("de.feelx88.GitDiffLine", 1, 0, "GitDiffLine");

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("gitManager", &manager);
  engine.load(QUrl(QLatin1String("qrc:/main.qml")));

  return app.exec();
}
