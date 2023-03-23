#ifndef GITFILE_HPP
#define GITFILE_HPP

#include <QMetaType>
#include <QString>

struct GitFile {
  bool staged = false, unstaged = false, added = false, modified = false,
       deleted = false, ignored = false;
  QString path = "";
  char flag = '\0';

  bool operator<(const GitFile &other) const {
    return ignored || (other.path < path);
  }
};
Q_DECLARE_METATYPE(GitFile)

#endif // GITFILE_HPP
