#include "commit.hpp"
#include "ui_commit.h"

#include <QShortcut>
#include <QMessageBox>

#include "project.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"

struct CommitPrivate
{
  GitInterface *gitInterface = nullptr;
  QList<GitFile> unstagedFiles, stagedFiles;

  void connectSignals(Commit *_this)
  {
    _this->connect(_this->ui->pushButton, &QPushButton::clicked, _this, [=]{
      if (stagedFiles.empty())
      {
        QMessageBox dlg;
        dlg.setText(_this->tr("There are no staged files. Would you like to commit everything?"));
        dlg.addButton(QMessageBox::Yes);
        dlg.addButton(QMessageBox::No);
        dlg.setDefaultButton(QMessageBox::Yes);

        if (dlg.exec() == QMessageBox::Yes)
        {
          for (auto unstagedFile : QList<GitFile>(unstagedFiles))
          {
            gitInterface->stageFile(unstagedFile.path);
          }
        }
      }
      gitInterface->commit(_this->ui->plainTextEdit->toPlainText());
      emit gitInterface->fileDiffed("", {}, false);
      _this->ui->pushButton_2->setEnabled(true);
    });

    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+Return"), _this->ui->plainTextEdit);
    _this->connect(shortcut, &QShortcut::activated, _this->ui->pushButton, &QPushButton::click);
  }
};

DOCK_WIDGET_IMPL(
  Commit,
  tr("Commit editor")
)

Commit::Commit(MainWindow *mainWindow) :
DockWidget(mainWindow),
ui(new Ui::Commit),
_impl(new CommitPrivate)
{
  ui->setupUi(this);

  _impl->connectSignals(this);
}

Commit::~Commit()
{
    delete ui;
}

QVariant Commit::configuration()
{
  return QVariant(ui->plainTextEdit->toPlainText());
}

void Commit::configure(const QVariant &configuration)
{
  ui->plainTextEdit->setPlainText(configuration.toString());
}

void Commit::onProjectSwitched(Project *newProject)
{
  _impl->gitInterface = nullptr;
  DockWidget::onProjectSwitched(newProject);
}

void Commit::onRepositorySwitched(GitInterface *newGitInterface, QObject *activeRepositoryContext)
{
  _impl->gitInterface = newGitInterface;

  connect(ui->pushButton_2, &QPushButton::clicked, activeRepositoryContext, [=]{
    newGitInterface->revertLastCommit();
  });

  connect(_impl->gitInterface, &GitInterface::lastCommitReverted, activeRepositoryContext, [=](const QString &message){
    ui->plainTextEdit->setPlainText(message);
    ui->pushButton_2->setDisabled(true);
  });

  connect(_impl->gitInterface, &GitInterface::commited, activeRepositoryContext, [=]{
    ui->plainTextEdit->clear();
  });

  connect(_impl->gitInterface, &GitInterface::stagingAreaChanged, activeRepositoryContext, [=](const QList<GitFile> &list){
    _impl->stagedFiles = list;
  });

  connect(_impl->gitInterface, &GitInterface::nonStagingAreaChanged, activeRepositoryContext, [=](const QList<GitFile> &list){
    _impl->unstagedFiles = list;
  });
}
