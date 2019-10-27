#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QToolBar>
#include <QDir>
#include <QDockWidget>
#include <QDebug>
#include <QSettings>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QLabel>
#include <QMovie>
#include <QMap>
#include <QtConcurrent/QtConcurrent>
#include <QInputDialog>
#include <QTimer>
#include <QDirIterator>

#include "gitinterface.hpp"
#include "components/dockwidget.hpp"
#include "toolbareditor.hpp"
#include "toolbaractions.hpp"
#include "project.hpp"
#include "projectsettingsdialog.hpp"

struct MainWindowPrivate
{
  GitInterface* selectedGitInterface;
  QMap<QString, GitInterface*> gitInterfaces;
  QList<QString> repositories;
  int currentRepository = 0;
  QTimer *autoFetchTimer;
  bool editMode;
  Project *activeProject = nullptr;

  inline static const QString CONFIG_GEOMETRY = "geometry";
  inline static const QString CONFIG_STATE = "state";
  inline static const QString CONFIG_TABS = "tabs";
  inline static const QString CONFIG_TAB_NAME = "tabName";
  inline static const QString CONFIG_DOCK_WIDGETS = "dockWidgets";
  inline static const QString CONFIG_REPOSITORIES = "repositories";
  inline static const QString CONFIG_CURRENT_REPOSITORY = "currentRepository";
  inline static const QString CONFIG_DW_ID = "id";
  inline static const QString CONFIG_DW_CLASS = "class";
  inline static const QString CONFIG_DW_CONFIGURATION = "configuration";
  inline static const QString CONFIG_EDIT_MODE = "editMode";
  inline static const QString CONFIG_CURRENT_PROJECT_PATH = "currentProjectPath";

  void initGit(MainWindow *_this)
  {
    QSettings settings;
    QString projectPath = settings.value(CONFIG_CURRENT_PROJECT_PATH).toString();

    if (!QFile::exists(projectPath))
    {
      auto response = QMessageBox::information(
        _this,
        QObject::tr("No Project selected"),
        QObject::tr("Would you like to create a new project? Alternatively, you could open an existing one."),
        QMessageBox::Yes | QMessageBox::Open | QMessageBox::Abort
      );

      switch (response)
      {
      case QMessageBox::Yes:
      {
        Project *project = new Project();
        auto settingsDialog = new ProjectSettingsDialog(ProjectSettingsDialog::DialogMode::CREATE, project, _this);
        if (settingsDialog->exec() == QDialog::Accepted)
        {
          activeProject = project;
        }
        break;
      }
      case QMessageBox::Open:
      {
        QString fileName = QFileDialog::getOpenFileName(_this, QObject::tr("Select project to open"));

        if (!fileName.isEmpty())
        {
          activeProject = new Project(fileName, _this);
        }
        break;
      }
      default:
        break;
      }

      if (!activeProject)
      {
        QMessageBox message;
        message.setText(QObject::tr("No project selected! Closing."));
        message.setIcon(QMessageBox::Critical);
        message.exec();
        exit(EXIT_FAILURE);
      }
    }
    else
    {
      activeProject = new Project(projectPath, _this);
    }

    repositories = settings.value(CONFIG_REPOSITORIES).toStringList();

    if (repositories.empty() && !selectRepository(_this))
    {
      QMessageBox message;
      message.setText(_this->tr("No repository selected! Closing."));
      message.setIcon(QMessageBox::Critical);
      message.exec();
      exit(EXIT_FAILURE);
    }
    else
    {
      for (auto repository : repositories)
      {
        gitInterfaces.insert(repository, new GitInterface(_this, repository));
        addRepositoryMenuEntry(_this, repository);
      }
    }

    currentRepository = std::min(settings.value(CONFIG_CURRENT_REPOSITORY, 0).toInt(), repositories.size() - 1);
    selectedGitInterface = gitInterfaces.value(repositories.at(currentRepository), nullptr);
  }

  bool selectRepository(MainWindow *_this)
  {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    dialog.setWindowTitle(dialog.tr("Select repository to open"));
    dialog.exec();

    if (dialog.result() == QFileDialog::Accepted)
    {
      QDirIterator iterator(
        dialog.directory().absolutePath(),
        {".git"},
        QDir::Dirs | QDir::Hidden,
        QDirIterator::Subdirectories
      );

      while (iterator.hasNext())
      {
        QDir currentDir = QDir(iterator.next());
        currentDir.cdUp();
        QString path = currentDir.absolutePath();
        if (repositories.indexOf(path) == -1)
        {
          auto inserted = gitInterfaces.insert(path, new GitInterface(_this, path));
          repositories.append(path);
          addRepositoryMenuEntry(_this, path);
          emit _this->repositoryAdded(inserted.value());
        }
      }
      return true;
    }

    return false;
  }

  void addRepositoryMenuEntry(MainWindow *_this, const QString &path)
  {
    _this->ui->menuRepositories->addAction(path, _this, [=]{
      selectedGitInterface = gitInterfaces.value(path, nullptr);
      currentRepository = repositories.indexOf(path);
    });
  }

  void closeCurrentRepository(MainWindow *_this)
  {
    QString path = repositories.at(currentRepository);
    _this->ui->menuRepositories->removeAction(_this->ui->menuRepositories->actions().at(currentRepository));
    repositories.removeAt(currentRepository);
    emit _this->repositoryRemoved(gitInterfaces.value(path, nullptr));
    selectedGitInterface->disconnect(nullptr, _this);
    gitInterfaces.remove(path);

    if (repositories.empty())
    {
      if (!selectRepository(_this))
      {
        QMessageBox message;
        message.setText(_this->tr("No repository selected! Closing."));
        message.setIcon(QMessageBox::Critical);
        message.exec();
        saveSettings(_this);
        exit(EXIT_FAILURE);
      }
    }

    currentRepository = 0;
    selectedGitInterface = gitInterfaces.value(repositories.first(), nullptr);
    emit _this->repositorySwitched(selectedGitInterface);
  }

  void connectSignals(MainWindow *_this)
  {
    QSettings settings;

    _this->connect(_this->ui->actionTop, &QAction::triggered, _this, [=]{
      addToolbar(Qt::TopToolBarArea, _this);
    });

    _this->connect(_this->ui->actionBottom, &QAction::triggered, _this, [=]{
      addToolbar(Qt::BottomToolBarArea, _this);
    });

    _this->connect(_this->ui->actionLeft, &QAction::triggered, _this, [=]{
      addToolbar(Qt::LeftToolBarArea, _this);
    });

    _this->connect(_this->ui->actionRight, &QAction::triggered, _this, [=]{
      addToolbar(Qt::RightToolBarArea, _this);
    });

    _this->connect(_this->ui->actionReload_current_repository, &QAction::triggered, _this, [this]{
      selectedGitInterface->reload();
    });

    QObject::connect(_this->ui->actionNew_Project, &QAction::triggered, _this, [this,  _this]{
      Project *project = new Project();
      auto settingsDialog = new ProjectSettingsDialog(ProjectSettingsDialog::DialogMode::CREATE, project, _this);
      if (settingsDialog->exec() == QDialog::Accepted)
      {
        activeProject = project;
      }
    });

    QObject::connect(_this->ui->actionOpen_Project, &QAction::triggered, _this, [this,  _this]{
      QString fileName = QFileDialog::getOpenFileName(_this, "Select project to open");

      if (!fileName.isEmpty())
      {
        activeProject = new Project(fileName, _this);
      }
    });

    QObject::connect(_this->ui->actionProject_settings, &QAction::triggered, _this, [this,  _this]{
      (new ProjectSettingsDialog(ProjectSettingsDialog::DialogMode::EDIT, activeProject, _this))->exec();
    });

    _this->connect(_this->ui->actionOpen_Repository, &QAction::triggered, _this, [=]{
      selectRepository(_this);
    });
    _this->connect(_this->ui->actionClose_current_repository, &QAction::triggered, _this, [=]{
      closeCurrentRepository(_this);
    });

    _this->connect(_this->ui->actionStart_git_gui_for_current_repository, &QAction::triggered, _this, [=]{
      QProcess *process = new QProcess(_this);
      process->setProgram("git");
      process->setArguments({"gui"});
      process->setWorkingDirectory(repositories.at(currentRepository));
      process->startDetached();
    });

    _this->connect(_this->ui->actionStart_gitk_for_current_repository, &QAction::triggered, _this, [=]{
      QProcess *process = new QProcess(_this);
      process->setProgram("gitk");
      process->setArguments({"--all"});
      process->setWorkingDirectory(repositories.at(currentRepository));
      process->startDetached();
    });

    _this->connect(_this->ui->actionQuick_cleanup, &QAction::triggered, _this, [=]{
      QList<QString> branches;
      for (auto branch : selectedGitInterface->branches({"--merged", "master"}))
      {
        if (branch.name != "master")
        {
          branches.append(QString("<b>%1</b>").arg(branch.name));
        }
      }

      if (branches.isEmpty())
      {
        QMessageBox::information(
          _this,
          _this->tr("Quick cleanup"),
          _this->tr("There are no branches for cleanup.")
        );
        return;
      }

      QMessageBox dialog(
        QMessageBox::Warning,
        _this->tr("Quick cleanup"),
        _this->tr("Would you like to remove the following local branches? "
          "They are merged into master and have no upstream branch associated."
          "<br><br> %1").arg(branches.join("<br>")),
        QMessageBox::Yes | QMessageBox::No,
        _this
      );

      dialog.setTextFormat(Qt::RichText);
      if (dialog.exec() == QMessageBox::Yes)
      {
        for (auto branch: branches)
        {
          selectedGitInterface->deleteBranch(branch.remove(QRegExp("<[^>]*>")));
        }
        selectedGitInterface->reload();
      }
    });

    _this->connect(_this->ui->actionPush, &QAction::triggered, ToolBarActions::byId("push"), &QAction::trigger);
    _this->connect(_this->ui->actionPull_Rebase, &QAction::triggered, ToolBarActions::byId("pull"), &QAction::trigger);

    _this->connect(_this->ui->actionAbout_qt, &QAction::triggered, _this, QApplication::aboutQt);

    _this->connect(_this->ui->actionEdit_mode, &QAction::toggled, _this, [=](bool checked){
      editMode = checked;
      for (auto dockWidget : _this->findChildren<DockWidget*>())
      {
        dockWidget->setEditModeEnabled(editMode);
      }

      for(auto toolbar : _this->findChildren<QToolBar*>())
      {
        toolbar->setMovable(editMode);
      }
      _this->ui->tabWidget->setTabsClosable(editMode);
    });

    _this->connect(_this->ui->actionAdd_tab, &QAction::triggered, _this, [=]{
      QString tabName = QInputDialog::getText(
        _this,
        _this->tr("Tab name"),
        _this->tr("Please enter the new tab's name")
      );
      if (!tabName.isNull())
      {
        QMainWindow *tab = createTab(_this);
        _this->ui->tabWidget->addTab(tab, tabName);
      }
    });

    _this->connect(_this->ui->tabWidget, &QTabWidget::tabCloseRequested, _this, [=](int index){
      _this->ui->tabWidget->removeTab(index);
    });

    _this->connect(_this->ui->actionRestore_defaults, &QAction::triggered, _this, [=]{
      auto response = QMessageBox::question(
        _this,
        _this->tr("Restore defaults"),
        _this->tr("Do really want to restore the default tab and widget configuration?")
      );
      if (response == QMessageBox::Yes)
      {
        _this->ui->tabWidget->clear();
        initFirstTimeConfig(_this);
      }
    });

    _this->connect(_this->ui->actionStash_changes, &QAction::triggered, ToolBarActions::byId("stash"), &QAction::trigger);
    _this->connect(_this->ui->actionStash_pop, &QAction::triggered, ToolBarActions::byId("unstash"), &QAction::trigger);
  }

  QToolBar *addToolbar(Qt::ToolBarArea area, MainWindow *_this)
  {
    QToolBar *toolbar = new QToolBar(_this);
    toolbar->setContextMenuPolicy(Qt::CustomContextMenu);
    toolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
    toolbar->setMovable(editMode);

    _this->connect(toolbar, &QToolBar::customContextMenuRequested, _this, [=](const QPoint &pos){
      QAction *configAction = new QAction(_this->tr("Configure toolbar..."), toolbar);
      _this->connect(configAction, &QAction::triggered, _this, [=]{
        (new ToolBarEditor(toolbar))->show();
      });
      QAction *removeAction = new QAction(_this->tr("Remove toolbar"), toolbar);
      _this->connect(removeAction, &QAction::triggered, _this, [=]{
        _this->removeToolBar(toolbar);
        toolbar->deleteLater();
      });

      QMenu *menu = new QMenu(toolbar);

      menu->addActions({
        configAction,
        removeAction,
      });

      menu->popup(toolbar->mapToGlobal(pos));
    });

    _this->addToolBar(area, toolbar);
    return toolbar;
  }

  void initAutoFetchTimer(MainWindow *_this)
  {
    autoFetchTimer = new QTimer(_this);
    autoFetchTimer->setInterval(30000);
    auto timeout = [=]{
      for (auto interface : gitInterfaces)
      {
        QtConcurrent::run([=]{
          interface->fetch();
        });
      }
    };
    _this->connect(autoFetchTimer, &QTimer::timeout, _this, timeout);
    timeout();
    autoFetchTimer->start();
  }

  void connectGitInterfaceSignals(MainWindow *_this, GitInterface *gitInterface)
  {
    _this->connect(gitInterface, &GitInterface::reloaded, _this, [=]{
      emit _this->repositoryAdded(gitInterfaces.value(gitInterface->path()));
    });
  }

  void populateMenu(MainWindow *_this)
  {
    for (DockWidget::RegistryEntry *entry : DockWidget::registeredDockWidgets())
    {
      QAction *action = _this->ui->menuAdd_view->addAction(entry->name, [=]{
        DockWidget::create(
          entry->id,
          _this,
          static_cast<QMainWindow*>(_this->ui->tabWidget->currentWidget()),
          selectedGitInterface
        );
        emit _this->repositorySwitched(selectedGitInterface);
        for (auto interface : gitInterfaces) {
          emit _this->repositoryAdded(interface);
          interface->reload();
        }
        _this->ui->actionEdit_mode->setChecked(true);
      });
      action->setData(entry->id);
    }
  }

  QMainWindow *createTab(MainWindow *_this)
  {
    QMainWindow *tab = new QMainWindow(_this->ui->tabWidget);
    tab->setDockOptions(
      QMainWindow::AllowNestedDocks |
      QMainWindow::AllowTabbedDocks |
      QMainWindow::AnimatedDocks |
      QMainWindow::GroupedDragging
    );
    return tab;
  }

  void initFirstTimeConfig(MainWindow *_this)
  {
    QMainWindow *main = createTab(_this);
    QMap<QString, QVariant> unstagedConfig = {{"unstaged", true}};
    DockWidget::create(
      "RepositoryFiles",
      _this,
      main,
      selectedGitInterface,
      QUuid::createUuid().toString(),
      unstagedConfig
    );
    QMap<QString, QVariant> stagedConfig = {{"unstaged", false}};
    DockWidget::create(
      "RepositoryFiles",
      _this,
      main,
      selectedGitInterface,
      QUuid::createUuid().toString(),
      stagedConfig
    );
    DockWidget::create(
      "DiffView",
      _this,
      main,
      selectedGitInterface
    );
    DockWidget::create(
      "Commit",
      _this,
      main,
      selectedGitInterface
    );
    DockWidget::create(
      "RepositoryList",
      _this,
      main,
      selectedGitInterface
    );
    DockWidget::create(
      "BranchList",
      _this,
      main,
      selectedGitInterface
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

    _this->ui->tabWidget->addTab(main, _this->tr("Main"));

    QMainWindow *history = createTab(_this);
    DockWidget::create(
      "LogView",
      _this,
      history,
      selectedGitInterface
    );
    _this->ui->tabWidget->addTab(history, _this->tr("History"));

    _this->ui->actionEdit_mode->setChecked(true);

    for (auto toolbar : _this->findChildren<QToolBar*>())
    {
      _this->removeToolBar(toolbar);
      toolbar->deleteLater();
    }

    auto toolbar = addToolbar(Qt::ToolBarArea::TopToolBarArea, _this);
    toolbar->addAction(ToolBarActions::byId("pull"));
    toolbar->addAction(ToolBarActions::byId("push"));
    toolbar->addAction(ToolBarActions::byId("pull-all"));
    toolbar->addAction(ToolBarActions::byId("push-all"));
    toolbar->addAction(ToolBarActions::byId("new-branch"));
    toolbar->addAction(ToolBarActions::byId("stash"));
    toolbar->addAction(ToolBarActions::byId("unstash"));
  }

  void restoreSettings(MainWindow *_this)
  {
    QSettings settings;

    if (!settings.contains(CONFIG_GEOMETRY))
    {
      initFirstTimeConfig(_this);
      return;
    }

    _this->restoreGeometry(settings.value(CONFIG_GEOMETRY).toByteArray());

    QMap<QString, QVariant> tabs =
      settings.value(CONFIG_TABS).toMap();

    for (auto tab : tabs.toStdMap())
    {
      QMap<QString, QVariant> config = tab.second.toMap();
      QList<QVariant> dockWidgetConfigurations = config.value(CONFIG_DOCK_WIDGETS).toList();

      QMainWindow *page = createTab(_this);

      for (QVariant dockWidgetConfiguration : dockWidgetConfigurations)
      {
        QMap<QString, QVariant> config = dockWidgetConfiguration.toMap();
        DockWidget::create(
          config.value(CONFIG_DW_CLASS).toString(),
          _this,
          page,
          selectedGitInterface,
          config.value(CONFIG_DW_ID).toString(),
          config.value(CONFIG_DW_CONFIGURATION)
        );
      }
      page->restoreState(config.value(CONFIG_STATE).toByteArray());
      page->restoreGeometry(config.value(CONFIG_GEOMETRY).toByteArray());
      _this->ui->tabWidget->addTab(page, config.value(CONFIG_TAB_NAME).toString());
    }

    QList<QVariant> toolbars = settings.value("toolbars").toList();
    for (auto toolbarConfig: toolbars)
    {
      QMap<QString, QVariant> config = toolbarConfig.toMap();
      QToolBar *toolbar = addToolbar(static_cast<Qt::ToolBarArea>(config.value("area").toInt()), _this);
      toolbar->restoreGeometry(config.value("geometry").toByteArray());

      for (auto action : config.value("actions").toList())
      {
        toolbar->addAction(ToolBarActions::byId(action.toString()));
      }
    }

    _this->ui->actionEdit_mode->setChecked(settings.value(CONFIG_EDIT_MODE, true).toBool());
  }

  void postInit()
  {
    selectedGitInterface->reload();

    for (auto interface : gitInterfaces)
    {
      if (interface != selectedGitInterface) {
        QtConcurrent::run([interface]{
          interface->reload();
        });
      }
    }
  }

  void saveSettings(MainWindow *_this)
  {
    QSettings settings;
    settings.setValue(CONFIG_GEOMETRY, _this->saveGeometry());
    settings.setValue(CONFIG_STATE, _this->saveState());
    settings.setValue(CONFIG_REPOSITORIES, QVariant(repositories));
    settings.setValue(CONFIG_CURRENT_REPOSITORY, QVariant(currentRepository));
    settings.setValue(CONFIG_EDIT_MODE, editMode);
    settings.setValue(CONFIG_CURRENT_PROJECT_PATH, activeProject->fileName());

    QMap<QString, QVariant> tabs;

    for (int x = 0; x < _this->ui->tabWidget->count(); ++x)
    {
      QMap<QString, QVariant> config;
      QList<QVariant> dockWidgetConfigurations;
      QMainWindow *tab = static_cast<QMainWindow*>(_this->ui->tabWidget->widget(x));

      for (auto dockWidget : tab->findChildren<DockWidget*>())
      {
        QMap<QString, QVariant> configuration;
        configuration.insert(
          CONFIG_DW_CLASS, dockWidget->metaObject()->className()
        );
        configuration.insert(
          CONFIG_DW_ID, dockWidget->objectName()
        );
        configuration.insert(
          CONFIG_DW_CONFIGURATION, dockWidget->configuration()
        );
        dockWidgetConfigurations.append(configuration);
        delete dockWidget;
      }

      config.insert(CONFIG_DOCK_WIDGETS, dockWidgetConfigurations);
      config.insert(CONFIG_STATE, tab->saveState());
      config.insert(CONFIG_GEOMETRY, tab->saveGeometry());
      config.insert(CONFIG_TAB_NAME, _this->ui->tabWidget->tabText(x));
      tabs.insert(QString::number(x), config);
    }

    QList<QVariant> toolbars;
    for (auto toolbar : _this->findChildren<QToolBar*>())
    {
      QList<QVariant> actions;
      for (auto action: toolbar->actions())
      {
        actions.push_back(action->data());
      }

      QMap<QString, QVariant> config = {
        {"area", _this->toolBarArea(toolbar)},
        {"actions", actions},
      };
      toolbars.append(config);
    }
    settings.setValue("toolbars", toolbars);

    settings.setValue(CONFIG_TABS, tabs);
  }
};

#include <QSvgWidget>

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow),
_impl(new MainWindowPrivate)
{
  ui->setupUi(this);

  ToolBarActions::initialize(this);

  _impl->initGit(this);
  _impl->connectSignals(this);
  _impl->initAutoFetchTimer(this);

  for (auto interface : _impl->gitInterfaces) {
    _impl->connectGitInterfaceSignals(this, interface);
  }

  _impl->populateMenu(this);
  _impl->restoreSettings(this);

  for (auto interface : _impl->gitInterfaces) {
    emit repositoryAdded(interface);
  }

  _impl->postInit();
  emit repositorySwitched(_impl->selectedGitInterface);
}

MainWindow::~MainWindow()
{
  _impl->saveSettings(this);
  QThreadPool::globalInstance()->clear();
  QThreadPool::globalInstance()->waitForDone();

  for (auto interface : _impl->gitInterfaces)
  {
    interface->disconnect(nullptr, this);
  }

  delete ui;
}

const QList<GitInterface*> MainWindow::repositories() const
{
  return _impl->gitInterfaces.values();
}

void MainWindow::openRepository()
{
  _impl->selectRepository(this);
}

void MainWindow::closeCurrentRepository()
{
  _impl->closeCurrentRepository(this);
}

void MainWindow::switchRepository(const QString &path)
{
  _impl->currentRepository = _impl->repositories.indexOf(path);
  _impl->selectedGitInterface = _impl->gitInterfaces.value(path, nullptr);
  emit repositorySwitched(_impl->selectedGitInterface);
  _impl->selectedGitInterface->reload();

  setWindowTitle(QString("git-qui - %1").arg(path.split('/').last()));
}

void MainWindow::changeEvent(QEvent *ev)
{
  if (ev->type() == QEvent::ActivationChange && isActiveWindow())
  {
    _impl->postInit();
  }
}
