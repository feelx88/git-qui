#include "repositoryfileconfig.hpp"
#include "ui_repositoryfileconfig.h"

RepositoryFileConfig::RepositoryFileConfig(QWidget *parent) :
QDialog(parent),
ui(new Ui::RepositoryFileConfig)
{
  ui->setupUi(this);
}

RepositoryFileConfig::~RepositoryFileConfig()
{
  delete ui;
}

bool RepositoryFileConfig::unstaged()
{
  return ui->radioButton->isChecked();
}
