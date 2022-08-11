#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QSharedPointer>
#include <QThreadPool>

#include "core.hpp"
#include "gitinterface.hpp"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName("feelx88");
  app.setOrganizationDomain("feelx88.de");

  QSettings::setDefaultFormat(QSettings::IniFormat);

  qRegisterMetaType<QList<GitFile>>();
  qRegisterMetaType<QList<GitCommit>>();
  qRegisterMetaType<QSharedPointer<GitCommit>>();
  qRegisterMetaType<QWeakPointer<GitCommit>>();
  qRegisterMetaType<QList<GitBranch>>();
  qRegisterMetaType<QList<GitDiffLine>>();
  qRegisterMetaType<QList<GitTree>>();
  qRegisterMetaType<QSharedPointer<GitTree>>();
  qRegisterMetaTypeStreamOperators<QList<QVariantMap>>();

  qRegisterMetaType<GitInterface::ActionTag>();
  qRegisterMetaType<GitInterface::ErrorType>();

  Core core;
  if (!core.init()) {
    return EXIT_FAILURE;
  }

  return app.exec();
}
