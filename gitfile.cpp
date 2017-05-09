#include "gitfile.h"

struct GitFile::GitFilePrivate
{
  QString path;
  bool modified;
};

GitFile::GitFile(QObject *parent)
  : QObject(parent),
    _impl(new GitFilePrivate)
{
}

void GitFile::setPath(QString path)
{
  _impl->path = path;
  emit pathChanged();
}

QString GitFile::path()
{
  return _impl->path;
}

void GitFile::setModified(bool modified)
{
  _impl->modified = modified;
  emit modifiedChanged();
}

bool GitFile::modified()
{
  return _impl->modified;
}
