#include <iostream>

#include <QApplication>
#include <QSettings>

#include "mainwindow.hpp"
#include "gitinterface.hpp"
#include "repository.hpp"

int main(int argc, char *argv[])
{
  QSettings::setDefaultFormat(QSettings::IniFormat);

  qRegisterMetaType<QList<GitFile>>();
  qRegisterMetaType<QList<GitCommit>>();
  qRegisterMetaType<QList<GitBranch>>();
  qRegisterMetaType<QList<Repository>>();
  qRegisterMetaTypeStreamOperators<QList<Repository>>();

  QApplication app(argc, argv);
  app.setOrganizationName("feelx88");
  app.setOrganizationDomain("feelx88.de");

  MainWindow w;
  w.show();

  return app.exec();
}
