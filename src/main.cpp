#include <iostream>

#include <QApplication>
#include <QSettings>

#include "core.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"

int main(int argc, char *argv[])
{
  QSettings::setDefaultFormat(QSettings::IniFormat);

  qRegisterMetaType<QList<GitFile>>();
  qRegisterMetaType<QList<GitCommit>>();
  qRegisterMetaType<QList<GitBranch>>();
  qRegisterMetaTypeStreamOperators<QList<QVariantMap>>();

  QApplication app(argc, argv);
  app.setOrganizationName("feelx88");
  app.setOrganizationDomain("feelx88.de");

  Core core;

  return app.exec();
}
