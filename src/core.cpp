#include "core.hpp"

#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

#include "project.hpp"
#include "mainwindow.hpp"
#include "gitinterface.hpp"
#include "projectsettingsdialog.hpp"
#include "toolbaractions.hpp"
#include "initialwindowconfiguration.hpp"

struct ConfigurationKey
{
  static constexpr const char *CURRENT_PROJECT = "currentProject";
  static constexpr const char *MAIN_WINDOWS = "mainWindows";
};

struct CoreImpl
{
  Core *_this;
  Project *project = nullptr;
  QList<MainWindow*> mainWindows;

  CoreImpl(Core *core)
    : _this(core)
  {}

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
};

Core::Core(QObject *parent)
: QObject(parent),
  _impl(new CoreImpl(this))
{}

Core::~Core()
{
  QSettings settings;

  settings.setValue(ConfigurationKey::CURRENT_PROJECT, _impl->project->fileName());

  QVariantList mainWindows;
  for (auto &window : _impl->mainWindows)
  {
    mainWindows.append(window->configuration());
  }
  settings.setValue(ConfigurationKey::MAIN_WINDOWS, mainWindows);

  settings.sync();
}

bool Core::init()
{
  QSettings settings;

  if(!_impl->loadProject(settings))
  {
    return false;
  }

  ToolBarActions::initialize(this);

  _impl->createWindows(settings);

  _impl->project->activeRepository()->reload();

  return true;
}

void Core::changeProject(Project *project)
{
  _impl->project = project;

  emit projectChanged(_impl->project);

  project->activeRepository()->reload();
}

Project *Core::project() const
{
  return _impl->project;
}
