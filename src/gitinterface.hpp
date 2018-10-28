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
  GitInterface(QObject *parent, const QString &path);
  virtual ~GitInterface();
public slots:
  void switchRepository(const QString &path);
  void reload();
  void status();
  void log();
  void commit(const QString &message);
  void stageFile(const QString &path);
  void unstageFile(const QString &path);
  void selectFile(bool unstaged, const QString &path);
  void diffFile(bool unstaged, const QString &path);
  void addLines(const QList<GitDiffLine> &lines, bool unstage);
  void push();
  void pull(bool rebase);
  void setFullFileDiff(bool fullFileDiff);
signals:
  void fileChanged(const QFile& fileName);
  void nonStagingAreaChanged(const QList<GitFile>&);
  void stagingAreaChanged(const QList<GitFile>&);
  void logChanged(const QList<GitCommit> &logs);
  void commited();
  void fileSelected(bool unstaged, const QString &path);
  void fileDiffed(const QString &path, QList<GitDiffLine> lines, bool unstaged);
  void repositorySwitched(const QString &path);
  void reloaded();
  void branchChanged(const QString &branch, bool hasChanges);
  void pushed();
  void pulled();

  void error(const QString &message);

private:
  QScopedPointer<GitInterfacePrivate> _impl;
};

#endif // GITINTERFACE_H
