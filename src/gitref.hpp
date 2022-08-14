#ifndef GITREF_HPP
#define GITREF_HPP

#include <QObject>

struct GitRef {
  GitRef() = default;
  GitRef(const GitRef &other) = default;
  QString name, remotePart;
  bool isHead = false, isTag = false, isRemote = false;
};
Q_DECLARE_METATYPE(GitRef);

#endif // GITREF_HPP
