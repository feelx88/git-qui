#include "toolbaractions.hpp"

#include "cleanupdialog.hpp"
#include "core.hpp"
#include "gitinterface.hpp"
#include "project.hpp"
#include "qobjecthelpers.hpp"

#include <QAction>
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>

QMap<QString, QAction *> ToolBarActions::_actionMap;

void ToolBarActions::initialize(Core *core) {
  addAction(ActionID::STASH, "archive-insert", "Stash changes");
  addAction(ActionID::UNSTASH, "archive-remove", "Unstash changes");
  addAction(ActionID::PUSH, "go-up", "Push current repository");
  addAction(ActionID::PULL, "go-down", "Pull current repository (with rebase)");
  addAction(ActionID::PUSH_ALL, "go-top", "Push all repositories");
  addAction(ActionID::PULL_ALL, "go-bottom",
            "Pull all repositories (with rebase)");
  addAction(ActionID::NEW_BRANCH, "distribute-graph-directed",
            "Create new branch");
  addAction(ActionID::CLEANUP, "edit-clear-history", "Clean up repository");

  for (auto &[id, action] : _actionMap.toStdMap()) {
    action->setData(id);
  }

  auto projectChanged = [=](Project *newProject) {
    auto repositoryChanged = [=](GitInterface *repository,
                                 QObject *activeRepositoryContext) {
      QObject::connect(_actionMap[ActionID::STASH], &QAction::triggered,
                       activeRepositoryContext, [=] { repository->stash(); });

      QObject::connect(_actionMap[ActionID::UNSTASH], &QAction::triggered,
                       activeRepositoryContext,
                       [=] { repository->stashPop(); });

      QObject::connect(
          _actionMap[ActionID::PUSH], &QAction::triggered,
          activeRepositoryContext, [=] {
            QString branch = repository->activeBranch().name;
            bool addUpstream = false;
            if (repository->activeBranch().upstreamName.isEmpty()) {
              addUpstream =
                  QMessageBox::question(
                      QApplication::activeWindow(),
                      QObject::tr("No upstream branch configured"),
                      QObject::tr("Would you like to set the default upstream "
                                  "branch to origin/%1?")
                          .arg(branch),
                      QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
            }

            repository->push("origin", branch, addUpstream);
          });

      QObject::connect(
          _actionMap[ActionID::PULL], &QAction::triggered,
          activeRepositoryContext, [=] {
            bool stash = false;

            if (!repository->files().empty()) {
              stash =
                  QMessageBox::question(
                      QApplication::activeWindow(),
                      QObject::tr("There are open changes"),
                      QObject::tr("There are open changes in this repository. "
                                  "Would you like to stash your changes before "
                                  "pushing and unstash them afterwards?"),
                      QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
            }

            if (stash) {
              repository->stash();
            }
            repository->pull(true);
            if (stash) {
              repository->stashPop();
            }
          });

      QObject::connect(_actionMap[ActionID::NEW_BRANCH], &QAction::triggered,
                       activeRepositoryContext, [=] {
                         repository->createBranch(QInputDialog::getText(
                             QApplication::activeWindow(),
                             QObject::tr("Create new branch"),
                             QObject::tr("New branch name")));
                       });
    };

    QObject::connect(newProject, &Project::repositorySwitched, newProject,
                     repositoryChanged);

    QObject::connect(_actionMap[ActionID::PUSH_ALL], &QAction::triggered,
                     newProject, [=] {
                       for (auto repo : newProject->repositoryList()) {
                         emit repo->pushStarted();
                         repo->push();
                       }
                     });

    QObject::connect(_actionMap[ActionID::PULL_ALL], &QAction::triggered,
                     newProject, [=] {
                       for (auto repo : newProject->repositoryList()) {
                         repo->pull(true);
                       }
                     });

    repositoryChanged(core->project()->activeRepository(),
                      core->project()->activeRepositoryContext());
  };

  QObject::connect(core, &Core::projectChanged, core, projectChanged);
  projectChanged(core->project());

  QObject::connect(
      _actionMap[ActionID::CLEANUP], &QAction::triggered, core,
      [=] { (new CleanUpDialog(core, qApp->activeWindow()))->exec(); });
}

const QMap<QString, QAction *> ToolBarActions::all() { return _actionMap; }

QAction *ToolBarActions::byId(const QString &id) {
  return _actionMap.value(id);
}

void ToolBarActions::addAction(QString id, QString icon, QString text) {
  _actionMap.insert(
      id,
      new QAction(QIcon::fromTheme(
                      icon, QIcon(QString(":/deploy/icons/%1.svg").arg(icon))),
                  QObject::tr(text.toStdString().c_str())));
}
