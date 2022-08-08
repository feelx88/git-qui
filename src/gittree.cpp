#include "gittree.hpp"

#include <QMap>
#include <gitcommit.hpp>

struct GitTreePrivate {
  QMap<QString, QWeakPointer<GitCommit>> commitMap;
  QList<QSharedPointer<GitCommit>> commitList;
};

GitTree::GitTree(const QList<GitCommit> &commits) : _impl(new GitTreePrivate) {
  for (const auto &commit : commits) {
    auto commitPtr = QSharedPointer<GitCommit>::create(GitCommit(commit));
    _impl->commitMap.insert(commit.id, commitPtr);
    _impl->commitList.append(commitPtr);
  }

  for (const auto &commit : qAsConst(_impl->commitList)) {
    for (const auto &parentId : qAsConst(commit->parentIds)) {
      auto parentCommit = _impl->commitMap.find(parentId);
      if (parentCommit != _impl->commitMap.end()) {
        auto parentCommitPtr = (*parentCommit).lock();
        if (parentCommitPtr) {
          parentCommitPtr->childCommits.append(commit);
          commit->parentCommits.append(parentCommitPtr);
        }
      }
    }
  }
}

const QList<QSharedPointer<GitCommit>> &GitTree::commitList() const {
  return _impl->commitList;
}
