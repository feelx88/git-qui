#include <iostream>

#include <QApplication>
#include <QMessageBox>
#include <QSettings>

#include "core.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"
#include "toolbaractions.hpp"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  app.setOrganizationName("feelx88");
  app.setOrganizationDomain("feelx88.de");

  QSettings::setDefaultFormat(QSettings::IniFormat);

  qRegisterMetaType<QList<GitFile>>();
  qRegisterMetaType<QList<GitCommit>>();
  qRegisterMetaType<QList<GitBranch>>();
  qRegisterMetaTypeStreamOperators<QList<QVariantMap>>();

  Core core;

  if (!core.init())
  {
    return EXIT_FAILURE;
  }

  ToolBarActions::initialize(&core);

  return app.exec();
}
