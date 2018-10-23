#ifndef GITINTERFACE_H
#define GITINTERFACE_H

#include <memory>
#include <QObject>
#include <QVariant>

#include "gitfile.hpp"
#include "gitdiffline.h"

class QDir;
class QFile;
class GitCommit;

class GitInterfacePrivate;

class GitInterface : public QObject
{
  Q_OBJECT
public:
  GitInterface(QObject *parent, const QDir &repositoryPath);
  virtual ~GitInterface();
public slots:
  void reload();
  void status();
  void log();
  void commit(const QString &message);
  void stageFile(const QString &path);
  void unstageFile(const QString &path);
  void selectFile(bool unstaged, const QString &path);
  void diffFile(bool unstaged, const QString &path);
signals:
  void fileChanged(const QFile& fileName);
  void nonStagingAreaChanged(QList<GitFile>);
  void stagingAreaChanged(QList<GitFile>);
  void logChanged(QVariantList logs);
  void commited();
  void fileSelected(bool unstaged, const QString &path);
  void fileDiffed(const QString &path, QList<GitDiffLine> lines);

  void error(const QString &message);

private:
  QScopedPointer<GitInterfacePrivate> _impl;
};

#endif // GITINTERFACE_H
