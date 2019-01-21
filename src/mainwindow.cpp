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

struct MainWindowPrivate
{
  QSharedPointer<GitInterface> selectedGitInterface;
  QMap<QString, QSharedPointer<GitInterface>> gitInterfaces;
  QList<QString> repositories;
  int currentRepository = 0;
  QLabel *progressSpinner;
  QTimer *autoFetchTimer;
  bool editMode;

  inline static const QString CONFIG_GEOMETRY = "geometry";
  inline static const QString CONFIG_STATE = "state";
  inline static const QString CONFIG_DOCK_WIDGETS = "dockWidgets";
  inline static const QString CONFIG_REPOSITORIES = "repositories";
  inline static const QString CONFIG_CURRENT_REPOSITORY = "currentRepository";
  inline static const QString CONFIG_DW_ID = "id";
  inline static const QString CONFIG_DW_CLASS = "class";
  inline static const QString CONFIG_DW_CONFIGURATION = "configuration";
  inline static const QString CONFIG_EDIT_MODE = "editMode";

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
        gitInterfaces.insert(repository, QSharedPointer<GitInterface>(new GitInterface(_this, repository)));
        addRepositoryMenuEntry(_this, repository);
      }
    }

    currentRepository = std::min(settings.value(CONFIG_CURRENT_REPOSITORY, 0).toInt(), repositories.size() - 1);
    selectedGitInterface = gitInterfaces.value(repositories.at(currentRepository), nullptr);
  }

  bool selectRepository(MainWindow *_this)
  {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::DirectoryOnly);
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
        auto inserted = gitInterfaces.insert(path, QSharedPointer<GitInterface>(new GitInterface(_this, path)));
        repositories.append(path);
        addRepositoryMenuEntry(_this, path);
        emit _this->repositoryAdded(inserted.value());
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
      selectedGitInterface->reload();
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

    progressSpinner = new QLabel(_this);
    QMovie *movie = new QMovie(QDir::currentPath() + "/loading.gif", QByteArray(), _this);
    progressSpinner->hide();
    movie->start();
    progressSpinner->setMovie(movie);
    _this->statusBar()->addPermanentWidget(progressSpinner);
    _this->statusBar()->setSizeGripEnabled(false);
    _this->statusBar()->hide();

    _this->connect(_this->statusBar(), &QStatusBar::messageChanged, _this, [=](const QString &message){
      if (message.isEmpty())
      {
        _this->statusBar()->hide();
        progressSpinner->hide();
      }
    });

    _this->connect(_this->ui->actionPush, &QAction::triggered, _this, [=]{

      if (selectedGitInterface->activeBranch().upstreamName.isEmpty())
      {
        QString branch = selectedGitInterface->activeBranch().name;
        bool addUpstream = QMessageBox::question(
          _this,
          _this->tr("No upstream branch configured"),
          _this->tr("Would you like to set the default upstream branch to origin/%1?").arg(branch),
          QMessageBox::Yes,
          QMessageBox::No
        ) == QMessageBox::Yes;

        if(addUpstream)
        {
          this->selectedGitInterface->setUpstream("origin", branch);
        }
      }

      progressSpinner->show();
      _this->statusBar()->show();
      _this->statusBar()->showMessage("Pushing...");
      QtConcurrent::run([=]{
        selectedGitInterface->push();
      });
    });
    _this->connect(_this->ui->actionPull, &QAction::triggered, _this, [=]{
      progressSpinner->show();
      _this->statusBar()->show();
      _this->statusBar()->showMessage("Pulling...");
      QtConcurrent::run([=]{
        selectedGitInterface->pull(false);
      });
    });
    _this->connect(_this->ui->actionPull_Rebase, &QAction::triggered, _this, [=]{
      progressSpinner->show();
      _this->statusBar()->show();
      _this->statusBar()->showMessage("Pulling...");
      QtConcurrent::run([=]{
        selectedGitInterface->pull(true);
      });
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

    _this->connect(_this->ui->actionAbout_qt, &QAction::triggered, _this, QApplication::aboutQt);

    _this->connect(_this->ui->actionEdit_mode, &QAction::toggled, _this, [=](bool checked){
      editMode = checked;
      for (auto dockWidget : _this->findChildren<DockWidget*>())
      {
        dockWidget->setEditModeEnabled(editMode);
      }
    });
  }

  void initAutoFetchTimer(MainWindow *_this)
  {
    autoFetchTimer = new QTimer(_this);
    autoFetchTimer->setInterval(30000);
    _this->connect(autoFetchTimer, &QTimer::timeout, _this, [=]{
      for (auto interface : gitInterfaces)
      {
        QtConcurrent::run([=]{
          interface->fetch();
        });
      }
    });
    autoFetchTimer->start();
  }

  void connectGitInterfaceSignals(MainWindow *_this, const QSharedPointer<GitInterface> &gitInterface)
  {
    _this->connect(gitInterface.get(), &GitInterface::reloaded, _this, [=]{
      emit _this->repositoryAdded(gitInterfaces.value(gitInterface->path()));
    });
    _this->connect(gitInterface.get(), &GitInterface::pushed, _this, [=]{
      progressSpinner->hide();
      _this->statusBar()->showMessage(_this->tr("Pushed succesfully"), 3000);
      gitInterface->log();
    });
    _this->connect(gitInterface.get(), &GitInterface::pulled, _this, [=]{
      progressSpinner->hide();
      _this->statusBar()->showMessage(_this->tr("Pulled succesfully"), 3000);
      gitInterface->log();
    });

    _this->connect(gitInterface.get(), &GitInterface::error, _this, [=](const QString& message){
      progressSpinner->hide();
      _this->statusBar()->show();
      _this->statusBar()->showMessage(message, 3000);
    });
  }

  void populateMenu(MainWindow *_this)
  {
    for (DockWidget::RegistryEntry *entry : DockWidget::registeredDockWidgets())
    {
      QAction *action = _this->ui->menuAdd_view->addAction(entry->name, [=]{
        DockWidget::create(entry->id, _this, selectedGitInterface);
        emit _this->repositorySwitched(selectedGitInterface);
        for (auto interface : gitInterfaces) {
          emit _this->repositoryAdded(interface);
          interface->reload();
        }
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
        selectedGitInterface,
        config.value(CONFIG_DW_ID).toString(),
        config.value(CONFIG_DW_CONFIGURATION)
      );
    }

    _this->restoreState(settings.value(CONFIG_STATE).toByteArray());

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

  for (auto interface : _impl->gitInterfaces)
  {
    interface->disconnect(nullptr, this);
  }

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

void MainWindow::switchRepository(const QString &path)
{
  _impl->currentRepository = _impl->repositories.indexOf(path);
  _impl->selectedGitInterface = _impl->gitInterfaces.value(path, nullptr);
  emit repositorySwitched(_impl->selectedGitInterface);
  _impl->selectedGitInterface->reload();

  setWindowTitle(QString("MainWindow - %1").arg(path.split('/').last()));
}

void MainWindow::changeEvent(QEvent *ev)
{
  if (ev->type() == QEvent::ActivationChange && isActiveWindow())
  {
    _impl->postInit();
  }
}
