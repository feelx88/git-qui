#include "toolbaractions.hpp"

#include <QAction>
#include <QtSvg>

#include "qobjecthelpers.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"

QMap<QString, QAction*> ToolBarActions::_actionMap;

void ToolBarActions::initialize(MainWindow *mainWindow)
{
  addAction("stash", "archive-insert", "Stash changes");
  addAction("unstash", "archive-remove", "Unstash changes");
  addAction("push", "collapse-all", "Push current repository");
  addAction("pull", "expand-all", "Pull current repository (with rebase)");
  addAction("push-all", "go-top", "Push all repositories");
  addAction("pull-all", "go-bottom", "Pull all repositories (with rebase)");

  for (auto &[id, action]: _actionMap.toStdMap())
  {
    action->setData(id);
  }

  QObject::connect(mainWindow, &MainWindow::repositorySwitched, mainWindow, [=](GitInterface *repository){
    RECONNECT(_actionMap["stash"], &QAction::triggered, _actionMap["stash"], [=]{
      repository->stash();
    });

    RECONNECT(_actionMap["unstash"], &QAction::triggered, _actionMap["unstash"], [=]{
      repository->stashPop();
    });

    RECONNECT(_actionMap["push"], &QAction::triggered, _actionMap["push"], [=]{
      repository->push();
    });

    RECONNECT(_actionMap["pull"], &QAction::triggered, _actionMap["pull"], [=]{
      repository->pull(true);
    });
  });

  QObject::connect(_actionMap["push-all"], &QAction::triggered, _actionMap["push-all"], [=]{
    for (auto repo : mainWindow->repositories())
    {
      repo->push();
    }
  });

  QObject::connect(_actionMap["pull-all"], &QAction::triggered, _actionMap["pull-all"], [=]{
    for (auto repo : mainWindow->repositories())
    {
      repo->pull(true);
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
