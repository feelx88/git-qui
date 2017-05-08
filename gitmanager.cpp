#include "gitmanager.h"

#include <git2.h>

struct GitManager::GitManagerPrivate
{
  std::shared_ptr<git_repository> repo;
};

GitManager::GitManager(QObject *parent)
  : _impl(new GitManagerPrivate)
{
}

void GitManager::init()
{
  git_libgit2_init();
}

void GitManager::openRepository(const QString &path)
{
  git_repository *repo = _impl->repo.get();
  if (git_repository_open(&repo, path.toStdString().c_str()))
  {
    emit gitError("Could not open repository at " + path);
  }
}
