#include "repository.hpp"

#include "gitinterface.hpp"

struct RepositoryImpl
{
};

Repository::Repository(const QString &name, const QString &path, QObject *parent)
  : QObject(parent),
    name(name),
    path(path),
    gitInterface(new GitInterface(this, path)),
    _impl(new RepositoryImpl)
{
}
