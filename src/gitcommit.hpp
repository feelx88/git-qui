#ifndef GITCOMMIT_H
#define GITCOMMIT_H

#include <QObject>
#include <QVariant>

class GitCommit : public QObject
{
  Q_OBJECT

  Q_PROPERTY(QString id MEMBER id)
  Q_PROPERTY(QString message MEMBER message)
  Q_PROPERTY(QStringList branches MEMBER branches)
  Q_PROPERTY(QVariantList parents MEMBER parents)
  Q_PROPERTY(QVariantList children MEMBER children)

public:
  GitCommit(QObject *parent)
  : QObject(parent)
  {
  }

  GitCommit()
  : QObject()
  {
  }

  GitCommit(const GitCommit &other)
  : QObject(other.parent())
  {
    id = other.id;
    message = other.message;
    branches = other.branches;
    parents = other.parents;
    children = other.children;
  }

  virtual ~GitCommit() {}
  QString id, message;
  QStringList branches;
  QVariantList parents, children;
};
Q_DECLARE_METATYPE(GitCommit)

#endif // GITCOMMIT_H
