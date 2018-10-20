#include "repositoryfiles.hpp"
#include "ui_repositoryfiles.h"

RepositoryFiles::RepositoryFiles(QWidget *parent) :
QDockWidget(parent),
ui(new Ui::RepositoryFiles)
{
  ui->setupUi(this);

  ui->radioButton->click();

  connect(ui->radioButton, &QRadioButton::clicked, [this]{
    ui->stackedWidget->setCurrentIndex(0);
  });

  connect(ui->radioButton_2, &QRadioButton::clicked, [this]{
    ui->stackedWidget->setCurrentIndex(1);
  });
}

RepositoryFiles::~RepositoryFiles()
{
  delete ui;
}
