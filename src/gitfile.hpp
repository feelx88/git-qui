#ifndef GITFILE_HPP
#define GITFILE_HPP

#include <QMetaType>
#include <QString>

struct GitFile {
  bool staged, unstaged, added, modified, deleted;
  QString path;

  bool operator<(const GitFile &other) { return other.path < path; }
};
Q_DECLARE_METATYPE(GitFile)

#endif // GITFILE_HPP
