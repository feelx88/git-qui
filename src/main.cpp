#include <iostream>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>
#include <QQmlContext>
#include <QQuickStyle>
#include <QCommandLineParser>
#include <QSettings>
#include <QFileSystemWatcher>

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

  QQmlApplicationEngine engine;
  QPM_INIT(engine);
  engine.load(QUrl(QLatin1String("qrc:/qml/ui/main.qml")));

  return app.exec();
}
