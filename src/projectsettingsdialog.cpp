#include "projectsettingsdialog.hpp"
#include "ui_projectsettingsdialog.h"

#include <QPushButton>
#include <QFileDialog>

#include "project.hpp"

struct ProjectSettingsDialogImpl
{
  Project *project;
};

ProjectSettingsDialog::ProjectSettingsDialog(ProjectSettingsDialog::DialogMode dialogMode, Project *project, QWidget *parent)
  : QDialog(parent),
    ui(new Ui::ProjectSettingsDialog),
    _impl(new ProjectSettingsDialogImpl)
{
  ui->setupUi(this);

  _impl->project = project;

  if (dialogMode == DialogMode::CREATE)
  {
    ui->buttonBox->button(QDialogButtonBox::Save)->hide();
    ui->buttonBox->button(QDialogButtonBox::Close)->hide();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  }
  else
  {
    ui->buttonBox->button(QDialogButtonBox::Ok)->hide();
    ui->buttonBox->button(QDialogButtonBox::Cancel)->hide();
  }

  ui->lineEdit->setText(_impl->project->name());
  ui->lineEdit_2->setText(_impl->project->fileName());
  for (Repository repository : _impl->project->repositoryList())
  {
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(repository.path.path()));
  }

  connect(ui->toolButton, &QToolButton::clicked, this, [this]{
    _impl->project->setFileName(QFileDialog::getOpenFileName(
      this,
      "Select project file"
    ));

    ui->lineEdit_2->setText(_impl->project->fileName());
    ui->toolButton_2->setEnabled(true);
  });

  ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Name" << "Path");

  connect(ui->toolButton_2, &QToolButton::clicked, this, [this]{
    _impl->project->addRepository();

    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(_impl->project->repositoryList().last().path.path()));

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  });
}

ProjectSettingsDialog::~ProjectSettingsDialog()
{
  delete ui;
}