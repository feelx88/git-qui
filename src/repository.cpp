#include "repository.hpp"

#include "gitinterface.hpp"

struct RepositoryImpl
{
  GitInterface *gitInterface;
};

Repository::Repository(const QString &name, const QString &path, QObject *parent)
  : QObject(parent),
    name(name),
    path(path),
    _impl(new RepositoryImpl)
{
  _impl->gitInterface = new GitInterface(parent, path);
}

Repository::Repository(const Repository &other)
  : Repository(other.name, other.path.path(), other.parent())
{
}

Repository &Repository::operator=(const Repository &other)
{
  name = other.name;
  path = other.path;
  _impl->gitInterface = other._impl->gitInterface;
  return *this;
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
