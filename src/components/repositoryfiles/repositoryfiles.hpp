#ifndef REPOSITORYFILES_H
#define REPOSITORYFILES_H

#include <QDockWidget>

#include "gitinterface.hpp"

namespace Ui {
class RepositoryFiles;
}

struct RepositoryFilesPrivate;

class RepositoryFiles : public QDockWidget
{
  Q_OBJECT
  friend struct RepositoryFilesPrivate;

public:
  explicit RepositoryFiles(QWidget *parent, QSharedPointer<GitInterface> gitInterface);
  ~RepositoryFiles();

private:
  Ui::RepositoryFiles *ui;
  QScopedPointer<RepositoryFilesPrivate> _impl;
};

#endif // REPOSITORYFILES_H
