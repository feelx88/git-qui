#include "toolbaractions.hpp"

#include "cleanupdialog.hpp"
#include "core.hpp"
#include "gitinterface.hpp"
#include "project.hpp"
#include "resetdialog.hpp"

#include <QAction>
#include <QApplication>
#include <QFutureWatcher>
#include <QInputDialog>
#include <QMessageBox>

QMap<QString, QAction *> ToolBarActions::_actionMap;

void ToolBarActions::initialize(Core *core) {
  addAction(ActionID::STASH, "archive-insert",
            ToolBarActions::tr("Stash changes"));
  addAction(ActionID::UNSTASH, "archive-remove",
            ToolBarActions::tr("Unstash changes"));
  addAction(ActionID::FETCH, "edit-download",
            ToolBarActions::tr("Fetch data from default remote"));
  addAction(ActionID::PUSH, "go-up",
            ToolBarActions::tr("Push current repository"));
  addAction(ActionID::PUSH_TAGS, "send-to",
            ToolBarActions::tr("Push tags of current repository"));
  addAction(ActionID::PULL, "go-down",
            ToolBarActions::tr("Pull current repository (with rebase)"));
  addAction(ActionID::PUSH_ALL, "go-top",
            ToolBarActions::tr("Push all repositories"));
  addAction(ActionID::PULL_ALL, "go-bottom",
            ToolBarActions::tr("Pull all repositories (with rebase)"));
  addAction(ActionID::NEW_BRANCH, "distribute-graph-directed",
            ToolBarActions::tr("Create new branch"));
  addAction(ActionID::CLEANUP, "edit-clear-history",
            ToolBarActions::tr("Clean up repository"));
  addAction(ActionID::RESET, "edit-undo",
            ToolBarActions::tr("Reset current branch to HEAD"));

  for (auto &[id, action] : _actionMap.toStdMap()) {
    action->setData(id);
  }

  auto projectChanged = [=](Project *newProject) {
    auto repositoryChanged = [=](QSharedPointer<GitInterface> repository,
                                 QSharedPointer<QObject>
                                     activeRepositoryContext) {
      disconnectRepositoryActions();

      QObject::connect(_actionMap[ActionID::STASH], &QAction::triggered,
                       activeRepositoryContext.get(),
                       [=] { repository->stash(); });

      QObject::connect(_actionMap[ActionID::UNSTASH], &QAction::triggered,
                       activeRepositoryContext.get(),
                       [=] { repository->stashPop(); });

      QObject::connect(_actionMap[ActionID::FETCH], &QAction::triggered,
                       activeRepositoryContext.get(),
                       [=] { repository->fetch(); });

      QObject::connect(
          _actionMap[ActionID::PUSH], &QAction::triggered,
          activeRepositoryContext.get(), [=] {
            QString branch = repository->activeBranch().name;
            bool addUpstream = false;
            if (repository->activeBranch().upstreamName.isEmpty()) {
              addUpstream =
                  QMessageBox::question(
                      QApplication::activeWindow(),
                      ToolBarActions::tr("No upstream branch configured"),
                      ToolBarActions::tr(
                          "Would you like to set the default upstream "
                          "branch to origin/%1?")
                          .arg(branch),
                      QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
            }

            repository->push("origin", branch, addUpstream);
          });

      QObject::connect(_actionMap[ActionID::PUSH_TAGS], &QAction::triggered,
                       activeRepositoryContext.get(),
                       [=] { repository->pushTags("origin"); });

      QObject::connect(
          _actionMap[ActionID::PULL], &QAction::triggered,
          activeRepositoryContext.get(), [=] {
            bool stash = false, openChanges = false;

            for (const auto &file : repository->files()) {
              if (!file.ignored) {
                openChanges = true;
                break;
              }
            }

            if (openChanges) {
              stash =
                  QMessageBox::question(
                      QApplication::activeWindow(),
                      ToolBarActions::tr("There are open changes"),
                      ToolBarActions::tr(
                          "There are open changes in this repository. "
                          "Would you like to stash your changes before "
                          "pulling and unstash them afterwards?"),
                      QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
            }

            if (stash) {
              auto stashWatcher =
                       new QFutureWatcher<void>(activeRepositoryContext.get()),
                   pullWatcher =
                       new QFutureWatcher<void>(activeRepositoryContext.get());

              QObject::connect(stashWatcher, &QFutureWatcher<void>::finished,
                               activeRepositoryContext.get(), [=] {
                                 pullWatcher->setFuture(repository->pull(true));
                                 stashWatcher->deleteLater();
                               });

              QObject::connect(pullWatcher, &QFutureWatcher<void>::finished,
                               activeRepositoryContext.get(), [=] {
                                 repository->stashPop();
                                 pullWatcher->deleteLater();
                               });

              stashWatcher->setFuture(repository->stash());
            } else {
              repository->pull(true);
            }
          });

      QObject::connect(
          _actionMap[ActionID::NEW_BRANCH], &QAction::triggered,
          activeRepositoryContext.get(), [=] {
            QString baseCommit;
            auto widget = focusedWidget(ActionID::NEW_BRANCH);
            if (widget) {
              baseCommit =
                  widget->property(ActionCallerProperty::NEW_BRANCH_BASE_COMMIT)
                      .toString();
            }

            QString branchName =
                QInputDialog::getText(QApplication::activeWindow(),
                                      ToolBarActions::tr("Create new branch"),
                                      ToolBarActions::tr("New branch name"));

            if (!branchName.isEmpty()) {
              repository->createBranch(branchName, baseCommit);
            }
          });
    };

    QObject::connect(newProject, &Project::repositorySwitched, newProject,
                     repositoryChanged);

    QObject::connect(_actionMap[ActionID::PUSH_ALL], &QAction::triggered,
                     newProject, [=] {
                       auto repositories = newProject->repositoryList();
                       for (auto repo : repositories) {
                         repo->push();
                       }
                     });

    QObject::connect(_actionMap[ActionID::PULL_ALL], &QAction::triggered,
                     newProject, [=] {
                       auto repositories = newProject->repositoryList();
                       for (auto repo : repositories) {
                         repo->pull(true);
                       }
                     });

    QObject::connect(
        _actionMap[ActionID::RESET], &QAction::triggered, core, [=] {
          QString repositoryName, ref;
          auto widget = focusedWidget(ActionID::RESET);

          if (widget) {
            repositoryName =
                widget->property(ActionCallerProperty::RESET_REPOSITORY)
                    .toString();
            ref = widget->property(ActionCallerProperty::RESET_REF).toString();
          }

          auto repository = repositoryName.isEmpty()
                                ? newProject->activeRepository()
                                : newProject->repositoryByName(repositoryName);

          ref = ref.isEmpty() ? "HEAD" : ref;

          if (repository) {
            auto commit = GitCommit();
            commit.id = ref;
            ResetDialog dialog(repository->activeBranch(), commit);

            if (dialog.exec() == QDialog::Accepted) {
              repository->resetToCommit(ref, dialog.resetType());
            }
          }
        });

    repositoryChanged(core->project()->activeRepository(),
                      core->project()->activeRepositoryContext());
  };

  QObject::connect(core, &Core::projectChanged, core, projectChanged);
  projectChanged(core->project());

  QObject::connect(core, &Core::beforeProjectChanged, core, [=] {
    disconnectRepositoryActions();
    disconnectProjectActions();
  });

  QObject::connect(
      _actionMap[ActionID::CLEANUP], &QAction::triggered, core,
      [=] { (new CleanUpDialog(core, qApp->activeWindow()))->exec(); });
}

void ToolBarActions::disconnectActions() {
  disconnectProjectActions();
  disconnectRepositoryActions();
  disconnectApplicationActions();
}

const QMap<QString, QAction *> ToolBarActions::all() { return _actionMap; }

QAction *ToolBarActions::byId(const QString &id) {
  return _actionMap.value(id);
}

void ToolBarActions::connectById(const QString &id, QAction *action) {
  auto parentAction = byId(id);
  action->setParent(parentAction);
  QObject::connect(action, &QAction::triggered, action,
                   [parentAction] { parentAction->trigger(); });
}

void ToolBarActions::disconnectProjectActions() {
  for (auto entry : {ActionID::PULL_ALL, ActionID::PUSH_ALL, ActionID::RESET}) {
    _actionMap[entry]->disconnect();
  }
}

void ToolBarActions::disconnectRepositoryActions() {
  for (auto entry : {ActionID::STASH, ActionID::UNSTASH, ActionID::FETCH,
                     ActionID::PUSH, ActionID::PULL, ActionID::NEW_BRANCH}) {
    _actionMap[entry]->disconnect();
  }
}

void ToolBarActions::disconnectApplicationActions() {
  for (auto entry : {ActionID::CLEANUP}) {
    _actionMap[entry]->disconnect();
  }
}

void ToolBarActions::addAction(QString id, QString icon, QString text) {
  _actionMap.insert(
      id,
      new QAction(QIcon::fromTheme(
                      icon, QIcon(QString(":/deploy/icons/%1.svg").arg(icon))),
                  text.toStdString().c_str()));
}

QWidget *ToolBarActions::focusedWidget(const QString &id) {
  auto action = _actionMap[id];

  {
    const auto &widgets = action->associatedWidgets();
    for (const auto &widget : widgets) {
      if (widget->hasFocus()) {
        return widget;
      }
    }
  }

  for (const auto &child : action->children()) {
    const auto &widgets = static_cast<QAction *>(child)->associatedWidgets();
    for (const auto &widget : widgets) {
      if (widget->hasFocus()) {
        return widget;
      }
    }
  }

  return nullptr;
}
