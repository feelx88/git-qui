#ifndef GITINTERFACE_H
#define GITINTERFACE_H

#include <QObject>
#include <QVariant>

#include "actiontag.hpp"
#include "gitbranch.hpp"
#include "gitcommit.hpp"
#include "gitdiffline.hpp"
#include "gitfile.hpp"

class QDir;
class QFile;

enum class ErrorType;

class GitInterfacePrivate;

class GitInterface : public QObject {
  Q_OBJECT
public:
  GitInterface(const QString &name, const QString &path,
               QObject *parent = nullptr);
  virtual ~GitInterface();

  const QString name() const;
  void setName(const QString &name);
  const QString path() const;
  void setPath(const QString &path);
  const GitBranch activeBranch() const;
  bool actionRunning() const;

  const QList<GitBranch> branches(const QList<QString> &args) const;

  const QList<GitFile> files() const;

  static QString errorLogFileName();

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
  void push(const QString &remote = "origin",
            const QVariant &branch = QVariant(), bool setUpstream = false);
  void pull(bool rebase);
  void setFullFileDiff(bool fullFileDiff);
  void revertLastCommit();
  void resetLines(const QList<GitDiffLine> &lines);
  void checkoutPath(const QString &path);
  void changeBranch(const QString &branchName,
                    const QString &upstreamBranchName = "");
  void createBranch(const QString &name);
  void deleteBranch(const QString &name);
  void setUpstream(const QString &remote, const QString &branch);
  void stash();
  void stashPop();
signals:
  void fileChanged(const QFile &fileName);
  void nonStagingAreaChanged(const QList<GitFile> &);
  void stagingAreaChanged(const QList<GitFile> &);
  void logChanged(const QList<GitCommit> &logs);
  void commited();
  void fileSelected(bool unstaged, const QString &path);
  void fileDiffed(const QString &path, QList<GitDiffLine> lines, bool unstaged);
  void reloaded();
  void branchChanged(const QString &branch, bool hasChanges, bool hasUpstream,
                     int behindRemote, int aheadRemote);
  void pushStarted();
  void pushed();
  void pullStarted();
  void pulled();
  void lastCommitReverted(const QString &lastCommitMessage);
  void branchesChanged(const QList<GitBranch> branches);

  void actionStarted(const int &action);
  void actionFinished(const int &action);

  void error(const QString &message, ActionTag tag, ErrorType type,
             bool consoleOutput = false);

private:
  QScopedPointer<GitInterfacePrivate> _impl;
};

#endif // GITINTERFACE_H
