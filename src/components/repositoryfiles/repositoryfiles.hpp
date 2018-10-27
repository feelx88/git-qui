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
  explicit RepositoryFiles(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface, bool unstaged);
  ~RepositoryFiles() override;

  virtual QVariant configuration() override;
private:
  Ui::RepositoryFiles *ui;
  QScopedPointer<RepositoryFilesPrivate> _impl;
};

#endif // REPOSITORYFILES_H