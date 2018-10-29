#include "commit.hpp"
#include "ui_commit.h"

#include <QMainWindow>
#include <QShortcut>
#include <QMessageBox>

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
      _this->ui->pushButton_2->setEnabled(true);
    });

    _this->connect(_this->ui->pushButton_2, &QPushButton::clicked, _this, [=]{
      gitInterface->revertLastCommit();
      _this->ui->pushButton_2->setDisabled(true);
    });

    _this->connect(gitInterface.get(), &GitInterface::lastCommitReverted, _this, [=](const QString &message){
      _this->ui->plainTextEdit->setPlainText(message);
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

    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+Return"), _this->ui->plainTextEdit);
    _this->connect(shortcut, &QShortcut::activated, _this->ui->pushButton, &QPushButton::click);
  }

  static void initialize(QMainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface)
  {
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, new Commit(mainWindow, gitInterface));
  }

  static void restore(QMainWindow *mainWindow, const QSharedPointer<GitInterface> &gitInterface, const QString &id, const QVariant &configuration)
  {
    Commit *commit = new Commit(mainWindow, gitInterface);
    commit->setObjectName(id);
    commit->ui->plainTextEdit->setPlainText(configuration.toString());
    mainWindow->addDockWidget(Qt::TopDockWidgetArea, commit);
  }
};

DOCK_WIDGET_IMPL(
  Commit,
  tr("Commit editor"),
  &CommitPrivate::initialize,
  &CommitPrivate::restore
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
