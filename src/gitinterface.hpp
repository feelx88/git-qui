#ifndef GITINTERFACE_H
#define GITINTERFACE_H

#include <memory>
#include <QObject>
#include <QVariant>

#include "gitfile.hpp"
#include "gitdiffline.h"
#include "gitcommit.hpp"
#include "gitbranch.hpp"

class QDir;
class QFile;

class GitInterfacePrivate;

class GitInterface : public QObject
{
  Q_OBJECT
public:
  GitInterface(QObject *parent, const QString &path);
  virtual ~GitInterface();

  const QString path();
  const GitBranch activeBranch();

public slots:
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
  void revertLastCommit();
  void resetLines(const QList<GitDiffLine> &lines);
  void checkoutPath(const QString &path);
  void changeBranch(const QString &branchName);
  void createBranch(const QString &name);
  void deleteBranch(const QString &name);
  void setUpstream(const QString &remote, const QString &branch);
signals:
  void fileChanged(const QFile& fileName);
  void nonStagingAreaChanged(const QList<GitFile>&);
  void stagingAreaChanged(const QList<GitFile>&);
  void logChanged(const QList<GitCommit> &logs);
  void commited();
  void fileSelected(bool unstaged, const QString &path);
  void fileDiffed(const QString &path, QList<GitDiffLine> lines, bool unstaged);
  void reloaded();
  void branchChanged(const QString &branch, bool hasChanges, bool hasUpstream, int behindRemote, int aheadRemote);
  void pushed();
  void pulled();
  void lastCommitReverted(const QString &lastCommitMessage);
  void branchesChanged(const QList<GitBranch> branches);

  void error(const QString &message);

private:
  QScopedPointer<GitInterfacePrivate> _impl;
};

#endif // GITINTERFACE_H
