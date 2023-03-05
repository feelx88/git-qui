#ifndef GITINTERFACE_H
#define GITINTERFACE_H

#include <QFuture>
#include <QObject>
#include <QVariant>

#include "gitbranch.hpp"
#include "gitdiffline.hpp"
#include "gitfile.hpp"
#include "gittree.hpp"

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
    RELOAD,

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
    GIT_TAG,
    GIT_STASH,
    GIT_STASH_APPLY,
    GIT_CHECKOUT,
    GIT_REMOTE
  };
  Q_ENUM(ActionTag);

  enum class ErrorType { GENERIC = 0, STDERR, ALREADY_RUNNING };
  Q_ENUM(ErrorType);

  enum class ResetType { MIXED = 0, SOFT, HARD };
  Q_ENUM(ResetType);

  GitInterface(const QString &name, const QString &path,
               QObject *parent = nullptr);
  virtual ~GitInterface();

  const QString name() const;
  void setName(const QString &name);
  const QString path() const;
  void setPath(const QString &path);
  const GitBranch activeBranch() const;
  bool actionRunning() const;

  QFuture<QList<GitBranch>> branch(const QList<QString> &args);

  const QList<GitFile> files() const;
  const QList<GitFile> unstagedFiles() const;
  const QList<GitFile> stagedFiles() const;

  const QList<GitBranch> branches() const;
  const QList<QString> remotes() const;

  static QString errorLogFileName();

public slots:
  void setFullFileDiff(bool fullFileDiff);
  void fetchNonBlocking();
  QFuture<void> reload();
  void status();
  void historyStatus(const QString &commitId);
  void log();
  QFuture<void> fetch();
  QFuture<bool> commit(const QString &message);
  QFuture<void> stageFile(const QString &path);
  QFuture<void> stageFiles(const QStringList &paths);
  QFuture<void> unstageFile(const QString &path);
  void selectFile(bool unstaged, const QString &path);
  void diffFile(bool unstaged, const QString &path);
  QFuture<void> addLines(const QList<GitDiffLine> &lines, bool unstage);
  QFuture<void> push(const QString &remote = "origin",
                     const QVariant &branch = QVariant(),
                     bool setUpstream = false);
  QFuture<void> pushTags(const QString &remote = "origin");
  QFuture<void> pull(bool rebase);
  QFuture<void> revertLastCommit();
  QFuture<void> resetLines(const QList<GitDiffLine> &lines);
  QFuture<void> checkoutPath(const QString &path);
  QFuture<void> changeBranch(const QString &branchName,
                             const QString &upstreamBranchName = "");
  QFuture<void> createBranch(const QString &name,
                             const QString &baseCommit = "");
  QFuture<void> deleteBranch(const QString &name);
  QFuture<void> setUpstream(const QString &remote, const QString &branch);
  QFuture<void> createTag(const QString &name, const QString &commitId);
  QFuture<void> deleteTag(const QString &name);
  QFuture<void> stash();
  QFuture<void> stashPop();
  QFuture<void> resetToCommit(
      const QString &commitId,
      const GitInterface::ResetType &type = GitInterface::ResetType::MIXED);
signals:
  void fileChanged(const QFile &fileName);
  void nonStagingAreaChanged(const QList<GitFile> &);
  void stagingAreaChanged(const QList<GitFile> &);
  void historyFilesChanged(const QString &commitId, const QList<GitFile> &);
  void logChanged(QSharedPointer<GitTree> tree);
  void fileSelected(bool unstaged, const QString &path);
  void fileDiffed(const QString &path, QList<GitDiffLine> lines, bool unstaged);
  void branchChanged(const GitBranch &branch);
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
