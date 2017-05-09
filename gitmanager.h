#ifndef GITMANAGER_H
#define GITMANAGER_H

#include <memory>
#include <QObject>
#include <QVariant>
#include <QVariantList>

#include <gitfile.h>

class GitManager : public QObject
{
  Q_OBJECT
public:
  GitManager(QObject *parent);
  void init();
  void openRepository(const QString &path);

  QList<GitFile*> status();

  Q_INVOKABLE QVariantList statusVariant();
  Q_INVOKABLE QString headName();
  Q_INVOKABLE void addPath(const QString &path);

signals:
  void gitError(const QString &message);

private:
  struct GitManagerPrivate;
  std::shared_ptr<GitManagerPrivate> _impl;
};

#endif // GITMANAGER_H
