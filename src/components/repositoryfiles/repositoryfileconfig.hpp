#ifndef REPOSITORYFILECONFIG_HPP
#define REPOSITORYFILECONFIG_HPP

#include <QDialog>

namespace Ui {
class RepositoryFileConfig;
}

class RepositoryFileConfig : public QDialog
{
  Q_OBJECT

public:
  explicit RepositoryFileConfig(QWidget *parent = nullptr);
  ~RepositoryFileConfig();

  bool unstaged();

private:
  Ui::RepositoryFileConfig *ui;
};

#endif // REPOSITORYFILECONFIG_HPP
