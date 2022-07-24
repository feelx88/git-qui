#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QThreadPool>

#include "core.hpp"
#include "gitinterface.hpp"
#include "toolbaractions.hpp"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName("feelx88");
  app.setOrganizationDomain("feelx88.de");

  QSettings::setDefaultFormat(QSettings::IniFormat);

  qRegisterMetaType<QList<GitFile>>();
  qRegisterMetaType<QList<GitCommit>>();
  qRegisterMetaType<QList<GitBranch>>();
  qRegisterMetaType<QList<GitDiffLine>>();
  qRegisterMetaTypeStreamOperators<QList<QVariantMap>>();

  qRegisterMetaType<GitInterface::ActionTag>();
  qRegisterMetaType<GitInterface::ErrorType>();

  Core core;
  if (!core.init()) {
    return EXIT_FAILURE;
  }

  return app.exec();
}
