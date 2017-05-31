#ifndef GITCOMMIT_H
#define GITCOMMIT_H

#include <QObject>
#include <QQmlListProperty>

class GitCommit : public QObject
{
  Q_OBJECT
public:
  explicit GitCommit(QObject *parent = 0);
  Q_PROPERTY(QString id MEMBER id NOTIFY idChanged)
  Q_PROPERTY(QString message MEMBER message NOTIFY messageChanged)
  Q_PROPERTY(QStringList branches MEMBER branches NOTIFY branchesChanged)
  Q_PROPERTY(QQmlListProperty<GitCommit> parents READ qmlParents NOTIFY parentsChanged)

  QString id = "", message ="";
  QStringList branches;
  QList<GitCommit*> parents;

  QQmlListProperty<GitCommit> qmlParents();

signals:
  void idChanged();
  void messageChanged();
  void branchesChanged();
  void parentsChanged();
};

#endif // GITCOMMIT_H
