#include "repositoryfilesconfig.hpp"
#include "ui_repositoryfilesconfig.h"

RepositoryFilesConfig::RepositoryFilesConfig(QWidget *parent) :
QDialog(parent),
ui(new Ui::RepositoryFilesConfig)
{
  ui->setupUi(this);
}

RepositoryFilesConfig::~RepositoryFilesConfig()
{
  delete ui;
}

bool RepositoryFilesConfig::unstaged()
{
  return ui->radioButton->isChecked();
}
