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
