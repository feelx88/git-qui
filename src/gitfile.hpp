#ifndef GITFILE_HPP
#define GITFILE_HPP

#include <QMetaType>
#include <QString>

struct GitFile {
  bool staged = false, unstaged = false, added = false, modified = false,
       deleted = false;
  QString path = "";

  bool operator<(const GitFile &other) const { return other.path < path; }
};
Q_DECLARE_METATYPE(GitFile)

#endif // GITFILE_HPP
