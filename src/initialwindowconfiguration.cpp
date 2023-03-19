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

#include <AutoHideSideBar.h>

#include <components/historyfiles/historyfiles.h>

#define SPLIT_DOCK_WIDGET(target, direction, first, second)                    \
  target->splitDockWidget(                                                     \
      static_cast<ads::CDockWidget *>(main->children()[first]),                \
      static_cast<ads::CDockWidget *>(main->children()[second]),               \
      Qt::direction);

void InitialWindowConfiguration::create(MainWindow *mainWindow) {
  // Commit tab
  auto commitTab = mainWindow->createTab(MainWindow::tr("Commit"));
  auto commitTabDockManager = commitTab->findChild<ads::CDockManager *>();

  mainWindow->addDockWidget<DiffView>(0, {}, ads::TopDockWidgetArea);
  mainWindow->addDockWidget<Commit>(0, {}, ads::BottomDockWidgetArea);

  auto unstagedFiles = mainWindow->addDockWidget<RepositoryFiles>(
      0, QVariantMap({{"unstaged", true}}), ads::LeftDockWidgetArea);
  auto stagedFiles = mainWindow->addDockWidget<RepositoryFiles>(
      0, QVariantMap({{"unstaged", false}}));
  commitTabDockManager->addDockWidget(ads::BottomDockWidgetArea, stagedFiles,
                                      unstagedFiles->dockAreaWidget());

  auto repositoryList = mainWindow->addDockWidget<RepositoryList>(
      0, {}, ads::RightDockWidgetArea);
  auto branchList = mainWindow->addDockWidget<BranchList>(0, {});
  commitTabDockManager->addDockWidget(ads::BottomDockWidgetArea, branchList,
                                      repositoryList->dockAreaWidget());

  auto errorLog = mainWindow->addDockWidget<ErrorLog>(0);
  commitTabDockManager->sideTabBar(ads::SideBarBottom)
      ->insertDockWidget(0, errorLog);

  // History tab
  auto historyTab = mainWindow->createTab(MainWindow::tr("History"));
  auto historyTabDockManager = historyTab->findChild<ads::CDockManager *>();

  mainWindow->addDockWidget<LogView>(1, {}, ads::LeftDockWidgetArea);
  mainWindow->addDockWidget<RepositoryList>(1, {}, ads::RightDockWidgetArea);

  auto historyFiles =
      mainWindow->addDockWidget<HistoryFiles>(1, {}, ads::BottomDockWidgetArea);
  auto historyDiffView = mainWindow->addDockWidget<DiffView>(
      1, QVariantMap{{"historyMode", true}});
  historyTabDockManager->addDockWidget(ads::RightDockWidgetArea,
                                       historyDiffView,
                                       historyFiles->dockAreaWidget());

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
