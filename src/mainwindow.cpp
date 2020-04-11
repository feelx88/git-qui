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
  static constexpr const char *TABS = "tabs";
  static constexpr const char *TAB_NAME = "tabName";
  static constexpr const char *DOCK_WIDGETS = "dockWidgets";
  static constexpr const char *DW_ID = "id";
  static constexpr const char *DW_CLASS = "class";
  static constexpr const char *DW_CONFIGURATION = "configuration";
  static constexpr const char *EDIT_MODE = "editMode";
};

struct MainWindowPrivate
{
  MainWindow *_this;
  Core *core;
  bool editMode;
  QVariantMap configuration;

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
  }

  void populateMenu()
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

  void restoreSettings(MainWindow *_this)
  {
    if (!configuration.contains(ConfigurationKeys::GEOMETRY))
    {
      InitialWindowConfiguration::create(_this);
      return;
    }

    _this->restoreGeometry(configuration.value(ConfigurationKeys::GEOMETRY).toByteArray());

    QMap<QString, QVariant> tabs =
      configuration.value(ConfigurationKeys::TABS).toMap();

    for (auto tab : tabs.toStdMap())
    {
      QMap<QString, QVariant> config = tab.second.toMap();
      QList<QVariant> dockWidgetConfigurations = config.value(ConfigurationKeys::DOCK_WIDGETS).toList();

      QMainWindow *page = _this->createTab(config.value(ConfigurationKeys::TAB_NAME).toString());

      for (QVariant dockWidgetConfiguration : dockWidgetConfigurations)
      {
        QMap<QString, QVariant> config = dockWidgetConfiguration.toMap();
        DockWidget::create(
          config.value(ConfigurationKeys::DW_CLASS).toString(),
          _this,
          page,
          config.value(ConfigurationKeys::DW_ID).toString(),
          config.value(ConfigurationKeys::DW_CONFIGURATION)
        );
      }
      page->restoreState(config.value(ConfigurationKeys::STATE).toByteArray());
      page->restoreGeometry(config.value(ConfigurationKeys::GEOMETRY).toByteArray());
    }

    QList<QVariant> toolbars = configuration.value("toolbars").toList();
    for (auto toolbarConfig: toolbars)
    {
      QMap<QString, QVariant> config = toolbarConfig.toMap();
      QToolBar *toolbar = _this->addToolbar(static_cast<Qt::ToolBarArea>(config.value("area").toInt()));
      toolbar->restoreGeometry(config.value("geometry").toByteArray());

      for (auto action : config.value("actions").toList())
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

  _impl->configuration = configuration;
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
  return _impl->configuration;
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

void MainWindow::changeEvent(QEvent *ev)
{
  if (ev->type() == QEvent::ActivationChange && isActiveWindow())
  {
  }
}
