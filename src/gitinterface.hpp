#ifndef GITINTERFACE_H
#define GITINTERFACE_H

#include <QObject>
#include <QVariant>

#include "gitfile.hpp"
#include "gitdiffline.hpp"
#include "gitcommit.hpp"
#include "gitbranch.hpp"
#include "errortag.hpp"

class QDir;
class QFile;

class GitInterfacePrivate;

class GitInterface : public QObject
{
  Q_OBJECT
public:
  GitInterface(const QString &name, const QString &path, QObject *parent = nullptr);
  virtual ~GitInterface();

  const QString name() const;
  void setName(const QString &name);
  const QString path() const;
  void setPath(const QString &path);
  const GitBranch activeBranch() const;

  const QList<GitBranch> branches(const QList<QString> &args) const;

  const QList<GitFile> files() const;

public slots:
  void reload();
  void status();
  void log();
  void fetch();
  bool commit(const QString &message);
  void stageFile(const QString &path);
  void unstageFile(const QString &path);
  void selectFile(bool unstaged, const QString &path);
  void diffFile(bool unstaged, const QString &path);
  void addLines(const QList<GitDiffLine> &lines, bool unstage);
  void push(const QString& remote = "origin", const QVariant& branch = QVariant(), bool setUpstream = false);
  void pull(bool rebase);
  void setFullFileDiff(bool fullFileDiff);
  void revertLastCommit();
  void resetLines(const QList<GitDiffLine> &lines);
  void checkoutPath(const QString &path);
  void changeBranch(const QString &branchName, const QString &upstreamBranchName = "");
  void createBranch(const QString &name);
  void deleteBranch(const QString &name);
  void setUpstream(const QString &remote, const QString &branch);
  void stash();
  void stashPop();
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
  void pushStarted();
  void pushed();
  void pullStarted();
  void pulled();
  void lastCommitReverted(const QString &lastCommitMessage);
  void branchesChanged(const QList<GitBranch> branches);

  void error(const QString &message, ErrorTag tag, bool consoleOutput = false);

private:
  QScopedPointer<GitInterfacePrivate> _impl;
};

#endif // GITINTERFACE_H
