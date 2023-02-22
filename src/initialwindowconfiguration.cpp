#include "initialwindowconfiguration.hpp"

#include <QToolBar>

#include "components/dockwidget.hpp"
#include "mainwindow.hpp"
#include "toolbaractions.hpp"

#include "components/branchlist/branchlist.hpp"
#include "components/commit/commit.hpp"
#include "components/diffview/diffview.hpp"
#include "components/errorlog/errorlog.hpp"
#include "components/logview/logview.hpp"
#include "components/repositoryfiles/repositoryfiles.hpp"
#include "components/repositorylist/repositorylist.hpp"

#define SPLIT_DOCK_WIDGET(target, direction, first, second)                    \
  target->splitDockWidget(                                                     \
      static_cast<ads::CDockWidget *>(main->children()[first]),                \
      static_cast<ads::CDockWidget *>(main->children()[second]),               \
      Qt::direction);

void InitialWindowConfiguration::create(MainWindow *mainWindow) {
  // Commit tab
  mainWindow->createTab(mainWindow->tr("Commit"));

  mainWindow->addDockWidget<DiffView>(0, {}, ads::CenterDockWidgetArea);
  mainWindow->addDockWidget<Commit>(0, {}, ads::CenterDockWidgetArea);

  mainWindow->addDockWidget<RepositoryFiles>(
      0, QVariantMap({{"unstaged", true}}), ads::LeftDockWidgetArea);
  mainWindow->addDockWidget<RepositoryFiles>(
      0, QVariantMap({{"unstaged", false}}), ads::LeftDockWidgetArea);

  mainWindow->addDockWidget<RepositoryList>(0, {}, ads::RightDockWidgetArea);
  mainWindow->addDockWidget<BranchList>(0, {}, ads::RightDockWidgetArea);

  // History tab
  mainWindow->createTab(mainWindow->tr("History"));

  mainWindow->addDockWidget<LogView>(1, {}, ads::LeftDockWidgetArea);
  mainWindow->addDockWidget<RepositoryList>(1, {}, ads::RightDockWidgetArea);

  // Error log tab
  mainWindow->createTab(mainWindow->tr("Error log"));

  mainWindow->addDockWidget<ErrorLog>(2);

  // Enable edit mode
  mainWindow->setEditMode(true);

  // Add default toolbars
  auto toolBars = mainWindow->findChildren<QToolBar *>();
  for (auto toolbar : toolBars) {
    mainWindow->removeToolBar(toolbar);
    toolbar->deleteLater();
  }

  auto toolbar = mainWindow->addToolbar(Qt::ToolBarArea::TopToolBarArea);
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::FETCH));
  toolbar->addSeparator();
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::PULL));
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::PUSH));
  toolbar->addSeparator();
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::PULL_ALL));
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::PUSH_ALL));
  toolbar->addSeparator();
  toolbar->addAction(
      ToolBarActions::byId(ToolBarActions::ActionID::NEW_BRANCH));
  toolbar->addSeparator();
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::STASH));
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::UNSTASH));
  toolbar->addSeparator();
  toolbar->addAction(ToolBarActions::byId(ToolBarActions::ActionID::CLEANUP));
}
