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

#include "gitinterface.hpp"
#include "components/dockwidget.hpp"

struct MainWindowPrivate
{
  QSharedPointer<GitInterface> gitInterface;
  QList<QString> repositories;
  int currentRepository = 0;

  inline static const QString CONFIG_GEOMETRY = "geometry";
  inline static const QString CONFIG_STATE = "state";
  inline static const QString CONFIG_DOCK_WIDGETS = "dockWidgets";
  inline static const QString CONFIG_REPOSITORIES = "repositories";
  inline static const QString CONFIG_CURRENT_REPOSITORY = "currentRepository";
  inline static const QString CONFIG_DW_ID = "id";
  inline static const QString CONFIG_DW_CLASS = "class";
  inline static const QString CONFIG_DW_CONFIGURATION = "configuration";

  void initGit(MainWindow *_this)
  {
    QSettings settings;
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
        addRepositoryMenuEntry(_this, repository);
      }
    }

    currentRepository = std::min(settings.value(CONFIG_CURRENT_REPOSITORY, 0).toInt(), repositories.size() - 1);
    gitInterface.reset(new GitInterface(_this, repositories.at(currentRepository)));
  }

  bool selectRepository(MainWindow *_this)
  {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::DirectoryOnly);
    dialog.setWindowTitle(dialog.tr("Select repository to open"));
    dialog.exec();

    if (dialog.result() == QFileDialog::Accepted)
    {
      QString path = dialog.directory().absolutePath();
      repositories.append(path);
      addRepositoryMenuEntry(_this, path);
      emit _this->repositoryAdded(path);
      return true;
    }

    return false;
  }

  void addRepositoryMenuEntry(MainWindow *_this, const QString &path)
  {
    _this->ui->menuRepositories->addAction(path, _this, [=]{
      gitInterface->switchRepository(path);
      currentRepository = repositories.indexOf(path);
    });
  }

  void closeCurrentRepository(MainWindow *_this)
  {
    QString path = repositories.at(currentRepository);
    _this->ui->menuRepositories->removeAction(_this->ui->menuRepositories->actions().at(currentRepository));
    repositories.removeAt(currentRepository);
    emit _this->repositoryRemoved(path);

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
    gitInterface->switchRepository(repositories.at(0));
  }

  void connectSignals(MainWindow *_this)
  {
    _this->connect(_this->ui->actionTop, &QAction::triggered, _this, [_this]{
      _this->addToolBar(Qt::TopToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionBottom, &QAction::triggered, _this, [_this]{
      _this->addToolBar(Qt::BottomToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionLeft, &QAction::triggered, _this, [_this]{
      _this->addToolBar(Qt::LeftToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionRight, &QAction::triggered, _this, [_this]{
      _this->addToolBar(Qt::RightToolBarArea, new QToolBar(_this));
    });

    _this->connect(_this->ui->actionReload_current_repository, &QAction::triggered, _this, [this]{
      gitInterface->reload();
    });

    _this->connect(_this->ui->actionOpen_Repository, &QAction::triggered, _this, [=]{
      selectRepository(_this);
    });
    _this->connect(_this->ui->actionClose_current_repository, &QAction::triggered, _this, [=]{
      closeCurrentRepository(_this);
    });

    _this->statusBar()->hide();
    _this->connect(gitInterface.get(), &GitInterface::error, _this, [=](const QString& message){
      _this->statusBar()->show();
      _this->statusBar()->showMessage(message, 3000);
    });
    _this->connect(_this->statusBar(), &QStatusBar::messageChanged, _this, [=](const QString &message){
      if (message.isEmpty())
      {
        _this->statusBar()->hide();
      }
    });

    _this->connect(gitInterface.get(), &GitInterface::reloaded, _this, [=]{
      for (auto repository : repositories)
      {
        emit _this->repositoryAdded(repository);
      }
    });

    _this->connect(gitInterface.get(), &GitInterface::repositorySwitched, _this, [=](const QString &path){
      currentRepository = repositories.indexOf(path);
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

  void restoreSettings(MainWindow *_this)
  {
    QSettings settings;
    _this->restoreGeometry(settings.value(CONFIG_GEOMETRY).toByteArray());

    QList<QVariant> dockWidgetConfigurations =
      settings.value(CONFIG_DOCK_WIDGETS).toList();

    for (QVariant dockWidgetConfiguration : dockWidgetConfigurations)
    {
      QMap<QString, QVariant> config = dockWidgetConfiguration.toMap();
      DockWidget::create(
        config.value(CONFIG_DW_CLASS).toString(),
        _this,
        gitInterface,
        config.value(CONFIG_DW_ID).toString(),
        config.value(CONFIG_DW_CONFIGURATION)
      );
    }

    _this->restoreState(settings.value(CONFIG_STATE).toByteArray());
  }

  void postInit()
  {
    gitInterface->reload();
    emit gitInterface->repositorySwitched(repositories.at(currentRepository));
  }

  void saveSettings(MainWindow *_this)
  {
    QSettings settings;
    settings.setValue(CONFIG_GEOMETRY, _this->saveGeometry());
    settings.setValue(CONFIG_STATE, _this->saveState());
    settings.setValue(CONFIG_REPOSITORIES, QVariant(repositories));
    settings.setValue(CONFIG_CURRENT_REPOSITORY, QVariant(currentRepository));

    QList<QVariant> dockWidgetConfigurations;

    for (auto dockWidget : _this->findChildren<DockWidget*>())
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

    settings.setValue(CONFIG_DOCK_WIDGETS, QVariant(dockWidgetConfigurations));
  }
};

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow),
_impl(new MainWindowPrivate)
{
  ui->setupUi(this);

  _impl->initGit(this);
  _impl->connectSignals(this);
  _impl->populateMenu(this);
  _impl->restoreSettings(this);
  _impl->postInit();
}

MainWindow::~MainWindow()
{
  _impl->saveSettings(this);
  delete ui;
}

void MainWindow::openRepository()
{
  _impl->selectRepository(this);
}

void MainWindow::closeCurrentRepository()
{
  _impl->closeCurrentRepository(this);
}
