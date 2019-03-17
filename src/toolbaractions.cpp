#include "toolbaractions.hpp"

#include <QAction>
#include <QtSvg>

#include "qobjecthelpers.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"

QMap<QString, QAction*> ToolBarActions::_actionMap;

void ToolBarActions::initialize(MainWindow *mainWindow)
{
  auto qIcon = [](const QString &name){
    return QIcon::fromTheme(
      name,
      QIcon(QString(":/deploy/icons/%1.svg").arg(name))
    );
  };

  _actionMap = {
    {
      "stash",
      new QAction(qIcon("archive-insert"), QObject::tr("Stash changes"))
    },
    {
      "unstash",
      new QAction(qIcon("archive-remove"), QObject::tr("Unstash changes"))
    },
  };

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
