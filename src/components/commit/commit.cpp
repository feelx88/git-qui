#include "commit.hpp"
#include "ui_commit.h"

#include <QShortcut>
#include <QMessageBox>
#include <QMenu>

#include "project.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"

struct CommitPrivate
{
  GitInterface *gitInterface = nullptr;
  QList<GitFile> unstagedFiles, stagedFiles;
  QList<QString> messageHistory;
  QMenu *messageMenu;

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
        else
        {
          return;
        }
      }
      QString message = _this->ui->plainTextEdit->toPlainText();
      gitInterface->commit(message);
      emit gitInterface->fileDiffed("", {}, false);
      _this->ui->pushButton_2->setEnabled(true);

      addHistoryEntry(_this, message);
    });

    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+Return"), _this->ui->plainTextEdit);
    _this->connect(shortcut, &QShortcut::activated, _this->ui->pushButton, &QPushButton::click);

    _this->ui->pushButton_3->setEnabled(!messageHistory.empty());
    messageMenu = new QMenu(_this);
    _this->ui->pushButton_3->setMenu(messageMenu);
  }

  void addHistoryEntry(Commit *_this, QString message)
  {
    if (message.isEmpty())
    {
      return;
    }

    message.replace("\n", " ");

    messageHistory.removeAll(message);
    messageHistory.push_back(message);

    messageMenu->clear();
    for (const QString &message: messageHistory)
    {
      auto truncatedMessage = message;

      if (truncatedMessage.length() > 100)
      {
        truncatedMessage.truncate(100);
        truncatedMessage.append("...");
      }

      messageMenu->addAction(truncatedMessage, messageMenu, [=]{
        _this->ui->plainTextEdit->setPlainText(message);
      });
    }

    _this->ui->pushButton_3->setEnabled(true);
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
  return QVariantMap({
   {"messageHistory", QVariant(_impl->messageHistory.mid(_impl->messageHistory.length() - 20))},
   {"message", QVariant(ui->plainTextEdit->toPlainText())}
  });
}
#include <QDebug>
void Commit::configure(const QVariant &configuration)
{
  qDebug() << configuration;
  QVariantMap config = configuration.toMap();
  for (const QString &message : config.value("messageHistory").toStringList())
  {
    _impl->addHistoryEntry(this, message);
  }
  ui->plainTextEdit->setPlainText(config.value("currentMessage").toString());
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
