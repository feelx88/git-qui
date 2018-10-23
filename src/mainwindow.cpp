#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QToolBar>
#include <QDir>
#include <QDockWidget>
#include <QDebug>
#include <QSettings>
#include <QStatusBar>

#include "gitinterface.hpp"
#include "components/dockwidget.hpp"

struct MainWindowPrivate
{
  QSharedPointer<GitInterface> gitInterface;

  inline static const QString CONFIG_GEOMETRY = "geometry";
  inline static const QString CONFIG_STATE = "state";
  inline static const QString CONFIG_DOCK_WIDGETS = "dockWidgets";
  inline static const QString CONFIG_DW_ID = "id";
  inline static const QString CONFIG_DW_CLASS = "class";
  inline static const QString CONFIG_DW_CONFIGURATION = "configuration";

  void connectSignals(MainWindow *_this)
  {
    _this->connect(_this->ui->actionTop, &QAction::triggered, [_this]{
      _this->addToolBar(Qt::TopToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionBottom, &QAction::triggered, [_this]{
      _this->addToolBar(Qt::BottomToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionLeft, &QAction::triggered, [_this]{
      _this->addToolBar(Qt::LeftToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionRight, &QAction::triggered, [_this]{
      _this->addToolBar(Qt::RightToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionReload_current_repository, &QAction::triggered, [this]{
      gitInterface->reload();
    });

    _this->statusBar()->hide();
    _this->connect(gitInterface.get(), &GitInterface::error, [=](const QString& message){
      _this->statusBar()->show();
      _this->statusBar()->showMessage(message, 3000);
    });
    _this->connect(_this->statusBar(), &QStatusBar::messageChanged, [=](const QString &message){
      if (message.isEmpty())
      {
        _this->statusBar()->hide();
      }
    });
  }

  void populateMenu(MainWindow *_this)
  {
    for (DockWidget::RegistryEntry *entry : DockWidget::registeredDockWidgets())
    {
      QAction *action = _this->ui->menuAdd_view->addAction(entry->name, [=]{
        entry->initializer(_this, gitInterface);
        gitInterface->reload();
      });
      action->setData(entry->id);
    }
  }
};

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow),
_impl(new MainWindowPrivate)
{
  ui->setupUi(this);

  _impl->gitInterface.reset(new GitInterface(this, QDir::current()));
  _impl->connectSignals(this);
  _impl->populateMenu(this);

  QSettings settings;
  restoreGeometry(settings.value(MainWindowPrivate::CONFIG_GEOMETRY).toByteArray());

  QList<QVariant> dockWidgetConfigurations =
    settings.value(MainWindowPrivate::CONFIG_DOCK_WIDGETS).toList();

  for (QVariant dockWidgetConfiguration : dockWidgetConfigurations)
  {
    QMap<QString, QVariant> config = dockWidgetConfiguration.toMap();
    DockWidget::create(
      config.value(MainWindowPrivate::CONFIG_DW_CLASS).toString(),
      this,
      _impl->gitInterface,
      config.value(MainWindowPrivate::CONFIG_DW_ID).toString(),
      config.value(MainWindowPrivate::CONFIG_DW_CONFIGURATION)
    );
  }

  restoreState(settings.value(MainWindowPrivate::CONFIG_STATE).toByteArray());

  _impl->gitInterface->reload();
}

MainWindow::~MainWindow()
{
  QSettings settings;
  settings.setValue(MainWindowPrivate::CONFIG_GEOMETRY, saveGeometry());
  settings.setValue(MainWindowPrivate::CONFIG_STATE, saveState());

  QList<QVariant> dockWidgetConfigurations;

  for (auto dockWidget : findChildren<DockWidget*>())
  {
    QMap<QString, QVariant> configuration;
    configuration.insert(
      MainWindowPrivate::CONFIG_DW_CLASS, dockWidget->metaObject()->className()
    );
    configuration.insert(
      MainWindowPrivate::CONFIG_DW_ID, dockWidget->objectName()
    );
    configuration.insert(
      MainWindowPrivate::CONFIG_DW_CONFIGURATION, dockWidget->configuration()
    );
    dockWidgetConfigurations.append(configuration);
    delete dockWidget;
  }

  settings.setValue(MainWindowPrivate::CONFIG_DOCK_WIDGETS, QVariant(dockWidgetConfigurations));

  delete ui;
}
