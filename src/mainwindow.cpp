#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMap>
#include <QMessageBox>
#include <QMovie>
#include <QProcess>
#include <QSettings>
#include <QStatusBar>
#include <QSvgWidget>
#include <QTimer>
#include <QToolBar>

#include "components/dockwidget.hpp"
#include "core.hpp"
#include "gitinterface.hpp"
#include "initialwindowconfiguration.hpp"
#include "project.hpp"
#include "projectsettingsdialog.hpp"
#include "toolbaractions.hpp"
#include "toolbareditor.hpp"

struct ConfigurationKeys {
  static constexpr const char *GEOMETRY = "geometry";
  static constexpr const char *DOCK_CONFIGURATION = "dockConfiguration";
  static constexpr const char *STATE = "state";
  static constexpr const char *EDIT_MODE = "editMode";

  static constexpr const char *TABS = "tabs";
  static constexpr const char *TAB_NAME = "tabName";

  static constexpr const char *DOCK_WIDGETS = "dockWidgets";
  static constexpr const char *DOCKWIDGET_ID = "id";
  static constexpr const char *DOCKWIDGET_CLASS = "class";
  static constexpr const char *DOCKWIDGET_CONFIGURATION = "configuration";

  static constexpr const char *TOOLBARS = "toolbars";
  static constexpr const char *TOOLBAR_AREA = "area";
  static constexpr const char *TOOLBAR_ACTIONS = "actions";
};

QString SEPARATOR_ID = "separator";

struct MainWindowPrivate {
  MainWindow *_this;
  Core *core;
  bool editMode = false;
  QTimer *toolbarsDisableTimer;

  MainWindowPrivate(MainWindow *mainWindow, Core *core)
      : _this(mainWindow), core(core), toolbarsDisableTimer(new QTimer(_this)) {
    toolbarsDisableTimer->setInterval(
        DockWidget::CHILD_WIDGET_AUTO_DISABLE_DEBOUNCE_TIME);
    toolbarsDisableTimer->setSingleShot(true);
  }

  void connectSignals() {
    QObject::connect(
        core, &Core::projectChanged, _this,
        [=, this](Project *project) { connectProjectSignals(project); });
    connectProjectSignals(core->project());

    QObject::connect(_this->ui->actionEnable_auto_fetch, &QAction::toggled,
                     _this, [=, this](bool toggled) {
                       auto project = _this->core()->project();
                       project->setAutoFetchEnabled(toggled);
                       project->save();
                     });

    _this->connect(_this->ui->actionTop, &QAction::triggered, _this,
                   [=, this] { _this->addToolbar(Qt::TopToolBarArea); });

    _this->connect(_this->ui->actionBottom, &QAction::triggered, _this,
                   [=, this] { _this->addToolbar(Qt::BottomToolBarArea); });

    _this->connect(_this->ui->actionLeft, &QAction::triggered, _this,
                   [=, this] { _this->addToolbar(Qt::LeftToolBarArea); });

    _this->connect(_this->ui->actionRight, &QAction::triggered, _this,
                   [=, this] { _this->addToolbar(Qt::RightToolBarArea); });

    _this->connect(_this->ui->actionAbout_qt, &QAction::triggered, _this,
                   QApplication::aboutQt);

    _this->connect(_this->ui->actionAbout, &QAction::triggered, _this,
                   [this] { about(); });

    _this->connect(_this->ui->actionEdit_mode, &QAction::toggled, _this,
                   [=, this](bool checked) {
                     editMode = checked;
                     auto dockWidgets = _this->findChildren<DockWidget *>();
                     for (auto dockWidget : dockWidgets) {
                       dockWidget->setEditModeEnabled(editMode);
                     }

                     auto toolBars = _this->findChildren<QToolBar *>();
                     for (auto toolbar : toolBars) {
                       toolbar->setMovable(editMode);
                     }
                     _this->ui->tabWidget->setTabsClosable(editMode);
                   });

    _this->connect(_this->ui->actionAdd_tab, &QAction::triggered, _this,
                   [=, this] {
                     QString tabName = QInputDialog::getText(
                         _this, MainWindow::tr("Tab name"),
                         MainWindow::tr("Please enter the new tab's name"));
                     if (!tabName.isNull()) {
                       _this->createTab(tabName);
                     }
                   });

    _this->connect(
        _this->ui->tabWidget, &QTabWidget::tabCloseRequested, _this,
        [=, this](int index) { _this->ui->tabWidget->removeTab(index); });

    _this->connect(
        _this->ui->actionRestore_defaults, &QAction::triggered, _this,
        [=, this] {
          auto response = QMessageBox::question(
              _this, MainWindow::tr("Restore defaults"),
              MainWindow::tr("Do really want to restore the default tab "
                             "and widget configuration?"));
          if (response == QMessageBox::Yes) {
            _this->ui->tabWidget->clear();
            InitialWindowConfiguration::create(_this);
          }
        });

    QObject::connect(
        _this->ui->actionOpen_Project, &QAction::triggered, _this, [=, this] {
          auto fileName = QFileDialog::getOpenFileName(
              _this, MainWindow::tr("Open Project"));

          if (!fileName.isNull()) {
            _this->core()->project()->save();
            _this->core()->changeProject(new Project(fileName, _this->core()));
          }

          populateRecentProjectsMenu();
        });

    QObject::connect(
        _this->ui->actionNew_Project, &QAction::triggered, _this, [=, this] {
          Project *project = new Project(_this->core());
          auto result =
              (new ProjectSettingsDialog(
                   ProjectSettingsDialog::DialogMode::CREATE, project, _this))
                  ->exec();

          if (result == QDialog::Accepted) {
            _this->core()->project()->save();
            _this->core()->changeProject(project);
            populateRecentProjectsMenu();
          }
        });

    QObject::connect(
        _this->ui->actionProject_settings, &QAction::triggered, _this,
        [=, this] {
          (new ProjectSettingsDialog(ProjectSettingsDialog::DialogMode::EDIT,
                                     _this->core()->project(), _this))
              ->exec();
        });

    QObject::connect(
        _this->ui->actionReload_current_repository, &QAction::triggered, _this,
        [=, this] { _this->core()->project()->activeRepository()->reload(); });

    connectMenuToToolbarAction(_this->ui->actionPush,
                               ToolBarActions::ActionID::PUSH);
    connectMenuToToolbarAction(_this->ui->actionPull,
                               ToolBarActions::ActionID::PULL);
    connectMenuToToolbarAction(_this->ui->actionStash_changes,
                               ToolBarActions::ActionID::STASH);
    connectMenuToToolbarAction(_this->ui->actionStash_pop,
                               ToolBarActions::ActionID::UNSTASH);
    connectMenuToToolbarAction(_this->ui->actionClean_up_project,
                               ToolBarActions::ActionID::CLEANUP);

    QObject::connect(
        _this->ui->actionStart_gitk_for_current_repository, &QAction::triggered,
        _this, [=, this] {
          QProcess *process = new QProcess(_this);
#ifdef FLATPAK_BUILD
          process->setProgram("flatpak-spawn");
          process->setArguments(
              {"--host",
               QString("--directory=%1")
                   .arg(_this->core()->project()->activeRepository()->path()),
               "gitk", "--all"});
#else
          process->setProgram("gitk");
          process->setArguments({"--all"});
          process->setWorkingDirectory(
              _this->core()->project()->activeRepository()->path());
#endif
          process->startDetached();
        });

    QObject::connect(
        _this->ui->actionStart_git_gui_for_current_repository,
        &QAction::triggered, _this, [=, this] {
          QProcess *process = new QProcess(_this);
#ifdef FLATPAK_BUILD
          process->setProgram("flatpak-spawn");
          process->setArguments(
              {"--host",
               QString("--directory=%1")
                   .arg(_this->core()->project()->activeRepository()->path()),
               "git", "gui"});
#else
          process->setProgram("git");
          process->setArguments({"gui"});
          process->setWorkingDirectory(
              _this->core()->project()->activeRepository()->path());
#endif
          process->startDetached();
        });

    QObject::connect(toolbarsDisableTimer, &QTimer::timeout, _this, [=, this] {
      auto toolBars = _this->findChildren<QToolBar *>();
      for (auto toolbar : toolBars) {
        toolbar->setDisabled(true);
      }
    });
  }

  inline void connectMenuToToolbarAction(QAction *entry,
                                         const QString &action) {
    QObject::connect(entry, &QAction::triggered, ToolBarActions::byId(action),
                     &QAction::trigger);
  }

  void connectProjectSignals(Project *project) {
    QObject::connect(project, &Project::autoFetchChanged, project,
                     [=, this](bool enabled) {
                       _this->ui->actionEnable_auto_fetch->setChecked(enabled);
                     });
    _this->ui->actionEnable_auto_fetch->setChecked(project->autoFetchEnabled());
  }

  void populateAddViewMenu() {
    auto registeredDockWidgets = DockWidget::registeredDockWidgets();
    for (DockWidget::RegistryEntry *entry : registeredDockWidgets) {
      QAction *action =
          _this->ui->menuAdd_view->addAction(entry->name, _this, [=, this] {
            auto tabDockManager = _this->ui->tabWidget->currentWidget()
                                      ->findChild<ads::CDockManager *>();
            DockWidget::create(entry->id, _this, tabDockManager);
            _this->ui->actionEdit_mode->setChecked(true);
          });
      action->setData(entry->id);
    }
  }

  void populateRecentProjectsMenu() {
    _this->ui->menuRecent_Projects->clear();
    for (auto &[path, name] : _this->core()->recentProjects().toStdMap()) {
      _this->ui->menuRecent_Projects->addAction(
          QString("%1 (%2)").arg(name.toString(), path), _this,
          [=, this, path = path] {
            if (!QFile::exists(path)) {
              QMessageBox::critical(
                  _this, MainWindow::tr("File not found"),
                  MainWindow::tr("File %1 does not exist!").arg(path));
              return;
            }
            _this->core()->project()->save();
            _this->core()->changeProject(new Project(path, _this->core()));
          });
    }

    if (!_this->core()->recentProjects().isEmpty()) {
      _this->ui->menuRecent_Projects->addAction(
          MainWindow::tr("Clear"), _this, [=, this] {
            _this->core()->clearRecentProjects();
            populateRecentProjectsMenu();
          });
    } else {
      auto action = new QAction(MainWindow::tr("No recent projects"));
      action->setEnabled(false);
      _this->ui->menuRecent_Projects->addAction(action);
    }
  }

  void loadConfiguration(const QVariantMap &configuration) {
    _this->restoreGeometry(
        configuration.value(ConfigurationKeys::GEOMETRY).toByteArray());

    QVariantMap tabs = configuration.value(ConfigurationKeys::TABS).toMap();

    for (const auto &tab : tabs.toStdMap()) {
      QVariantMap config = tab.second.toMap();
      QVariantList dockWidgetConfigurations =
          config.value(ConfigurationKeys::DOCK_WIDGETS).toList();

      auto page = _this->createTab(
          config.value(ConfigurationKeys::TAB_NAME).toString());
      auto tabDockManager = page->findChild<ads::CDockManager *>();

      for (const QVariant &dockWidgetConfiguration : dockWidgetConfigurations) {
        QVariantMap config = dockWidgetConfiguration.toMap();
        DockWidget::create(
            config.value(ConfigurationKeys::DOCKWIDGET_CLASS).toString(), _this,
            tabDockManager,
            config.value(ConfigurationKeys::DOCKWIDGET_ID).toString(),
            config.value(ConfigurationKeys::DOCKWIDGET_CONFIGURATION));
      }
      tabDockManager->restoreState(
          config.value(ConfigurationKeys::DOCK_CONFIGURATION).toByteArray());
    }

    QVariantList toolbars =
        configuration.value(ConfigurationKeys::TOOLBARS).toList();
    for (const auto &toolbarConfig : toolbars) {
      QVariantMap config = toolbarConfig.toMap();
      QToolBar *toolbar = _this->addToolbar(static_cast<Qt::ToolBarArea>(
          config.value(ConfigurationKeys::TOOLBAR_AREA).toInt()));
      toolbar->restoreGeometry(
          config.value(ConfigurationKeys::GEOMETRY).toByteArray());

      auto toolBarActions =
          config.value(ConfigurationKeys::TOOLBAR_ACTIONS).toList();
      for (const auto &action : toolBarActions) {
        if (action.toString() == SEPARATOR_ID) {
          toolbar->addSeparator();
        } else {
          toolbar->addAction(ToolBarActions::byId(action.toString()));
        }
      }
      connectToolbarActions(toolbar);
    }

    _this->ui->actionEdit_mode->setChecked(
        configuration.value(ConfigurationKeys::EDIT_MODE, true).toBool());
  }

  void connectToolbarActions(QToolBar *toolbar) {
    QObject::connect(
        core, &Core::projectChanged, toolbar,
        [this](Project *project) { onToolbarProjectChanged(project); });

    onToolbarProjectChanged(core->project());
    onToolbarRepositorySwitched(core->project()->activeRepository(),
                                core->project()->activeRepositoryContext());
  }

  void onToolbarProjectChanged(Project *project) {
    _this->setWindowTitle(QString("git qui - %1").arg(project->name()));
    QObject::connect(project, &Project::repositorySwitched, _this,
                     [this](QSharedPointer<GitInterface> repository,
                            QSharedPointer<QObject> activeRepositoryContext) {
                       onToolbarRepositorySwitched(repository,
                                                   activeRepositoryContext);
                     });
  }

  void
  onToolbarRepositorySwitched(QSharedPointer<GitInterface> gitInterface,
                              QSharedPointer<QObject> activeRepositoryContext) {
    auto toolBars = _this->findChildren<QToolBar *>();
    for (auto toolbar : toolBars) {
      toolbar->setDisabled(gitInterface->actionRunning());
    }

    QObject::connect(
        gitInterface.get(), &GitInterface::actionStarted,
        activeRepositoryContext.get(),
        [=, this](const GitInterface::ActionTag &actionTag) {
          if (!DockWidget::NON_LOCKING_ACTIONS.contains(actionTag)) {
            this->toolbarsDisableTimer->start();
          }
        });
    QObject::connect(gitInterface.get(), &GitInterface::actionFinished,
                     activeRepositoryContext.get(), [=, this] {
                       this->toolbarsDisableTimer->stop();
                       auto toolBars = _this->findChildren<QToolBar *>();
                       for (auto toolbar : toolBars) {
                         toolbar->setDisabled(false);
                       }
                     });
  }

  void about() {
    QMessageBox::about(_this, MainWindow::tr("About git qui"),
                       MainWindow::tr("qt5 ui replacement for git-gui. \n\n"
                                      "Application Version: %1")
                           .arg(
#if defined(GIT_VERSION)
                               GIT_VERSION
#else
                               "development"
#endif
                               ));
  }
};

MainWindow::MainWindow(Core *core, const QVariantMap &configuration)
    : QMainWindow(nullptr), ui(new Ui::MainWindow),
      _impl(new MainWindowPrivate(this, core)) {
  ui->setupUi(this);
  ads::CDockManager::setConfigFlag(
      ads::CDockManager::DockAreaHideDisabledButtons, true);
  ads::CDockManager::setConfigFlag(
      ads::CDockManager::DockAreaDynamicTabsMenuButtonVisibility, true);
  ads::CDockManager::setAutoHideConfigFlags(
      ads::CDockManager::DefaultAutoHideConfig);
  _impl->connectSignals();
  _impl->loadConfiguration(configuration);
  _impl->populateAddViewMenu();
  _impl->populateRecentProjectsMenu();
}

MainWindow::~MainWindow() { delete ui; }

Core *MainWindow::core() { return _impl->core; }

QVariant MainWindow::configuration() const {
  QVariantMap configuration = {{ConfigurationKeys::GEOMETRY, saveGeometry()},
                               {ConfigurationKeys::STATE, saveState()},
                               {ConfigurationKeys::EDIT_MODE, _impl->editMode}};

  QVariantMap tabs;

  for (int x = 0; x < ui->tabWidget->count(); ++x) {
    QVariantList dockWidgetConfigurations;
    auto tab = ui->tabWidget->widget(x);
    auto tabDockManager = tab->findChild<ads::CDockManager *>();

    auto dockWidgets = tab->findChildren<DockWidget *>();
    for (auto dockWidget : dockWidgets) {
      dockWidgetConfigurations.append(QVariantMap(
          {{ConfigurationKeys::DOCKWIDGET_CLASS,
            dockWidget->metaObject()->className()},
           {ConfigurationKeys::DOCKWIDGET_ID, dockWidget->objectName()},
           {ConfigurationKeys::DOCKWIDGET_CONFIGURATION,
            dockWidget->configuration()}}));
    }

    tabs.insert(
        QString::number(x),
        QVariantMap(
            {{ConfigurationKeys::DOCK_WIDGETS, dockWidgetConfigurations},
             {ConfigurationKeys::DOCK_CONFIGURATION,
              tabDockManager->saveState()},
             {ConfigurationKeys::TAB_NAME, ui->tabWidget->tabText(x)}}));
  }
  configuration.insert(ConfigurationKeys::TABS, tabs);

  QVariantList toolbars;
  auto toolBars = findChildren<QToolBar *>();
  for (auto toolbar : toolBars) {
    QVariantList actions;
    auto toolBarActions = toolbar->actions();
    for (auto action : toolBarActions) {
      if (action->data().isNull()) {
        actions.push_back(SEPARATOR_ID);
      } else {
        actions.push_back(action->data());
      }
    }

    QVariantMap config = {
        {ConfigurationKeys::GEOMETRY, toolbar->geometry()},
        {ConfigurationKeys::TOOLBAR_AREA, toolBarArea(toolbar)},
        {ConfigurationKeys::TOOLBAR_ACTIONS, actions},
    };
    toolbars.append(config);
  }
  configuration.insert(ConfigurationKeys::TOOLBARS, toolbars);

  return configuration;
}

QToolBar *MainWindow::addToolbar(Qt::ToolBarArea area) {
  QToolBar *toolbar = new QToolBar(this);
  toolbar->setContextMenuPolicy(Qt::CustomContextMenu);
  toolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  toolbar->setMovable(_impl->editMode);

  connect(toolbar, &QToolBar::customContextMenuRequested, this,
          [=, this](const QPoint &pos) {
            QAction *configAction =
                new QAction(tr("Configure toolbar..."), toolbar);
            connect(configAction, &QAction::triggered, this,
                    [=, this] { (new ToolBarEditor(toolbar))->show(); });
            QAction *removeAction = new QAction(tr("Remove toolbar"), toolbar);
            connect(removeAction, &QAction::triggered, this, [=, this] {
              removeToolBar(toolbar);
              toolbar->deleteLater();
            });

            QMenu *menu = new QMenu(toolbar);

            menu->addActions({
                configAction,
                removeAction,
            });

            menu->popup(toolbar->mapToGlobal(pos));
          });

  _impl->connectToolbarActions(toolbar);

  addToolBar(area, toolbar);
  return toolbar;
}

QMainWindow *MainWindow::createTab(const QString &title) {
  QMainWindow *tab = new QMainWindow(ui->tabWidget);
  new ads::CDockManager(tab);
  ui->tabWidget->addTab(tab, title);
  return tab;
}

void MainWindow::setEditMode(bool enabled) {
  _impl->editMode = enabled;
  ui->actionEdit_mode->setChecked(enabled);
}

bool MainWindow::event(QEvent *ev) {
  switch (ev->type()) {
  case QEvent::ActivationChange:
    core()->project()->reloadAllRepositories();
    break;
  case QEvent::Close:
    qApp->quit();
    return true;
  default:
    break;
  }

  return QMainWindow::event(ev);
}

DockWidget *MainWindow::addDockWidget(const QString &className, int tabIndex,
                                      const QVariant &configuration,
                                      ads::DockWidgetArea area,
                                      const QString &uuid) {
  auto tabDockManager =
      ui->tabWidget->widget(tabIndex)->findChild<ads::CDockManager *>();

  return DockWidget::create(className, this, tabDockManager, uuid,
                            configuration, area);
}
