#ifndef GITCOMMIT_H
#define GITCOMMIT_H

#include <QDateTime>
#include <QList>

struct GitCommit {
public:
  QString id, message, author;
  QDateTime date;
  QList<QString> branches, parents, children;
};
Q_DECLARE_METATYPE(GitCommit)

#endif // GITCOMMIT_H
