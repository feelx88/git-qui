#ifndef GITMANAGER_H
#define GITMANAGER_H

#include <memory>

#include <QObject>

class GitManager : public QObject
{
  Q_OBJECT
public:
  GitManager(QObject *parent);
  void init();
  void openRepository(const QString &path);

signals:
  void gitError(const QString &message);

private:
  struct GitManagerPrivate;
  std::shared_ptr<GitManagerPrivate> _impl;
};

#endif // GITMANAGER_H
