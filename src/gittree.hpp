#ifndef GITTREE_H
#define GITTREE_H

#include "gitcommit.hpp"

#include <QObject>
#include <QSharedPointer>

struct GitTreePrivate;

class GitTree {
public:
  friend struct GitTreePrivate;

  GitTree(const QList<GitCommit> &commits = QList<GitCommit>());

  const QList<QSharedPointer<GitCommit>> &commitList() const;

private:
  GitTreePrivate *_impl;
};
Q_DECLARE_METATYPE(GitTree)
Q_DECLARE_METATYPE(QSharedPointer<GitTree>)

#endif // GITTREE_H
