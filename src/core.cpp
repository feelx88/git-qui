#include "core.hpp"

#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QTimer>
#include <QtConcurrent/QtConcurrentRun>

#include "project.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"
#include "projectsettingsdialog.hpp"
#include "toolbaractions.hpp"
#include "initialwindowconfiguration.hpp"

struct ConfigurationKey
{
  static constexpr const char *CURRENT_PROJECT = "currentProject";
  static constexpr const char *RECENT_PROJECTS = "recentProjects";
  static constexpr const char *MAIN_WINDOWS = "mainWindows";
};

struct CoreImpl
{
  Core *_this;
  Project *project = nullptr;
  QVariantMap recentProjects;
  QList<MainWindow*> mainWindows;
  QTimer *autoFetchTimer = nullptr;
  QFuture<void> autoFetchFuture;

  CoreImpl(Core *core)
    : _this(core)
  {}

  virtual ~CoreImpl()
  {
    autoFetchFuture.cancel();
    autoFetchFuture.waitForFinished();
  }

  bool loadProject(const QSettings &settings)
  {
    QString projectFileName = settings.value(ConfigurationKey::CURRENT_PROJECT).toString();

    if (projectFileName.isEmpty())
    {
      QMessageBox dialog(
        QMessageBox::Question,
        QObject::tr("No Project selected"),
        QObject::tr("Would you like to create a new project? Alternatively, you could open an existing one."),
        QMessageBox::Yes | QMessageBox::Open | QMessageBox::Abort
      );
      dialog.setButtonText(QMessageBox::Yes, QObject::tr("Create new project"));
      dialog.setButtonText(QMessageBox::Open, QObject::tr("Open existing project"));

      switch (dialog.exec())
      {
      case QMessageBox::Yes:
      {
        project = new Project(_this);
        auto settingsDialog = ProjectSettingsDialog(ProjectSettingsDialog::DialogMode::CREATE, project);
        if (settingsDialog.exec() != QDialog::Accepted)
        {
          QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("No project opened, closing."));
          return false;
        }
        break;
      }
      case QMessageBox::Open:
      {
        QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Select project to open"));

        if (!fileName.isEmpty())
        {
          project = new Project(fileName, _this);
        }
        break;
      }
      default:
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("No project opened, closing."));
        return false;
      }
    }
    else
    {
      project = new Project(projectFileName, _this);
    }

    return true;
  }

  void createWindows(const QSettings &settings)
  {
    if (settings.contains(ConfigurationKey::MAIN_WINDOWS))
    {
      for (auto& windowConfiguration : settings.value(ConfigurationKey::MAIN_WINDOWS).toList())
      {
        addWindow(windowConfiguration);
      }
    }
    else
    {
      addWindow(QVariant());
    }
  }

  void addWindow(const QVariant &configuration)
  {
    auto window = new MainWindow(_this, configuration.toMap());

    if (configuration.isNull())
    {
      InitialWindowConfiguration::create(window);
    }

    window->show();
    mainWindows.append(window);
  }

  void onAutoFetchTimerTimeout()
  {
    if (project && autoFetchFuture.isFinished())
    {
      autoFetchFuture = QtConcurrent::run([this]{
        for (auto &repository : project->repositoryList())
        {
          if (!autoFetchFuture.isCanceled())
          {
            repository->fetch();
          }
        }
      });
    }
  }
};

Core::Core(QObject *parent)
: QObject(parent),
  _impl(new CoreImpl(this))
{}

Core::~Core()
{
  if(project())
  {
    project()->save();
  }

  QSettings settings;

  settings.setValue(ConfigurationKey::CURRENT_PROJECT, _impl->project->fileName());

  if (!_impl->mainWindows.isEmpty())
  {
    QVariantList mainWindows;
    for (auto &window : _impl->mainWindows)
    {
      mainWindows.append(window->configuration());
    }
    settings.setValue(ConfigurationKey::MAIN_WINDOWS, mainWindows);
  }

  settings.setValue(ConfigurationKey::RECENT_PROJECTS, _impl->recentProjects);

  settings.sync();
}

bool Core::init()
{
  QSettings settings;

  if(!_impl->loadProject(settings))
  {
    return false;
  }
  {
    _impl->recentProjects = settings.value(ConfigurationKey::RECENT_PROJECTS).toMap();
  }

  ToolBarActions::initialize(this);

  _impl->createWindows(settings);

  project()->reloadAllRepositories();

  _impl->autoFetchTimer = new QTimer(this);
  connect(_impl->autoFetchTimer, &QTimer::timeout, this, std::bind(&CoreImpl::onAutoFetchTimerTimeout, _impl.get()));
  _impl->autoFetchTimer->setInterval(std::chrono::seconds(30));
  _impl->autoFetchTimer->start();

  return true;
}

void Core::changeProject(Project *newProject)
{
  _impl->autoFetchFuture.cancel();
  _impl->autoFetchFuture.waitForFinished();
  delete _impl->project;

  _impl->project = newProject;
  _impl->recentProjects.insert(newProject->fileName(), newProject->name());

  emit projectChanged(_impl->project);

  project()->reloadAllRepositories();
}

Project *Core::project() const
{
  return _impl->project;
}

QVariantMap Core::recentProjects() const
{
  return _impl->recentProjects;
}

void Core::clearRecentProjects()
{
  _impl->recentProjects.clear();
}
