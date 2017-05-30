#ifndef GIT_BIN_GITMANAGER_H
#define GIT_BIN_GITMANAGER_H

#include <git/agitmanager.h>

#include <memory>

namespace gitBin
{
Q_NAMESPACE

class GitManager : public AGitManager
{
  Q_OBJECT
public:
  explicit GitManager(QObject *parent = 0);
  virtual void init() override;
  virtual void openRepository(const QString &path) override;
  virtual QString repositoryRoot(const QString &) override;
  virtual QStringList repositoryFiles() override;

  virtual QList<GitFile *> status() override;
  virtual QList<GitDiffLine *> diffPath(const QString &path, bool diffStaged = false) override;
  virtual void stageLines(const QList<GitDiffLine *> &lines, bool revert) override;
  virtual QList<GitCommit*> log() override;

  virtual Q_INVOKABLE QString headName() override;
  virtual Q_INVOKABLE void stagePath(const QString &path) override;
  virtual Q_INVOKABLE void unstagePath(const QString &path) override;
  virtual Q_INVOKABLE void commit(const QString &message) override;
  virtual Q_INVOKABLE void checkout(const QString &path) override;
  virtual Q_INVOKABLE void push(const QString &branch, const QString &remote,
                                const QString &remoteBranch) override;

private:
  struct GitManagerPrivate;
  std::shared_ptr<GitManagerPrivate> _impl;
};

}

#endif // GIT_BIN_GITMANAGER_H
