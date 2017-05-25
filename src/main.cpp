#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QSettings>

#include <git/gitfile.h>
#include <git/gitdiffline.h>
#include <git/libgit2/libgit2gitmanager.h>
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

  // A boolean option with a single name (-p)
  QCommandLineOption gitImpl({"L", "libgit2"}, "Use libgit2 implementation instead of parsing git output");
  parser.addOption(gitImpl);

  parser.process(app);

  AGitManager *manager = nullptr;

  if(parser.isSet(gitImpl))
  {
    manager = new libgit2::GitManager(&app);
  }
  else
  {
    manager = new gitBin::GitManager(&app);
  }

  manager->connect(manager, &AGitManager::gitError, [&](const QString &message){
    std::cout << message.toStdString() << std::endl;
  });

  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, manager->repositoryRoot(QDir::currentPath()) + "/.git");

  qmlRegisterType<GitFile>("de.feelx88.GitFile", 1, 0, "GitFile");
  qmlRegisterType<GitDiffLine>("de.feelx88.GitDiffLine", 1, 0, "GitDiffLine");

  QQuickStyle::setFallbackStyle("Material");

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("gitManager", manager);
  engine.load(QUrl(QCoreApplication::applicationDirPath() + QLatin1String("/ui/default/main.qml")));

  manager->init();
  manager->openRepository(QDir::currentPath());

  return app.exec();
}
