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

public:
  virtual void init() override;
  virtual void openRepository(const QString &path) override;
  virtual QList<GitFile *> status() override;
  virtual QList<GitDiffLine *> diffPath(const QString &path) override;

  virtual Q_INVOKABLE QString headName() override;
  virtual Q_INVOKABLE void stagePath(const QString &path) override;
  virtual Q_INVOKABLE void unstagePath(const QString &path) override;
  virtual Q_INVOKABLE void commit(const QString &message) override;

private:
  struct GitManagerPrivate;
  std::shared_ptr<GitManagerPrivate> _impl;
};

}

#endif // GIT_BIN_GITMANAGER_H
