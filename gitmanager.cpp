#include "gitmanager.h"

#include <utility>
#include <QList>

#include <git2.h>

struct GitManager::GitManagerPrivate
{
  ~GitManagerPrivate()
  {
    git_repository_free(repo);
  }

  git_repository *repo;
};

GitManager::GitManager(QObject *parent)
  : QObject(parent),
    _impl(new GitManagerPrivate)
{
}

void GitManager::init()
{
  git_libgit2_init();
}

void GitManager::openRepository(const QString &path)
{
  if (git_repository_open(&_impl->repo, path.toStdString().c_str()))
  {
    emit gitError("Could not open repository at " + path);
  }
}

QList<GitFile*> GitManager::status()
{
  QList<GitFile*> list;

  git_status_list *status = nullptr;
  git_status_options options = GIT_STATUS_OPTIONS_INIT;
  git_status_list_new(&status, _impl->repo, &options);

  std::pair<QList<GitFile*>*, GitManager*> data = std::make_pair(&list, this);
  git_status_foreach(_impl->repo, [](const char* path, unsigned int status, void* _data) -> int {

    auto *data = static_cast<std::pair<QList<GitFile*>*, GitManager*>*>(_data);
    QList<GitFile*> *list = data->first;
    GitManager *manager = data->second;

    GitFile *file = new GitFile(manager);
    file->path = path;
    file->modified = status & (
          GIT_STATUS_INDEX_MODIFIED |
          GIT_STATUS_WT_MODIFIED
          );
    file->staged = status & (
          GIT_STATUS_INDEX_MODIFIED |
          GIT_STATUS_INDEX_NEW |
          GIT_STATUS_INDEX_DELETED |
          GIT_STATUS_INDEX_RENAMED |
          GIT_STATUS_INDEX_TYPECHANGE
          );
    list->append(file);
    return 0;
  }, &data);

  git_status_list_free(status);

  return list;
}

QVariantList GitManager::statusVariant()
{
  auto list = status();
  QVariantList propList;

  for(auto *file : list)
  {
    propList.append(qVariantFromValue(file));
  }

  return propList;
}

QString GitManager::headName()
{
  const char* name;
  git_reference *ref = nullptr;

  git_repository_head(&ref, _impl->repo);
  git_branch_name(&name, ref);

  git_reference_free(ref);

  return QString(name);
}
