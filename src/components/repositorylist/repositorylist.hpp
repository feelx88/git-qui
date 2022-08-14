#ifndef REPOSITORYLIST_H
#define REPOSITORYLIST_H

#include "components/dockwidget.hpp"
#include "gitinterface.hpp"

namespace Ui {
class RepositoryList;
}

struct RepositoryListPrivate;

class RepositoryList : public DockWidget {
  Q_OBJECT
  DOCK_WIDGET
  friend struct RepositoryListPrivate;

public:
  explicit RepositoryList(MainWindow *mainWindow);
  virtual ~RepositoryList() override;

protected:
  virtual void onProjectSwitched(Project *newProject) override;
  virtual void
  onRepositoryAdded(QSharedPointer<GitInterface> newGitInterface) override;
  virtual void onRepositorySwitched(
      QSharedPointer<GitInterface>,
      QSharedPointer<QObject> activeRepositoryContext) override;
  virtual void onRepositoryRemoved(QSharedPointer<GitInterface>) override;

private:
  Ui::RepositoryList *ui;
  QScopedPointer<RepositoryListPrivate> _impl;
};

#endif // REPOSITORYLIST_H
