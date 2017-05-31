#include "git/gitcommit.h"

GitCommit::GitCommit(QObject *parent)
  : QObject(parent)
{
}

QQmlListProperty<GitCommit> GitCommit::qmlParents()
{
  auto countFunc = [](QQmlListProperty<GitCommit>* list) {
    return reinterpret_cast<GitCommit*>(list->object)->parents.size();
  };
  auto atFunc = [](QQmlListProperty<GitCommit>* list, int index) {
    return reinterpret_cast<GitCommit*>(list->object)->parents.at(index);
  };
  return QQmlListProperty<GitCommit>(this, &parents, nullptr, countFunc, atFunc,
                                     nullptr);
}
