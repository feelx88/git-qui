#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QQmlContext>
#include <QQuickStyle>

#include <git/gitfile.h>
#include <git/gitdiffline.h>
#include <git/libgit2/gitmanager.h>

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication app(argc, argv);

  GitManager manager(&app);
  manager.connect(&manager, &GitManager::gitError, [&](const QString &message){
    std::cout << message.toStdString() << std::endl;
  });
  manager.init();
  manager.openRepository(QDir::currentPath());

  qmlRegisterType<GitFile>("de.feelx88.GitFile", 1, 0, "GitFile");
  qmlRegisterType<GitDiffLine>("de.feelx88.GitDiffLine", 1, 0, "GitDiffLine");

  QQuickStyle::setFallbackStyle("Material");

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("gitManager", &manager);
  engine.load(QUrl(QCoreApplication::applicationDirPath() + QLatin1String("/ui/default/main.qml")));

  return app.exec();
}
