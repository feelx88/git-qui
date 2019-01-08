#include <iostream>

#include <QApplication>
#include <QSettings>

#include "mainwindow.hpp"

#include "gitinterface.hpp"
#include "gitcommit.hpp"

int main(int argc, char *argv[])
{
  QSettings::setDefaultFormat(QSettings::IniFormat);

  qRegisterMetaType<GitCommit>();
  qRegisterMetaType<GitFile>();
  qRegisterMetaType<GitDiffLine>();

  QApplication app(argc, argv);
  app.setOrganizationName("feelx88");
  app.setOrganizationDomain("feelx88.de");

  MainWindow w;
  w.show();

  return app.exec();
}
