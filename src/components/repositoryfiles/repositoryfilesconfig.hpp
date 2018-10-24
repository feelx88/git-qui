#ifndef REPOSITORYFILECONFIG_HPP
#define REPOSITORYFILECONFIG_HPP

#include <QDialog>

namespace Ui {
class RepositoryFilesConfig;
}

class RepositoryFilesConfig : public QDialog
{
  Q_OBJECT

public:
  explicit RepositoryFilesConfig(QWidget *parent = nullptr);
  ~RepositoryFilesConfig();

  bool unstaged();

private:
  Ui::RepositoryFilesConfig *ui;
};

#endif // REPOSITORYFILECONFIG_HPP
