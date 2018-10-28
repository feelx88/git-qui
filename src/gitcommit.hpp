#ifndef GITCOMMIT_H
#define GITCOMMIT_H

#include <QList>
#include <QDate>

struct GitCommit
{
public:
  QString id, message;
  QDate date;
  QList<QString> branches, parents, children;
};
Q_DECLARE_METATYPE(GitCommit)

#endif // GITCOMMIT_H
