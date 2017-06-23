#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QSettings>
#include <QFileSystemWatcher>

#include <git/gitfile.h>
#include <git/gitdiffline.h>
#include <git/gitcommit.h>
#include <git/git-bin/gitbingitmanager.h>

int main(int argc, char *argv[])
{
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QCoreApplication::setOrganizationDomain("feelx88.de");
  QCoreApplication::setApplicationName("git-qui");
  QCoreApplication::setApplicationVersion("0.1.0");
  QGuiApplication app(argc, argv);

  QCommandLineParser parser;
  parser.setApplicationDescription("QML UI for git.");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("<working directory>", "Git working directory.");

  parser.process(app);

  AGitManager *manager = new gitBin::GitManager(&app);

  manager->connect(manager, &AGitManager::gitError, [&](const QString &message){
    std::cout << message.toStdString() << std::endl;
  });

  qmlRegisterType<GitFile>("de.feelx88.GitFile", 1, 0, "GitFile");
  qmlRegisterType<GitDiffLine>("de.feelx88.GitDiffLine", 1, 0, "GitDiffLine");
  qmlRegisterType<GitCommit>("de.feelx88.GitCommit", 1, 0, "GitCommit");

  QQmlApplicationEngine engine;
  QPM_INIT(engine);
  engine.load(QUrl(QLatin1String("qrc:/qml/ui/default/main.qml")));

  return app.exec();
}
