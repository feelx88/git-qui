#ifndef GITBRANCH_HPP
#define GITBRANCH_HPP

#include <QMetaType>
#include <QString>

struct GitBranch {
public:
  bool active, remote = false, hasChanges = false, hasUpstream = false;
  QString name, upstreamName = "";
  int commitsAhead = 0, commitsBehind = 0;
};

Q_DECLARE_METATYPE(GitBranch)

#endif // GITBRANCH_HPP
