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

#define SPLIT_DOCK_WIDGET(target, direction, first, second) target->splitDockWidget( \
  static_cast<QDockWidget*>(main->children()[first]), \
  static_cast<QDockWidget*>(main->children()[second]), \
  Qt::direction \
);

void InitialWindowConfiguration::create(MainWindow *mainWindow)
{
  QMainWindow *main = mainWindow->createTab(mainWindow->tr("main"));

  mainWindow->addDockWidget<RepositoryFiles>(0, QVariantMap({{"unstaged", true}}));
  mainWindow->addDockWidget<RepositoryFiles>(0, QVariantMap({{"unstaged", false}}));
  mainWindow->addDockWidget<DiffView>(0);
  mainWindow->addDockWidget<Commit>(0);
  mainWindow->addDockWidget<RepositoryList>(0);
  mainWindow->addDockWidget<BranchList>(0);

  SPLIT_DOCK_WIDGET(main, Vertical, 1, 2);
  SPLIT_DOCK_WIDGET(main, Vertical, 3, 4);
  SPLIT_DOCK_WIDGET(main, Vertical, 5, 6);

  mainWindow->createTab(mainWindow->tr("History"));

  mainWindow->addDockWidget<LogView>(1);

  mainWindow->setEditMode(true);

  for (auto toolbar : mainWindow->findChildren<QToolBar*>())
  {
    mainWindow->removeToolBar(toolbar);
    toolbar->deleteLater();
  }

  auto toolbar = mainWindow->addToolbar(Qt::ToolBarArea::TopToolBarArea);
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::PULL));
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::PUSH));
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::PULL_ALL));
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::PUSH_ALL));
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::NEW_BRANCH));
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::STASH));
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::UNSTASH));
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::CLEANUP));
}
