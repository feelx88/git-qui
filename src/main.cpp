#include <iostream>

#include <QApplication>
#include <QSettings>

#include "mainwindow.hpp"
#include "gitinterface.hpp"

int main(int argc, char *argv[])
{
  QSettings::setDefaultFormat(QSettings::IniFormat);

  qRegisterMetaType<QList<GitFile>>();
  qRegisterMetaType<QList<GitCommit>>();

  QApplication app(argc, argv);
  app.setOrganizationName("feelx88");
  app.setOrganizationDomain("feelx88.de");

  MainWindow w;
  w.show();

  return app.exec();
}
