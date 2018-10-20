#ifndef GITFILE_HPP
#define GITFILE_HPP

#include <QString>

struct GitFile
{
  bool staged, unstaged, modified, deleted;
  QString path;
};

#endif // GITFILE_HPP
