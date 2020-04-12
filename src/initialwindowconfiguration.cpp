#include "initialwindowconfiguration.hpp"

#include <QToolBar>

#include "mainwindow.hpp"
#include "toolbaractions.hpp"
#include "components/dockwidget.hpp"

#include "components/repositoryfiles/repositoryfiles.hpp"
#include "components/diffview/diffview.hpp"
#include "components/commit/commit.hpp"
#include "components/repositorylist/repositorylist.hpp"
#include "components/branchlist/branchlist.hpp"
#include "components/logview/logview.hpp"

#define ADD_DOCK_WIDGET_WITH_CONFIG(name, target, config) DockWidget::create( \
  #name, \
  mainWindow, \
  target, \
  QUuid::createUuid().toString(), \
  config \
)

#define ADD_DOCK_WIDGET(name, target) DockWidget::create( \
  #name, \
  mainWindow, \
  target, \
  QUuid::createUuid().toString() \
)

#define SPLIT_DOCK_WIDGET(target, direction, first, second) target->splitDockWidget( \
  static_cast<QDockWidget*>(main->children()[first]), \
  static_cast<QDockWidget*>(main->children()[second]), \
  Qt::direction \
);

void InitialWindowConfiguration::create(MainWindow *mainWindow)
{
  QMainWindow *main = mainWindow->createTab(mainWindow->tr("main"));
  ADD_DOCK_WIDGET_WITH_CONFIG(
    RepositoryFiles,
    main,
    (QMap<QString, QVariant>({{"unstaged", true}}))
  );
  ADD_DOCK_WIDGET_WITH_CONFIG(
    RepositoryFiles,
    main,
    (QMap<QString, QVariant>({{"unstaged", false}}))
  );
  ADD_DOCK_WIDGET(DiffView, main);
  ADD_DOCK_WIDGET(Commit, main);
  ADD_DOCK_WIDGET(RepositoryList, main);
  ADD_DOCK_WIDGET(BranchList, main);

  SPLIT_DOCK_WIDGET(main, Vertical, 1, 2);
  SPLIT_DOCK_WIDGET(main, Vertical, 3, 4);
  SPLIT_DOCK_WIDGET(main, Vertical, 5, 6);

  QMainWindow *history = mainWindow->createTab(mainWindow->tr("History"));
  ADD_DOCK_WIDGET(LogView, history);

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
