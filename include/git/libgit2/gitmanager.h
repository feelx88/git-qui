#ifndef GITMANAGER_H
#define GITMANAGER_H

#include <memory>
#include <QObject>
#include <QVariant>
#include <QVariantList>

#include <git/agitmanager.h>

class GitManager : public AGitManager
{
  Q_OBJECT
public:
  GitManager(QObject *parent);
  void init();
  void openRepository(const QString &path);

  QList<GitFile*> status();
  QList<GitDiffLine*> diffPath(const QString &path);

  Q_INVOKABLE QString headName();
  Q_INVOKABLE void stagePath(const QString &path);
  Q_INVOKABLE void unstagePath(const QString &path);
  Q_INVOKABLE void commit(const QString &message);

private:
  struct GitManagerPrivate;
  std::shared_ptr<GitManagerPrivate> _impl;
};

#endif // GITMANAGER_H
