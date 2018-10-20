#ifndef REPOSITORYFILES_H
#define REPOSITORYFILES_H

#include <QDockWidget>

namespace Ui {
class RepositoryFiles;
}

class RepositoryFiles : public QDockWidget
{
  Q_OBJECT

public:
  explicit RepositoryFiles(QWidget *parent = nullptr);
  ~RepositoryFiles();

private:
  Ui::RepositoryFiles *ui;
};

#endif // REPOSITORYFILES_H
