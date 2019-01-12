#ifndef GITBRANCH_HPP
#define GITBRANCH_HPP

#include <QString>
#include <QMetaType>

struct GitBranch
{
public:
  bool active;
  QString name;
  QString upstreamName;
};

Q_DECLARE_METATYPE(GitBranch)

#endif // GITBRANCH_HPP
