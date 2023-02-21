#include "projectsettingsdialog.hpp"
#include "ui_projectsettingsdialog.h"

#include <QFileDialog>
#include <QPushButton>

#include "gitinterface.hpp"
#include "project.hpp"

struct ProjectSettingsDialogPrivate {
  Project *project;

  void fillRepositoryList(ProjectSettingsDialog *_this) {
    _this->ui->repositoryTable->clearContents();
    _this->ui->repositoryTable->setRowCount(0);

    auto repositories = project->repositoryList();
    for (const auto &repository : repositories) {
      int row = _this->ui->repositoryTable->rowCount();
      _this->ui->repositoryTable->insertRow(row);
      _this->ui->repositoryTable->setItem(
          row, 0, new QTableWidgetItem(repository->name()));
      _this->ui->repositoryTable->setItem(
          row, 1, new QTableWidgetItem(repository->path()));
    }
  }
};

ProjectSettingsDialog::ProjectSettingsDialog(
    ProjectSettingsDialog::DialogMode dialogMode, Project *project,
    QWidget *parent)
    : QDialog(parent), ui(new Ui::ProjectSettingsDialog),
      _impl(new ProjectSettingsDialogPrivate) {
  ui->setupUi(this);

  _impl->project = project;

  if (dialogMode == DialogMode::CREATE) {
    ui->buttonBox->button(QDialogButtonBox::Close)->hide();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
  } else {
    ui->buttonBox->button(QDialogButtonBox::Ok)->hide();
    ui->buttonBox->button(QDialogButtonBox::Cancel)->hide();
    ui->addRepositoryButton->setEnabled(true);
  }

  ui->projectNameEdit->setText(_impl->project->name());
  ui->projectPathEdit->setText(_impl->project->fileName());
  ui->autoFetchEnabled->setChecked(_impl->project->autoFetchEnabled());
  _impl->fillRepositoryList(this);

  connect(ui->projectPathEdit, &QLineEdit::textChanged, this,
          [this](const QString &path) {
            ui->addRepositoryButton->setEnabled(!path.isEmpty());
          });

  connect(ui->projectPathChooseButton, &QToolButton::clicked, this, [this] {
    _impl->project->setFileName(
        QFileDialog::getSaveFileName(this, "Select project file"));

    ui->projectPathEdit->setText(_impl->project->fileName());
  });

  ui->repositoryTable->setHorizontalHeaderLabels(QStringList() << "Name"
                                                               << "Path");

  connect(ui->repositoryTable, &QTableWidget::itemSelectionChanged, this,
          [this] {
            ui->removeRepositoryButton->setEnabled(
                ui->repositoryTable->currentRow() >= 0);
          });

  connect(ui->repositoryTable->itemDelegate(),
          &QAbstractItemDelegate::commitData, this, [this]() {
            auto item = ui->repositoryTable->currentItem();
            QSharedPointer<GitInterface> repository =
                _impl->project->repositoryList().at(item->row());

            switch (item->column()) {
            case 0:
              repository->setName(item->text());
              break;
            case 1:
              repository->setPath(item->text());
              break;
            }

            _impl->project->save();
          });

  connect(ui->addRepositoryButton, &QToolButton::clicked, this, [this] {
    _impl->project->addRepository();
    _impl->fillRepositoryList(this);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
  });

  connect(ui->removeRepositoryButton, &QToolButton::clicked, this, [this] {
    _impl->project->removeRepository(ui->repositoryTable->currentRow());
    _impl->fillRepositoryList(this);
  });

  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this] {
    _impl->project->setFileName(ui->projectPathEdit->text());
    _impl->project->setName(ui->projectNameEdit->text());
    _impl->project->setAutoFetchEnabled(ui->autoFetchEnabled->isChecked());
    _impl->project->save();
  });
}

ProjectSettingsDialog::~ProjectSettingsDialog() { delete ui; }
