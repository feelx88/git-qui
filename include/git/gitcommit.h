#ifndef GITCOMMIT_H
#define GITCOMMIT_H

#include <QObject>

class GitCommit : public QObject
{
  Q_OBJECT
public:
  explicit GitCommit(QObject *parent = 0);
  Q_PROPERTY(QString id MEMBER id NOTIFY idChanged)
  Q_PROPERTY(QStringList branches MEMBER branches NOTIFY branchesChanged)
  Q_PROPERTY(QList<GitCommit*> parents MEMBER parents NOTIFY parentsChanged)

  QString id = "";
  QStringList branches;
  QList<GitCommit*> parents;

signals:
  void idChanged();
  void branchesChanged();
  void parentsChanged();
};

#endif // GITCOMMIT_H
