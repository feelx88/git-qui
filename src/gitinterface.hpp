#ifndef GITINTERFACE_H
#define GITINTERFACE_H

#include <QFuture>
#include <QObject>
#include <QVariant>

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
  enum class ActionTag {
    /** General **/
    NO_TAG = 0,

    /** git actions **/
    GIT_STATUS,
    GIT_LOG,
    GIT_FETCH,
    GIT_COMMIT,
    GIT_ADD,
    GIT_REMOVE,
    GIT_DIFF,
    GIT_PUSH,
    GIT_PULL,
    GIT_RESET,
    GIT_BRANCH,
    GIT_STASH
  };
  Q_ENUM(ActionTag);

  enum class ErrorType { GENERIC = 0, STDERR, ALREADY_RUNNING };
  Q_ENUM(ErrorType);

  GitInterface(const QString &name, const QString &path,
               QObject *parent = nullptr);
  virtual ~GitInterface();

  const QString name() const;
  void setName(const QString &name);
  const QString path() const;
  void setPath(const QString &path);
  const GitBranch activeBranch() const;
  bool actionRunning() const;

  QFuture<QList<GitBranch>> branches(const QList<QString> &args);

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

  void actionStarted(const GitInterface::ActionTag &action);
  void actionFinished(const GitInterface::ActionTag &action);

  void error(const QString &message, GitInterface::ActionTag tag,
             GitInterface::ErrorType type, bool consoleOutput = false);

private:
  QScopedPointer<GitInterfacePrivate> _impl;
};

#endif // GITINTERFACE_H
