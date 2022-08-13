#ifndef GITCOMMIT_H
#define GITCOMMIT_H

#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <QWeakPointer>

struct GitCommit {
public:
  GitCommit() = default;
  GitCommit(const GitCommit &) = default;
  QString id, message, author;
  QDateTime date;
  QList<QString> refs, parentIds;
  QList<QWeakPointer<GitCommit>> parentCommits, childCommits;
};
Q_DECLARE_METATYPE(GitCommit)
Q_DECLARE_METATYPE(QSharedPointer<GitCommit>)
Q_DECLARE_METATYPE(QWeakPointer<GitCommit>)

#endif // GITCOMMIT_H
