#ifndef REPOSITORYLIST_H
#define REPOSITORYLIST_H

#include "components/dockwidget.hpp"
#include "gitinterface.hpp"

namespace Ui {
class RepositoryList;
}

struct RepositoryListPrivate;

class RepositoryList : public DockWidget
{
  Q_OBJECT
  DOCK_WIDGET
  friend struct RepositoryListPrivate;

public:
  explicit RepositoryList(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface);
  ~RepositoryList();

private:
  Ui::RepositoryList *ui;
  QScopedPointer<RepositoryListPrivate> _impl;
};

#endif // REPOSITORYLIST_H
