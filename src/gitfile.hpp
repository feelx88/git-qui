#ifndef GITFILE_HPP
#define GITFILE_HPP

#include <QString>
#include <QMetaType>

struct GitFile
{
  bool staged, unstaged, added, modified, deleted;
  QString path;
};
Q_DECLARE_METATYPE(GitFile)

#endif // GITFILE_HPP
