#include "repository.hpp"

Repository::Repository()
{
}

QDataStream &operator>>(QDataStream &in, Repository &repository)
{
  QString path;
  in >> repository.name >> path;
  repository.path.setPath(path);
  return in;
}

QDataStream &operator<<(QDataStream &out, const Repository &repository)
{
  out << repository.name << repository.path.path();
  return out;
}
