#ifndef REPOSITORYFILES_H
#define REPOSITORYFILES_H

#include <QDockWidget>

#include "components/dockwidget.hpp"
#include "gitinterface.hpp"

namespace Ui {
class RepositoryFiles;
}

struct RepositoryFilesPrivate;

class RepositoryFiles : public DockWidget
{
  Q_OBJECT
  DOCK_WIDGET
  friend struct RepositoryFilesPrivate;

public:
  explicit RepositoryFiles(MainWindow *mainWindow);
  virtual ~RepositoryFiles() override;

  virtual QVariant configuration() override;
  virtual void configure(const QVariant &configuration) override;

protected:
  virtual void onProjectSwitched(Project *newProject) override;
  virtual void onRepositorySwitched(GitInterface *newGitInterface, QObject* activeRepositoryContext) override;

private:
  Ui::RepositoryFiles *ui;
  QScopedPointer<RepositoryFilesPrivate> _impl;
};

#endif // REPOSITORYFILES_H
