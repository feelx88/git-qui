#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QToolBar>
#include <QDir>
#include <QDockWidget>
#include <QSettings>
#include <QStatusBar>
#include <QMessageBox>
#include <QProcess>
#include <QLabel>
#include <QMovie>
#include <QMap>
#include <QtConcurrent/QtConcurrent>
#include <QInputDialog>
#include <QTimer>
#include <QDirIterator>
#include <QSvgWidget>
#include <QFileDialog>

#include "core.hpp"
#include "gitinterface.hpp"
#include "components/dockwidget.hpp"
#include "toolbareditor.hpp"
#include "toolbaractions.hpp"
#include "project.hpp"
#include "projectsettingsdialog.hpp"
#include "initialwindowconfiguration.hpp"

struct ConfigurationKeys
{
  static constexpr const char *GEOMETRY = "geometry";
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

struct MainWindowPrivate
{
  MainWindow *_this;
  Core *core;
  bool editMode;

  MainWindowPrivate(MainWindow *mainWindow, Core *core)
    : _this(mainWindow),
      core(core)
  {}

  void connectSignals()
  {
    QSettings settings;

    _this->connect(_this->ui->actionTop, &QAction::triggered, _this, [=]{
      _this->addToolbar(Qt::TopToolBarArea);
    });

    _this->connect(_this->ui->actionBottom, &QAction::triggered, _this, [=]{
      _this->addToolbar(Qt::BottomToolBarArea);
    });

    _this->connect(_this->ui->actionLeft, &QAction::triggered, _this, [=]{
      _this->addToolbar(Qt::LeftToolBarArea);
    });

    _this->connect(_this->ui->actionRight, &QAction::triggered, _this, [=]{
      _this->addToolbar(Qt::RightToolBarArea);
    });

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
        _this->createTab(tabName);
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
        InitialWindowConfiguration::create(_this);
      }
    });

    QObject::connect(_this->ui->actionOpen_Project, &QAction::triggered, _this, [=]{
      auto fileName = QFileDialog::getOpenFileName(_this, QObject::tr("Open Project"));

      if (!fileName.isNull())
      {
        _this->core()->project()->save();
        _this->core()->changeProject(new Project(fileName, _this->core()));
      }

      populateRecentProjectsMenu();
    });

    QObject::connect(_this->ui->actionProject_settings, &QAction::triggered, _this, [=]{
      (new ProjectSettingsDialog(ProjectSettingsDialog::DialogMode::EDIT, _this->core()->project(), _this))->exec();
    });

    QObject::connect(_this->ui->actionStart_gitk_for_current_repository, &QAction::triggered, _this, [=]{
      QProcess *process = new QProcess(_this);
      process->setProgram("gitk");
      process->setArguments({"--all"});
      process->setWorkingDirectory(_this->core()->project()->activeRepository()->path());
      process->startDetached();
    });

    QObject::connect(_this->ui->actionStart_git_gui_for_current_repository, &QAction::triggered, _this, [=]{
      QProcess *process = new QProcess(_this);
      process->setProgram("git");
      process->setArguments({"gui"});
      process->setWorkingDirectory(_this->core()->project()->activeRepository()->path());
      process->startDetached();
    });
  }

  void populateAddViewMenu()
  {
    for (DockWidget::RegistryEntry *entry : DockWidget::registeredDockWidgets())
    {
      QAction *action = _this->ui->menuAdd_view->addAction(entry->name, [=]{
        DockWidget::create(
          entry->id,
          _this,
          static_cast<QMainWindow*>(_this->ui->tabWidget->currentWidget())
        );
        _this->ui->actionEdit_mode->setChecked(true);
      });
      action->setData(entry->id);
    }
  }

  void populateRecentProjectsMenu()
  {
    _this->ui->menuRecent_Projects->clear();
    for (auto &[path, name]: _this->core()->recentProjects().toStdMap())
    {
      _this->ui->menuRecent_Projects->addAction(QString("%1 (%2)").arg(name.toString()).arg(path), [=, path = path]{
        if (!QFile::exists(path))
        {
          QMessageBox::critical(_this, QObject::tr("File not found"), QObject::tr("File %1 does not exist!").arg(path));
          return;
        }
        _this->core()->project()->save();
        _this->core()->changeProject(new Project(path, _this->core()));
      });
    }
  }

  void loadConfiguration(const QVariantMap &configuration)
  {
    _this->restoreGeometry(configuration.value(ConfigurationKeys::GEOMETRY).toByteArray());

    QVariantMap tabs =
      configuration.value(ConfigurationKeys::TABS).toMap();

    for (auto tab : tabs.toStdMap())
    {
      QVariantMap config = tab.second.toMap();
      QVariantList dockWidgetConfigurations = config.value(ConfigurationKeys::DOCK_WIDGETS).toList();

      QMainWindow *page = _this->createTab(config.value(ConfigurationKeys::TAB_NAME).toString());

      for (QVariant dockWidgetConfiguration : dockWidgetConfigurations)
      {
        QVariantMap config = dockWidgetConfiguration.toMap();
        DockWidget::create(
          config.value(ConfigurationKeys::DOCKWIDGET_CLASS).toString(),
          _this,
          page,
          config.value(ConfigurationKeys::DOCKWIDGET_ID).toString(),
          config.value(ConfigurationKeys::DOCKWIDGET_CONFIGURATION)
        );
      }
      page->restoreState(config.value(ConfigurationKeys::STATE).toByteArray());
      page->restoreGeometry(config.value(ConfigurationKeys::GEOMETRY).toByteArray());
    }

    QVariantList toolbars = configuration.value(ConfigurationKeys::TOOLBARS).toList();
    for (auto toolbarConfig: toolbars)
    {
      QVariantMap config = toolbarConfig.toMap();
      QToolBar *toolbar = _this->addToolbar(static_cast<Qt::ToolBarArea>(config.value(ConfigurationKeys::TOOLBAR_AREA).toInt()));
      toolbar->restoreGeometry(config.value(ConfigurationKeys::GEOMETRY).toByteArray());

      for (auto action : config.value(ConfigurationKeys::TOOLBAR_ACTIONS).toList())
      {
        toolbar->addAction(ToolBarActions::byId(action.toString()));
      }
    }

    _this->ui->actionEdit_mode->setChecked(configuration.value(ConfigurationKeys::EDIT_MODE, true).toBool());
  }
};

MainWindow::MainWindow(Core *core, const QVariantMap &configuration) :
  QMainWindow(nullptr),
  ui(new Ui::MainWindow),
  _impl(new MainWindowPrivate(this, core))
{
  ui->setupUi(this);
  _impl->connectSignals();
  _impl->loadConfiguration(configuration);
  _impl->populateAddViewMenu();
  _impl->populateRecentProjectsMenu();
}

MainWindow::~MainWindow()
{
  delete ui;
}

Core *MainWindow::core()
{
  return _impl->core;
}

QVariant MainWindow::configuration() const
{
  QVariantMap configuration = {
    {ConfigurationKeys::GEOMETRY, saveGeometry()},
    {ConfigurationKeys::STATE, saveState()},
    {ConfigurationKeys::EDIT_MODE, _impl->editMode}
  };

  QVariantMap tabs;

  for (int x = 0; x < ui->tabWidget->count(); ++x)
  {
    QVariantMap config;
    QVariantList dockWidgetConfigurations;
    QMainWindow *tab = static_cast<QMainWindow*>(ui->tabWidget->widget(x));

    for (auto dockWidget : tab->findChildren<DockWidget*>())
    {
      dockWidgetConfigurations.append(QVariantMap({
        {ConfigurationKeys::DOCKWIDGET_CLASS, dockWidget->metaObject()->className()},
        {ConfigurationKeys::DOCKWIDGET_ID, dockWidget->objectName()},
        {ConfigurationKeys::DOCKWIDGET_CONFIGURATION, dockWidget->configuration()}
      }));
    }

    tabs.insert(QString::number(x), QVariantMap({
      {ConfigurationKeys::DOCK_WIDGETS, dockWidgetConfigurations},
      {ConfigurationKeys::STATE, tab->saveState()},
      {ConfigurationKeys::GEOMETRY, tab->saveGeometry()},
      {ConfigurationKeys::TAB_NAME, ui->tabWidget->tabText(x)}
    }));
  }
  configuration.insert(ConfigurationKeys::TABS, tabs);

  QVariantList toolbars;
  for (auto toolbar : findChildren<QToolBar*>())
  {
    QVariantList actions;
    for (auto action: toolbar->actions())
    {
      actions.push_back(action->data());
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

QToolBar *MainWindow::addToolbar(Qt::ToolBarArea area)
{
  QToolBar *toolbar = new QToolBar(this);
  toolbar->setContextMenuPolicy(Qt::CustomContextMenu);
  toolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  toolbar->setMovable(_impl->editMode);

  connect(toolbar, &QToolBar::customContextMenuRequested, this, [=](const QPoint &pos){
    QAction *configAction = new QAction(tr("Configure toolbar..."), toolbar);
    connect(configAction, &QAction::triggered, this, [=]{
      (new ToolBarEditor(toolbar))->show();
    });
    QAction *removeAction = new QAction(tr("Remove toolbar"), toolbar);
    connect(removeAction, &QAction::triggered, this, [=]{
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

  addToolBar(area, toolbar);
  return toolbar;
}

QMainWindow *MainWindow::createTab(const QString &title)
{
  QMainWindow *tab = new QMainWindow(ui->tabWidget);
  tab->setDockOptions(
    QMainWindow::AllowNestedDocks |
    QMainWindow::AllowTabbedDocks |
    QMainWindow::AnimatedDocks |
    QMainWindow::GroupedDragging
  );
  ui->tabWidget->addTab(tab, title);
  return tab;
}

void MainWindow::setEditMode(bool enabled)
{
  _impl->editMode = enabled;
  ui->actionEdit_mode->setChecked(enabled);
}

bool MainWindow::event(QEvent *ev)
{
  switch(ev->type()) {
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

DockWidget *MainWindow::addDockWidget(
  const QString& className,
  int tabIndex,
  const QVariant &configuration,
  const QString& uuid
)
{
  tabIndex = tabIndex > 0 ? tabIndex : ui->tabWidget->currentIndex();

  return DockWidget::create(
    className,
    this,
    static_cast<QMainWindow*>(ui->tabWidget->widget(tabIndex)),
    uuid,
    configuration
  );
}
