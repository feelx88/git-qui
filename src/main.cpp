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

#if defined(USE_LIBIGT2)
  #include <git/libgit2/libgit2gitmanager.h>
#endif

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

#if defined (USE_LIBIGT2)
  // A boolean option with a single name (-p)
  QCommandLineOption gitImpl({"L", "libgit2"}, "Use libgit2 implementation instead of parsing git output");
  parser.addOption(gitImpl);
#endif

  parser.process(app);

  AGitManager *manager = nullptr;

#if defined (USE_LIBIGT2)
  if(parser.isSet(gitImpl))
  {
    manager = new libgit2::GitManager(&app);
  }
  else
#endif
  {
    manager = new gitBin::GitManager(&app);
  }

  manager->connect(manager, &AGitManager::gitError, [&](const QString &message){
    std::cout << message.toStdString() << std::endl;
  });

  QString repositoryRoot = manager->repositoryRoot(QDir::currentPath());
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, repositoryRoot + "/.git");

  qmlRegisterType<GitFile>("de.feelx88.GitFile", 1, 0, "GitFile");
  qmlRegisterType<GitDiffLine>("de.feelx88.GitDiffLine", 1, 0, "GitDiffLine");
  qmlRegisterType<GitCommit>("de.feelx88.GitCommit", 1, 0, "GitCommit");

  QQuickStyle::setFallbackStyle("Material");

  QFileSystemWatcher watcher;

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("gitManager", manager);
  engine.rootContext()->setContextProperty("watcher", &watcher);
  engine.load(QUrl(QCoreApplication::applicationDirPath() + QLatin1String("/ui/default/main.qml")));

  manager->init();
  manager->openRepository(QDir::currentPath());

  watcher.addPaths(manager->repositoryFiles());
  QObject::connect(&watcher, &QFileSystemWatcher::fileChanged, [&](const QString &path){
    watcher.addPath(path);
  });

  watcher.addPath(repositoryRoot);
  QObject::connect(&watcher, &QFileSystemWatcher::directoryChanged, [&]{
    watcher.addPaths(manager->repositoryFiles());
  });

  return app.exec();
}
