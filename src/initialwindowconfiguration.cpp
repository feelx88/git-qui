#include "initialwindowconfiguration.hpp"

#include <QToolBar>

#include "mainwindow.hpp"
#include "toolbaractions.hpp"
#include "components/dockwidget.hpp"

void InitialWindowConfiguration::create(MainWindow *mainWindow)
{
  QMainWindow *main = mainWindow->createTab(mainWindow->tr("main"));
  QMap<QString, QVariant> unstagedConfig = {{"unstaged", true}};
  DockWidget::create(
    "RepositoryFiles",
    mainWindow,
    main,
    QUuid::createUuid().toString(),
    unstagedConfig
  );
  QMap<QString, QVariant> stagedConfig = {{"unstaged", false}};
  DockWidget::create(
    "RepositoryFiles",
    mainWindow,
    main,
    QUuid::createUuid().toString(),
    stagedConfig
  );
  DockWidget::create(
    "DiffView",
    mainWindow,
    main
  );
  DockWidget::create(
    "Commit",
    mainWindow,
    main
  );
  DockWidget::create(
    "RepositoryList",
    mainWindow,
    main
  );
  DockWidget::create(
    "BranchList",
    mainWindow,
    main
  );
  main->splitDockWidget(
    static_cast<QDockWidget*>(main->children()[1]),
    static_cast<QDockWidget*>(main->children()[2]),
    Qt::Vertical
  );
  main->splitDockWidget(
    static_cast<QDockWidget*>(main->children()[3]),
    static_cast<QDockWidget*>(main->children()[4]),
    Qt::Vertical
  );
  main->splitDockWidget(
    static_cast<QDockWidget*>(main->children()[5]),
    static_cast<QDockWidget*>(main->children()[6]),
    Qt::Vertical
  );

  QMainWindow *history = mainWindow->createTab(mainWindow->tr("History"));
  DockWidget::create(
    "LogView",
    mainWindow,
    history
  );

  mainWindow->setEditMode(true);

  for (auto toolbar : mainWindow->findChildren<QToolBar*>())
  {
    mainWindow->removeToolBar(toolbar);
    toolbar->deleteLater();
  }

  auto toolbar = mainWindow->addToolbar(Qt::ToolBarArea::TopToolBarArea);
  toolbar->addAction(ToolBarActions::byId("pull"));
  toolbar->addAction(ToolBarActions::byId("push"));
  toolbar->addAction(ToolBarActions::byId("pull-all"));
  toolbar->addAction(ToolBarActions::byId("push-all"));
  toolbar->addAction(ToolBarActions::byId("new-branch"));
  toolbar->addAction(ToolBarActions::byId("stash"));
  toolbar->addAction(ToolBarActions::byId("unstash"));
}
