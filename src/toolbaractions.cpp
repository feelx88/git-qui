#include "toolbaractions.hpp"

#include <QAction>
#include <QtConcurrent/QtConcurrent>
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>

#include "qobjecthelpers.hpp"
#include "core.hpp"
#include "project.hpp"
#include "gitinterface.hpp"

QMap<QString, QAction*> ToolBarActions::_actionMap;

void ToolBarActions::initialize(Core *core)
{
  addAction("stash", "archive-insert", "Stash changes");
  addAction("unstash", "archive-remove", "Unstash changes");
  addAction("push", "go-up", "Push current repository");
  addAction("pull", "go-down", "Pull current repository (with rebase)");
  addAction("push-all", "go-top", "Push all repositories");
  addAction("pull-all", "go-bottom", "Pull all repositories (with rebase)");
  addAction("new-branch", "distribute-graph-directed", "Create new branch");

  for (auto &[id, action]: _actionMap.toStdMap())
  {
    action->setData(id);
  }

  QObject::connect(core->project(), &Project::repositorySwitched, core, [=](GitInterface *repository){
    RECONNECT(_actionMap["stash"], &QAction::triggered, _actionMap["stash"], [=]{
      repository->stash();
    });

    RECONNECT(_actionMap["unstash"], &QAction::triggered, _actionMap["unstash"], [=]{
      repository->stashPop();
    });

    RECONNECT(_actionMap["push"], &QAction::triggered, _actionMap["push"], [=]{
      QString branch = repository->activeBranch().name;
      bool addUpstream = false;
      if (repository->activeBranch().upstreamName.isEmpty())
      {
        addUpstream = QMessageBox::question(
          QApplication::activeWindow(),
          QObject::tr("No upstream branch configured"),
          QObject::tr("Would you like to set the default upstream branch to origin/%1?").arg(branch),
          QMessageBox::Yes,
          QMessageBox::No
        ) == QMessageBox::Yes;
      }

      QtConcurrent::run([=]{
        emit repository->pushStarted();
        repository->push("origin", branch, addUpstream);
      });
    });

    RECONNECT(_actionMap["pull"], &QAction::triggered, _actionMap["pull"], [=]{
      QtConcurrent::run([=]{
        emit repository->pullStarted();
        repository->pull(true);
      });
    });

    RECONNECT(_actionMap["new-branch"], &QAction::triggered, _actionMap["new-branch"], [=]{
      repository->createBranch(
        QInputDialog::getText(
          QApplication::activeWindow(),
          QObject::tr("Create new branch"),
          QObject::tr("New branch name")
        )
      );
    });
  });

  QObject::connect(_actionMap["push-all"], &QAction::triggered, _actionMap["push-all"], [=]{
    for (auto repo : core->project()->repositoryList())
    {
      emit repo->pushStarted();
      QtConcurrent::run([=]{
        repo->push();
      });
    }
  });

  QObject::connect(_actionMap["pull-all"], &QAction::triggered, _actionMap["pull-all"], [=]{
    for (auto repo : core->project()->repositoryList())
    {
      emit repo->pullStarted();
      QtConcurrent::run([=]{
        repo->pull(true);
      });
    }
  });
}

const QMap<QString, QAction*> ToolBarActions::all()
{
  return _actionMap;
}

QAction *ToolBarActions::byId(const QString &id)
{
  return _actionMap.value(id);
}

void ToolBarActions::addAction(QString id, QString icon, QString text)
{
  _actionMap.insert(
    id,
    new QAction(QIcon::fromTheme(
      icon,
      QIcon(QString(":/deploy/icons/%1.svg").arg(icon)
    )),
    QObject::tr(text.toStdString().c_str()))
  );
}
