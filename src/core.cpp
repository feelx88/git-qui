#include "core.hpp"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

#include "gitinterface.hpp"
#include "initialwindowconfiguration.hpp"
#include "mainwindow.hpp"
#include "project.hpp"
#include "projectsettingsdialog.hpp"
#include "toolbaractions.hpp"

struct ConfigurationKey {
  static constexpr const char *CURRENT_PROJECT = "currentProject";
  static constexpr const char *RECENT_PROJECTS = "recentProjects";
  static constexpr const char *MAIN_WINDOWS = "mainWindows";
};

struct CorePrivate {
  Core *_this;
  QSharedPointer<Project> project = nullptr;
  QVariantMap recentProjects;
  QList<MainWindow *> mainWindows;
  QTimer *autoFetchTimer = nullptr;

  CorePrivate(Core *core) : _this(core) {}

  bool loadProject(const QSettings &settings) {
    QString projectFileName =
        settings.value(ConfigurationKey::CURRENT_PROJECT).toString();

    if (projectFileName.isEmpty()) {
      QMessageBox dialog(
          QMessageBox::Question, QObject::tr("No Project selected"),
          QObject::tr("Would you like to create a new project? Alternatively, "
                      "you could open an existing one."),
          QMessageBox::Yes | QMessageBox::Open | QMessageBox::Abort);
      dialog.setButtonText(QMessageBox::Yes, QObject::tr("Create new project"));
      dialog.setButtonText(QMessageBox::Open,
                           QObject::tr("Open existing project"));

      switch (dialog.exec()) {
      case QMessageBox::Yes: {
        project.reset(new Project(_this));
        auto settingsDialog = ProjectSettingsDialog(
            ProjectSettingsDialog::DialogMode::CREATE, project.get());
        if (settingsDialog.exec() != QDialog::Accepted) {
          QMessageBox::critical(nullptr, QObject::tr("Error"),
                                QObject::tr("No project opened, closing."));
          return false;
        }
        break;
      }
      case QMessageBox::Open: {
        QString fileName = QFileDialog::getOpenFileName(
            nullptr, QObject::tr("Select project to open"));

        if (!fileName.isEmpty()) {
          project.reset(new Project(fileName, _this));
        }
        break;
      }
      default:
        QMessageBox::critical(nullptr, QObject::tr("Error"),
                              QObject::tr("No project opened, closing."));
        return false;
      }
    } else {
      project.reset(new Project(projectFileName, _this));
    }

    return true;
  }

  void createWindows(const QSettings &settings) {
    if (settings.contains(ConfigurationKey::MAIN_WINDOWS)) {
      for (auto &windowConfiguration :
           settings.value(ConfigurationKey::MAIN_WINDOWS).toList()) {
        addWindow(windowConfiguration);
      }
    } else {
      addWindow(QVariant());
    }
  }

  void addWindow(const QVariant &configuration) {
    auto window = new MainWindow(_this, configuration.toMap());

    if (configuration.isNull()) {
      InitialWindowConfiguration::create(window);
    }

    window->show();
    mainWindows.append(window);
  }

  void onAutoFetchTimerTimeout() {
    for (const auto &window : qAsConst(mainWindows)) {
      if (window->isActiveWindow()) {
        return;
      }
    }

    for (auto &repository : project->repositoryList()) {
      repository->fetchNonBlocking();
    }
  }
};

Core::Core(QObject *parent) : QObject(parent), _impl(new CorePrivate(this)) {}

Core::~Core() {
  if (project()) {
    project()->save();
  }

  QSettings settings;

  settings.setValue(ConfigurationKey::CURRENT_PROJECT,
                    _impl->project->fileName());

  if (!_impl->mainWindows.isEmpty()) {
    QVariantList mainWindows;
    for (auto &window : _impl->mainWindows) {
      mainWindows.append(window->configuration());
    }
    settings.setValue(ConfigurationKey::MAIN_WINDOWS, mainWindows);
  }

  settings.setValue(ConfigurationKey::RECENT_PROJECTS, _impl->recentProjects);

  settings.sync();
}

bool Core::init() {
  QSettings settings;

  if (!_impl->loadProject(settings)) {
    return false;
  }
  {
    _impl->recentProjects =
        settings.value(ConfigurationKey::RECENT_PROJECTS).toMap();
  }

  ToolBarActions::initialize(this);

  _impl->createWindows(settings);

  project()->reloadAllRepositories();

  _impl->autoFetchTimer = new QTimer(this);
  connect(_impl->autoFetchTimer, &QTimer::timeout, this,
          std::bind(&CorePrivate::onAutoFetchTimerTimeout, _impl.get()));
  _impl->autoFetchTimer->setInterval(
      std::chrono::seconds(AUTO_FETCH_INTERVAL_SECONDS));
  _impl->autoFetchTimer->start();

  return true;
}

void Core::changeProject(Project *newProject) {
  emit beforeProjectChanged(_impl->project.get());

  _impl->project.reset(newProject);
  _impl->recentProjects.insert(newProject->fileName(), newProject->name());

  emit projectChanged(_impl->project.get());

  project()->reloadAllRepositories();
}

Project *Core::project() const { return _impl->project.get(); }

QVariantMap Core::recentProjects() const { return _impl->recentProjects; }

void Core::clearRecentProjects() { _impl->recentProjects.clear(); }
