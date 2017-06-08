#ifndef LIBGIT2_GITMANAGER_H
#define LIBGIT2_GITMANAGER_H

#if defined (USE_LIBIGT2)

#include <memory>
#include <QObject>
#include <QVariant>
#include <QVariantList>

#include <git/agitmanager.h>

namespace libgit2
{
Q_NAMESPACE

class GitManager : public AGitManager
{
  Q_OBJECT
public:
  GitManager(QObject *parent);
  void init();
  void openRepository(const QString &path);
  QString repositoryRoot(const QString &);
  QStringList repositoryFiles();

  QList<GitFile*> status();
  QList<GitDiffLine*> diffPath(const QString &path, bool diffStaged = false);
  void stageLines(const QList<GitDiffLine *> &, bool);
  QList<GitCommit *> log();

  Q_INVOKABLE QString headName();
  Q_INVOKABLE void stagePath(const QString &path);
  Q_INVOKABLE void unstagePath(const QString &path);
  Q_INVOKABLE void commit(const QString &message);
  Q_INVOKABLE void checkout(const QString &path);
  Q_INVOKABLE void push(const QString &, const QString &, const QString &);

private:
  struct GitManagerPrivate;
  std::shared_ptr<GitManagerPrivate> _impl;
};

}

#endif // defined(USE_LIGIT2)

#endif // LIBGIT2_GITMANAGER_H
