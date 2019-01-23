#include "commit.hpp"
#include "ui_commit.h"

#include <QShortcut>
#include <QMessageBox>

#include "mainwindow.hpp"
#include "gitinterface.hpp"

struct CommitPrivate
{
  QSharedPointer<GitInterface> gitInterface;
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

    _this->connect(static_cast<MainWindow*>(_this->parent()), &MainWindow::repositorySwitched, _this, [=](QSharedPointer<GitInterface> newGitInterface){
      _this->disconnect(_this->ui->pushButton_2, &QPushButton::clicked, _this, nullptr);
      _this->disconnect(gitInterface.get(), &GitInterface::lastCommitReverted, _this, nullptr);
      _this->disconnect(gitInterface.get(), &GitInterface::commited, _this, nullptr);
      _this->disconnect(gitInterface.get(), &GitInterface::stagingAreaChanged, _this, nullptr);
      _this->disconnect(gitInterface.get(), &GitInterface::nonStagingAreaChanged, _this, nullptr);

      gitInterface = newGitInterface;

      _this->connect(_this->ui->pushButton_2, &QPushButton::clicked, _this, [=]{
        gitInterface->revertLastCommit();
      });

      _this->connect(gitInterface.get(), &GitInterface::lastCommitReverted, _this, [=](const QString &message){
        _this->ui->plainTextEdit->setPlainText(message);
        _this->ui->pushButton_2->setDisabled(true);
      });

      _this->connect(gitInterface.get(), &GitInterface::commited, _this, [=]{
        _this->ui->plainTextEdit->clear();
      });

      _this->connect(gitInterface.get(), &GitInterface::stagingAreaChanged, _this, [=](const QList<GitFile> &list){
        stagedFiles = list;
      });

      _this->connect(gitInterface.get(), &GitInterface::nonStagingAreaChanged, _this, [=](const QList<GitFile> &list){
        unstagedFiles = list;
      });
    });

    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+Return"), _this->ui->plainTextEdit);
    _this->connect(shortcut, &QShortcut::activated, _this->ui->pushButton, &QPushButton::click);
  }
};

DOCK_WIDGET_IMPL(
  Commit,
  tr("Commit editor")
)

Commit::Commit(QWidget *parent, const QSharedPointer<GitInterface> &gitInterface) :
DockWidget(parent),
ui(new Ui::Commit),
_impl(new CommitPrivate)
{
  ui->setupUi(this);
  _impl->gitInterface = gitInterface;

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
