#include "git/libgit2/gitmanager.h"

#include <utility>
#include <cstring>
#include <QList>

#include <git2.h>

struct GitManager::GitManagerPrivate
{
  GitManagerPrivate(GitManager* main)
    : _main(main)
  {
  }

  ~GitManagerPrivate()
  {
    git_repository_free(repo);
  }

  git_strarray strarrayFromQString(const QString &string);

  GitManager *_main;
  git_repository *repo;
};

GitManager::GitManager(QObject *parent)
  : QObject(parent),
    _impl(new GitManagerPrivate(this))
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

    if (status & GIT_STATUS_IGNORED) {
      return 0;
    }

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
    file->deleted = status &(
          GIT_STATUS_INDEX_DELETED |
          GIT_STATUS_WT_DELETED
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

void GitManager::stagePath(const QString &path)
{
  git_index *index = nullptr;
  git_repository_index(&index, _impl->repo);
  git_index_add_bypath(index, path.toStdString().c_str());
  git_index_write(index);
  git_index_free(index);
}

void GitManager::unstagePath(const QString &path)
{
  git_reference *ref = nullptr;
  git_repository_head(&ref, _impl->repo);

  git_object *head = nullptr;
  git_reference_peel(&head, ref, GIT_OBJ_COMMIT);

  git_strarray pathspec = _impl->strarrayFromQString(path);
  git_reset_default(_impl->repo, head, &pathspec);

  git_object_free(head);
  git_reference_free(ref);
}

QVariantList GitManager::diffPathVariant(const QString &path)
{
  auto list = diffPath(path);
  QVariantList propList;

  for(auto *diffLine : list)
  {
    propList.append(qVariantFromValue(diffLine));
  }

  return propList;
}

void GitManager::commit(const QString &message)
{
  // Get index
  git_index *index = nullptr;
  git_repository_index(&index, _impl->repo);

  // Get index tree
  git_oid tree_id;
  git_tree *tree = nullptr;
  git_commit *parent = nullptr;
  git_index_write_tree(&tree_id, index);
  git_tree_lookup(&tree, _impl->repo, &tree_id);

  // Get head commit
  git_oid parent_id;
  git_reference_name_to_id(&parent_id, _impl->repo, "HEAD");
  git_commit_lookup(&parent, _impl->repo, &parent_id);

  // Get author
  git_signature *author = nullptr;
  git_signature_default(&author, _impl->repo);

  // prettify message
  git_buf buf;
  buf.ptr = new char[message.size()];
  git_message_prettify(&buf, message.toStdString().c_str(), 1, '#');

  // Do the commit
  git_oid commit_id;
  git_commit_create_v(&commit_id, _impl->repo, "HEAD", author, author, nullptr,
                      buf.ptr, tree, 1, parent);

  // Clean up
  git_signature_free(author);
  git_commit_free(parent);
  git_tree_free(tree);
  git_index_free(index);
}

QList<GitDiffLine*> GitManager::diffPath(const QString &path)
{
  QList<GitDiffLine*> output;

  git_diff *diff = nullptr;
  git_diff_options options = GIT_DIFF_OPTIONS_INIT;
  options.pathspec = _impl->strarrayFromQString(path);
  options.interhunk_lines = 3;

  git_index *index = nullptr;
  git_repository_index(&index, _impl->repo);

  git_reference *head = nullptr;
  git_repository_head(&head, _impl->repo);

  git_object *headTree = nullptr;
  git_reference_peel(&headTree, head, GIT_OBJ_TREE);

  git_diff_tree_to_workdir_with_index(&diff, _impl->repo, reinterpret_cast<git_tree*>(headTree), &options);

  git_diff_print(diff, GIT_DIFF_FORMAT_PATCH, [](const git_diff_delta *,
                 const git_diff_hunk *,
                 const git_diff_line *line,
                 void *payload){
    GitDiffLine *diffLine = new GitDiffLine(nullptr);
    diffLine->content = QString::fromStdString(std::string(line->content, line->content_len - 1));
    diffLine->oldLine = line->old_lineno;
    diffLine->newLine = line->new_lineno;
    switch(line->origin)
    {
    case '+':
      diffLine->type = GitDiffLine::diffType::ADD;
      break;
    case '-':
      diffLine->type = GitDiffLine::diffType::REMOVE;
      break;
    case 'H':
      diffLine->type = GitDiffLine::diffType::HEADER;
      break;
    case 'F':
      diffLine->type = GitDiffLine::diffType::FILE_HEADER;
      break;
    case '=':
    case '>':
    case '<':
      diffLine->type = GitDiffLine::diffType::FILE_FOOTER;
      break;
    case 'B':
    case ' ':
    default:
      diffLine->type = GitDiffLine::diffType::CONTEXT;
      break;
    }
    static_cast<QList<GitDiffLine*>*>(payload)->append(diffLine);

    return 0;
  }, &output);

  git_object_free(headTree);
  git_diff_free(diff);
  git_index_free(index);
  return output;
}

git_strarray GitManager::GitManagerPrivate::strarrayFromQString(const QString &string)
{
  std::string str = string.toStdString();
  char *ch = new char[str.size()];
  std::memcpy(ch, str.c_str(), str.size() + 1);
  ch[str.size()] = 0;
  char **array = new char*[1];
  array[0] = ch;

  return {array, 1};
}
